#include "ImageSet.hpp"

#include "../../wx/artprovs.hpp"

#include <bas/log/uselog.h>
#include <bas/proc/Assets.hpp>
#include <bas/util/Path.hpp>

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/mstream.h>
#include <wx/string.h>

#include <algorithm>
#include <optional>

auto ident = [](const std::string& path) { return path; };

auto blank_bitmap = [](int width, int height, const wxArtClient& client) {
    return wxBitmap(width, height, 24);
};

const BitmapMode BitmapMode::DEFAULT = BitmapMode{
    .no_stockart = true,
    .no_asset = false,
    .assets_preferred = true,
    .exactly = false,
    .translate = nullptr, // ident,
    .fallback = blank_bitmap,
};

const ImageSet ImageSet::EMPTY = ImageSet(wxString(), std::nullopt);

ImageSet::ImageSet(wxArtID artId, std::string dir, std::string name, std::string text)
    : m_artId(artId), m_asset(Path(dir, name)) {
    if (text.empty())
        if (!artId.empty())
            text = artId.ToStdString();
        else
            text = m_asset->name();
    m_text = text;
    init();
}

ImageSet::ImageSet(std::optional<Path> asset, std::string text) : m_asset(asset) {
    if (text.empty())
        if (asset)
            text = asset->name();
        else
            text = "(blank)";
    m_text = text;
    init();
}

ImageSet::ImageSet(wxArtID artId, std::optional<Path> asset, std::string text)
    : m_artId(artId), m_asset(asset) {
    if (text.empty()) {
        if (!artId.empty())
            text = artId.ToStdString();
        else if (asset)
            text = asset->name();
        else
            text = "(blank)";
    }
    m_text = text;
    init();
}

void ImageSet::init() {
    if (m_asset) {
        Volume* memzip = assets.get();
        if (!memzip) {
            logerror("assets memzip is not ready");
        } else {
            detect(memzip);
        }
    }
}

ImageSet& ImageSet::detect(Volume* volume) {
    if (!volume)
        throw std::invalid_argument("volume is required");
    if (!m_asset)
        return *this;

    auto dir = m_asset->getParent();
    auto name = m_asset->name();

    auto files = volume->readDir(dir.str());
    for (const auto& _file : files) {
        Path _path(dir.str(), _file->name);
        std::string _extension = _path.extension();
        std::string _name = _path.name();

        int lastDash = _name.find_last_of('-');
        if (lastDash == std::string::npos)
            continue;

        std::string _stem = _name.substr(0, lastDash);
        if (_stem != name)
            continue;

        std::string wxh = _name.substr(lastDash + 1);

        int xpos = wxh.find('x');
        if (xpos == std::string::npos)
            continue;
        std::string widthStr = wxh.substr(0, wxh.find('x'));
        std::string heightStr = wxh.substr(wxh.find('x') + 1);

        // test if numbers are valid
        if (widthStr.empty() || heightStr.empty())
            continue;
        if (!std::all_of(widthStr.begin(), widthStr.end(), ::isdigit))
            continue;
        if (!std::all_of(heightStr.begin(), heightStr.end(), ::isdigit))
            continue;

        int width = std::stoi(widthStr);
        int height = std::stoi(heightStr);
        if (width <= 0 || height <= 0)
            continue;

        logdebug_fmt("found scale %dx%d for %s", width, height, _path.str().c_str());
        this->scale(width, height, _path);
    }
    return *this;
}

bool ImageSet::empty() const { return m_artId.empty() && !m_asset && m_scaledAssets.empty(); }

bool ImageSet::isSet() const { return !empty(); }

bool ImageSet::operator==(const ImageSet& other) const {
    return m_artId == other.m_artId && m_asset == other.m_asset &&
           m_asset_width == other.m_asset_width && m_asset_height == other.m_asset_height &&
           m_scaledAssets == other.m_scaledAssets;
}

ImageSet& ImageSet::artId(wxArtID id) {
    m_artId = id;
    return *this;
}

ImageSet& ImageSet::asset(std::optional<Path> p) {
    m_asset = p;
    return *this;
}

