#include "ImageSet.hpp"

#include "../../wx/artprovs.hpp"
#include "../../wx/images.hpp"

#include <bas/proc/AssetsRegistry.hpp>
#include <bas/util/Path.hpp>

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/mstream.h>
#include <wx/string.h>

#include <algorithm>
#include <cstdlib>
#include <optional>

#include <bas/log/uselog.h>

auto ident = [](const std::string& path) { return path; };

auto blank_bitmap = [](int width, int height, const wxArtClient& client, ReasonCode reason_code) {
    return bitmapWithReason(width, height, client, BitmapMode::DEFAULT, reason_code);
};

const BitmapMode BitmapMode::DEFAULT = BitmapMode{
    .no_stockart = true,
    .no_asset = false,
    .assets_preferred = true,
    .exactly = false,
    .include_raw = true,
    .translate = nullptr, // ident,
    .fallback = blank_bitmap,
};

const ImageSet ImageSet::EMPTY = ImageSet(wxString(), std::nullopt);

ImageSet::ImageSet(wxArtID artId, std::string dir, std::string tail, std::string text)
    : m_artId(artId), m_asset(Path(dir, tail)) {
    if (text.empty())
        if (!artId.empty())
            text = artId.ToStdString();
        else
            text = m_asset->name();
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

ImageSet::ImageSet(const Path& asset, std::string text) : m_artId(), m_asset(asset) {
    if (text.empty())
        text = asset.name();
    m_text = text;
    init();
}

void ImageSet::init() {
    if (m_asset) {
        Volume* assets = AssetsRegistry::instance().get();
        if (!assets) {
            logerror("assets is not ready");
        } else {
            detect(assets);
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

    if (!volume->isDirectory(dir.str())) {
        logerror_fmt("ImageSet: directory %s is not a directory", dir.str().c_str());
        return *this;
    }

    auto files = volume->readDir(dir.str());
    for (const auto& [k, _file] : files->children) {
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

        // logerror_fmt("found scale %dx%d for %s", width, height, _path.str().c_str());
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

std::optional<ScaledAsset> ImageSet::findBestMatch(int width, int height, const wxArtClient& client,
                                                   bool include_raw) const {
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
                                                            const wxArtClient& client,
                                                            bool include_raw) const {
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

BitmapResult ImageSet::toBitmap(int width, int height, const wxArtClient& client,
                                           const BitmapMode& mode) const {
    bool use_stockart = !mode.no_stockart && !m_artId.empty();
    bool match_exactly = mode.exactly;
    bool match_best = !match_exactly;
    bool assets_preferred = mode.assets_preferred;
    bool stockart_preferred = !assets_preferred;

    ReasonCode reason_code = ReasonCode::BMP_OK;

    if (use_stockart && stockart_preferred) {
        BitmapResult result = _bitmapFromArt(width, height, client, mode);
        if (result.hasBitmap()) {
            if (m_asset) {
                logdebug_fmt("Bitmap from stockart: %s, asset: %s", m_artId.ToStdString().c_str(), m_asset->str().c_str());
            }
            return result;
        }
        logdebug_fmt("Failed to create bitmap from stockart: %s", m_artId.ToStdString().c_str());
        reason_code = ReasonCode::BMP_BAD_ART_ALT;
    }

    do {
        if (mode.no_asset) {
            logdebug_fmt("Asset is disabled");
            reason_code = ReasonCode::BMP_DISABLED;
            break;
        }

        if (!m_asset) {
            logdebug_fmt("Asset is not set");
            reason_code = ReasonCode::BMP_NONE;
            break;
        }

        std::optional<std::string> _path =
            match_best ? findBestMatchAssetPath(width, height, client, mode.include_raw)
                       : findExactlyAssetPath(width, height, client);
        if (!_path) {
            logerror_fmt("No matched asset path for %s - %dx%d",
                         m_asset ? m_asset->str().c_str() : "(none)", width, height);
            break;
        }

        std::string path = *_path;

        logdebug_fmt("found asset %s %dx%d at path: %s", //
                     match_best ? "best match" : "exactly", width, height, path.c_str());

        std::optional<wxBitmap> bmp = imageLoadAsset(path, width, height);

        if (!bmp || !bmp->IsOk()) {
            reason_code = ReasonCode::BMP_BAD_ASSET;
            logwarn_fmt("Failed to create bitmap from image: %s, fallback to empty image",
                        path.c_str());
            break;
        }
        return BitmapResult(bmp);
    } while (false);

    if (use_stockart && !stockart_preferred) {
        BitmapResult result = _bitmapFromArt(width, height, client, mode);
        if (result.isOk())
            return result;
    }

    if (mode.fallback) {
        wxBitmap bmp = mode.fallback(width, height, client, reason_code);
        assert(bmp.IsOk());
        return bmp;
    }
    return reason_code;
}

BitmapResult ImageSet::_bitmapFromArt(int width, int height, const wxArtClient& client,
                                                 const BitmapMode& mode) const {
    ReasonCode reason_code = ReasonCode::BMP_OK;

    if (m_artId.empty()) {
        if (mode.fallback) {
            reason_code = ReasonCode::BMP_NO_ART_ID;
            wxBitmap bmp = mode.fallback(width, height, client, reason_code);
            assert(bmp.IsOk());
            return bmp;
        }
        return ReasonCode::BMP_NO_ART_ID;
    }

    bool match_exactly = mode.exactly;
    bool match_best = !match_exactly;
    bool assets_preferred = mode.assets_preferred;
    bool stockart_preferred = !assets_preferred;

    if (stockart_preferred) {
        wxBitmap bmp = wxArtProvider::GetBitmap(m_artId, client, wxSize(width, height));
        if (bmp.IsOk())
            return bmp;
        reason_code = ReasonCode::BMP_BAD_ART;
    }

    std::string path = wxArtProviders::getAlternativeArtAssetPath(m_artId);
    if (!path.empty()) {
        wxBitmap bmp = wxArtProvider::GetBitmap(path, client, wxSize(width, height));
        if (bmp.IsOk())
            return bmp;
    }

    // no alt-path or alt-bmp is bad.
    ImageSet alt(path);
    BitmapMode alt_mode{
        .no_stockart = true,
        .no_asset = false,
        .assets_preferred = mode.assets_preferred,
        .exactly = false,
        .include_raw = mode.include_raw,
        .translate = mode.translate,
        .fallback = mode.fallback,
    };
    BitmapResult result = alt.toBitmap(width, height, client, alt_mode);
    if (result.isOk())
        return result;
    if (reason_code == BMP_OK)
        reason_code = ReasonCode::BMP_BAD_ALT;
    else
        reason_code = ReasonCode::BMP_BAD_ART_ALT;

    if (mode.fallback) {
        wxBitmap bmp = mode.fallback(width, height, client, reason_code);
        assert(bmp.IsOk());
        return bmp;
    }
    return reason_code;
}

BitmapResult ImageSet::toBitmap1(int width, int height, const wxArtClient& client,
                             const BitmapMode& mode) const {
    ReasonCode reason_code = ReasonCode::BMP_OK;
    BitmapResult result = toBitmap(width, height, client, mode);
    if (result.isOk())
        return result;
    return bitmapWithReason(width, height, client, mode, reason_code);
}

BitmapResult ImageSet::_bitmapFromArt1(int width, int height, const wxArtClient& client,
                                   const BitmapMode& mode) const {
    ReasonCode reason_code = ReasonCode::BMP_OK;
    BitmapResult result = _bitmapFromArt(width, height, client, mode);
    if (result.isOk())
        return result;
    return bitmapWithReason(width, height, client, mode, result.getReason());
}

wxBitmap bitmapWithReason(int width, int height, const wxArtClient& client, const BitmapMode& mode,
                          ReasonCode reason_code) {

    wxBitmap bmp(width, height, 24);
    wxMemoryDC dc(bmp);
    dc.SetBackground(wxBrush(mode.m_backcolor));
    dc.Clear();
    dc.SetPen(wxPen(mode.m_border));
    dc.DrawRectangle(0, 0, width, height);

    std::string drawCode = "?";
    switch (reason_code) {
    case ReasonCode::BMP_OK:
        drawCode = "OK";
        break;
    case ReasonCode::BMP_NO_ART_ID:
        drawCode = "NA";
        break;
    case ReasonCode::BMP_DISABLED:
        drawCode = "DE";
        break;
    case ReasonCode::BMP_NONE:
        drawCode = "NN";
        break;
    case ReasonCode::BMP_BAD_ART:
        drawCode = "BA";
        break;
    case ReasonCode::BMP_BAD_ALT:
        drawCode = "BT";
        break;
    case ReasonCode::BMP_BAD_ART_ALT:
        drawCode = "AT";
        break;
    case ReasonCode::BMP_BAD_ASSET:
        drawCode = "EF";
        break;
    default:
        // if reason_code is printable char
        if (reason_code >= 32 && reason_code <= 126) {
            drawCode = std::to_string((char) reason_code);
        }
    }

    if (!drawCode.empty()) {
        // set font size to fit the bitmap
        int fontSize = std::min(width, height) / 2;
        dc.SetFont(wxFont(fontSize, wxFONTFAMILY_DEFAULT, wxFONTWEIGHT_NORMAL, wxFONTSTYLE_NORMAL));
        // center the text
        wxSize extent = dc.GetTextExtent(drawCode);
        int left = (width - extent.GetWidth()) / 2;
        int top = (height - extent.GetHeight()) / 2;
        // set text color
        dc.SetTextForeground(mode.m_color);
        dc.DrawText(drawCode, left, top);
    }
    return bmp;
}

void ImageSet::dump(std::ostream& os) const {
    os << "ImageSet: " << m_text << " " << m_artId << " " << m_asset->str() << std::endl;
    for (const auto& [size, path] : m_scaledAssets) {
        os << "  " << size.first << "x" << size.second << " " << path.str() << std::endl;
    }
}