ImageSet& ImageSet::scale(int w, int h, const Path& p) {
    m_scaledAssets.insert_or_assign({w, h}, p);
    return *this;
}

static int size_diff(int expected_w, int expected_h, int actual_w, int actual_h) {
    int dw = std::abs(actual_w - expected_w);
    int dh = std::abs(actual_h - expected_h);
    return dw * dw + dh * dh;
}

std::optional<ScaledAsset> ImageSet::findBestMatch(int width, int height,
                                                   const wxArtClient& client) const {
    std::optional<ScaledAsset> result = std::nullopt;
    int best_diff = -1;
    if (m_asset) {
        if (m_asset_width <= 0 || m_asset_height <= 0) {
            best_diff = std::numeric_limits<int>::max();
        } else {
            best_diff = size_diff(width, height, m_asset_width, m_asset_height);
        }
        result = std::make_optional<ScaledAsset>(m_asset_width, m_asset_height, *m_asset);
    }
    if (best_diff == 0)
        return result;

    for (const auto& [size, path] : m_scaledAssets) {
        int diff = size_diff(width, height, size.first, size.second);
        if (best_diff == -1 || diff < best_diff) {
            result = std::make_optional<ScaledAsset>(size.first, size.second, path);
            best_diff = diff;
            if (diff == 0)
                break;
        }
    }
    return result;
}

std::optional<ScaledAsset> ImageSet::findExactly(int width, int height,
                                                 const wxArtClient& client) const {
    for (const auto& [size, path] : m_scaledAssets) {
        if (size.first == width && size.second == height) {
            return std::make_optional<ScaledAsset>(size.first, size.second, path);
        }
    }
    if (m_asset && m_asset_width == width && m_asset_height == height) {
        return std::make_optional<ScaledAsset>(width, height, *m_asset);
    }
    return std::nullopt;
}

std::optional<std::string> ImageSet::findBestMatchAssetPath(int width, int height,
                                                            const wxArtClient& client) const {
    std::optional<ScaledAsset> sa = findBestMatch(width, height);
    if (sa) {
        return sa->path.str();
    }
    return std::nullopt;
}

std::optional<std::string> ImageSet::findExactlyAssetPath(int width, int height,
                                                          const wxArtClient& client) const {
    std::optional<ScaledAsset> sa = findExactly(width, height);
    if (sa) {
        return sa->path.str();
    }
    return std::nullopt;
}

std::optional<wxBitmap> ImageSet::toBitmap(int width, int height, const wxArtClient& client,
                                           const BitmapMode& mode) const {
    bool use_stockart = !mode.no_stockart && !m_artId.empty();
    bool use_asset = !mode.no_asset && m_asset;
    bool match_exactly = mode.exactly;
    bool match_best = !match_exactly;
    bool assets_preferred = mode.assets_preferred;
    bool stockart_preferred = !assets_preferred;

    if (use_stockart && stockart_preferred) {
        std::optional<wxBitmap> bmp = bitmapFromArt(width, height, client, mode);
        if (bmp && bmp->IsOk())
            return bmp;
    }

    do {
        if (!use_asset)
            break;

        std::optional<std::string> path = match_best ? findBestMatchAssetPath(width, height, client)
                                                     : findExactlyAssetPath(width, height, client);
        if (!path) {
            // logerror_fmt("No asset: %s", imageSet.name);
            break;
        }

        std::vector<uint8_t> data = assets_get_data(*path);
        if (data.empty()) {
            logerror_fmt("No asset: %s", path->c_str());
            // assets_dump_tree();
            break;
        }

        wxMemoryInputStream stream(data.data(), data.size());

        wxImage image;
        wxLogBuffer logBuf;
        wxLog* oldLog = wxLog::SetActiveTarget(&logBuf);
        bool loadStatus = image.LoadFile(stream);
        wxLog::SetActiveTarget(oldLog); // Restore original logger

        if (!loadStatus) {
            // This will contain the specific reason (e.g., "no handler for this type")
            wxString reason = logBuf.GetBuffer();
            logerror_fmt("Failed to load bitmap from asset: %s, reason %s", //
                         path->c_str(),                                     //
                         reason.ToStdString().c_str());

            // dump header of data
            // loginfo_fmt("file %s size %d", path.data(), data.size());
            // size_t hdrsize = std::min((size_t)1024, data.size());
            // hexdump(data, hdrsize);
            break;
        }

        if (!image.IsOk()) {
            logerror_fmt("error loaded bitmap from asset: %s, use empty image", path->c_str());
            break;
        }

        if ((width > 0 && height > 0) &&
            (image.GetWidth() != width || image.GetHeight() != height)) {
            loglog_fmt("Bitmap %s scaled from %dx%d to %dx%d", path->c_str(), //
                       image.GetWidth(), image.GetHeight(), width, height);
            image = image.Scale(width, height, wxIMAGE_QUALITY_BILINEAR);
        }

        wxBitmap bmp(image);
        if (!bmp.IsOk()) {
            logwarn_fmt("Failed to create bitmap from image: %s, fallback to empty image",
                        path->c_str());
            break;
        }
        return bmp;
    } while (false);

    if (use_stockart && !stockart_preferred) {
        std::optional<wxBitmap> bmp = bitmapFromArt(width, height, client, mode);
        if (bmp && bmp->IsOk())
            return bmp;
    }

    if (mode.fallback) {
        wxBitmap bmp = mode.fallback(width, height, client);
        assert(bmp.IsOk());
        return bmp;
    }
    return std::nullopt;
}

std::optional<wxBitmap> ImageSet::bitmapFromArt(int width, int height, const wxArtClient& client,
                                                const BitmapMode& mode) const {
    if (m_artId.empty()) {
        if (mode.fallback) {
            wxBitmap bmp = mode.fallback(width, height, client);
            assert(bmp.IsOk());
            return bmp;
        }
        return std::nullopt;
    }

    bool match_exactly = mode.exactly;
    bool match_best = !match_exactly;
    bool assets_preferred = mode.assets_preferred;
    bool stockart_preferred = !assets_preferred;

    if (stockart_preferred) {
        wxBitmap bmp = wxArtProvider::GetBitmap(m_artId, client, wxSize(width, height));
        if (bmp.IsOk())
            return bmp;
    }

    std::string path = wxArtProviders::getAlternativeArtAssetPath(m_artId);
    if (!path.empty()) {
        return wxArtProvider::GetBitmap(path, client, wxSize(width, height));
    }

    ImageSet alt(path);
    BitmapMode alt_mode{
        .no_stockart = true,
        .no_asset = false,
        .assets_preferred = mode.assets_preferred,
        .exactly = false,
        .translate = mode.translate,
        .fallback = mode.fallback,
    };
    std::optional<wxBitmap> bmp = alt.toBitmap(width, height, client, alt_mode);
    if (bmp && bmp->IsOk())
        return bmp;

    if (mode.fallback) {
        wxBitmap bmp = mode.fallback(width, height, client);
        assert(bmp.IsOk());
        return bmp;
    }
    return std::nullopt;
}

wxBitmap ImageSet::toBitmap1(int width, int height, const wxArtClient& client,
                             const BitmapMode& mode) const {
    std::optional<wxBitmap> bmp = toBitmap(width, height, client, mode);
    if (bmp && bmp->IsOk())
        return *bmp;
    return wxBitmap(width, height, 24);
}

wxBitmap ImageSet::bitmapFromArt1(int width, int height, const wxArtClient& client,
                                  const BitmapMode& mode) const {
    std::optional<wxBitmap> bmp = bitmapFromArt(width, height, client, mode);
    if (bmp && bmp->IsOk())
        return *bmp;
    return wxBitmap(width, height, 24);
}

void ImageSet::dump(std::ostream& os) const {
    os << "ImageSet: " << m_text << " " << m_artId << " " << m_asset->str() << std::endl;
    for (const auto& [size, path] : m_scaledAssets) {
        os << "  " << size.first << "x" << size.second << " " << path.str() << std::endl;
    }
}
