#include "ImageSet.hpp"

#include "../../wx/artprovs.hpp"
#include "../../wx/images.hpp"

#include <bas/log/uselog.h>
#include <bas/proc/UseAssets.hpp>
#include <bas/util/Path.hpp>

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/mstream.h>
#include <wx/string.h>

#include <algorithm>
#include <cstdlib>
#include <optional>

auto ident = [](const std::string& path) { return path; };

auto blank_bitmap = [](int width, int height, const wxArtClient& client, int reason_code) {
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

ImageSet::ImageSet(std::optional<Path> asset, std::string text) : m_artId(), m_asset(asset) {
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
        Volume* memzip = g_assets;
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

std::optional<wxBitmap> ImageSet::toBitmap(int width, int height, const wxArtClient& client,
                                           const BitmapMode& mode, int* reason_var) const {
    bool use_stockart = !mode.no_stockart && !m_artId.empty();
    bool use_asset = !mode.no_asset && m_asset;
    bool match_exactly = mode.exactly;
    bool match_best = !match_exactly;
    bool assets_preferred = mode.assets_preferred;
    bool stockart_preferred = !assets_preferred;

    int reason_code = 0;

    if (use_stockart && stockart_preferred) {
        std::optional<wxBitmap> bmp = _bitmapFromArt(width, height, client, mode);
        if (bmp && bmp->IsOk())
            return bmp;
        reason_code = ReasonCode::BMP_BAD_ART_ALT;
    }

    do {
        if (!use_asset)
            break;

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
        return bmp;
    } while (false);

    if (use_stockart && !stockart_preferred) {
        std::optional<wxBitmap> bmp = _bitmapFromArt(width, height, client, mode, &reason_code);
        if (bmp && bmp->IsOk())
            return bmp;
    }

    if (mode.fallback) {
        wxBitmap bmp = mode.fallback(width, height, client, reason_code);
        assert(bmp.IsOk());
        return bmp;
    }
    return std::nullopt;
}

std::optional<wxBitmap> ImageSet::_bitmapFromArt(int width, int height, const wxArtClient& client,
                                                 const BitmapMode& mode, int* reason_var) const {
    int reason_code = 0;

    if (m_artId.empty()) {
        if (mode.fallback) {
            reason_code = ReasonCode::BMP_NO_ART_ID;
            wxBitmap bmp = mode.fallback(width, height, client, reason_code);
            assert(bmp.IsOk());
            return bmp;
        }
        if (reason_var)
            *reason_var = ReasonCode::BMP_NO_ART_ID;
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
        reason_code = ReasonCode::BMP_BAD_ART;
    }

    std::string path = wxArtProviders::getAlternativeArtAssetPath(m_artId);
    if (!path.empty()) {
        wxBitmap bmp = wxArtProvider::GetBitmap(path, client, wxSize(width, height));
        if (bmp.IsOk())
            return bmp;
    }

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
    std::optional<wxBitmap> bmp = alt.toBitmap(width, height, client, alt_mode);
    if (bmp && bmp->IsOk())
        return bmp;
    if (reason_code == 0)
        reason_code = ReasonCode::BMP_BAD_ALT;
    else
        reason_code = ReasonCode::BMP_BAD_ART_ALT;
    if (reason_var)
        *reason_var = reason_code;

    if (mode.fallback) {
        wxBitmap bmp = mode.fallback(width, height, client, reason_code);
        assert(bmp.IsOk());
        return bmp;
    }
    return std::nullopt;
}

wxBitmap ImageSet::toBitmap1(int width, int height, const wxArtClient& client,
                             const BitmapMode& mode) const {
    int reason_code = 0;
    std::optional<wxBitmap> bmp = toBitmap(width, height, client, mode, &reason_code);
    if (bmp && bmp->IsOk())
        return *bmp;
    return bitmapWithReason(width, height, client, mode, reason_code);
}

wxBitmap ImageSet::_bitmapFromArt1(int width, int height, const wxArtClient& client,
                                   const BitmapMode& mode) const {
    int reason_code = 0;
    std::optional<wxBitmap> bmp = _bitmapFromArt(width, height, client, mode, &reason_code);
    if (bmp && bmp->IsOk())
        return *bmp;
    return bitmapWithReason(width, height, client, mode, reason_code);
}

wxBitmap bitmapWithReason(int width, int height, const wxArtClient& client, const BitmapMode& mode,
                          int reason_code) {

    wxBitmap bmp(width, height, 24);
    wxMemoryDC dc(bmp);
    dc.SetBackground(wxBrush(mode.m_backcolor));
    dc.Clear();
    dc.SetPen(wxPen(mode.m_border));
    dc.DrawRectangle(0, 0, width, height);

    char drawChar = 0;
    switch (reason_code) {
    case 0:
        drawChar = '-';
        break;
    case ReasonCode::BMP_NO_ART_ID:
        drawChar = 'I';
        break;
    case ReasonCode::BMP_BAD_ART:
        drawChar = 'A';
        break;
    case ReasonCode::BMP_BAD_ALT:
        drawChar = 'B';
        break;
    case ReasonCode::BMP_BAD_ART_ALT:
        drawChar = 'C';
        break;
    case ReasonCode::BMP_BAD_ASSET:
        drawChar = 'Z';
        break;
    default:
        // if reason_code is printable char
        if (reason_code >= 32 && reason_code <= 126) {
            drawChar = (char)reason_code;
        } else {
            drawChar = '?';
        }
    }

    if (drawChar) {
        // set font size to fit the bitmap
        int fontSize = std::min(width, height) / 2;
        dc.SetFont(wxFont(fontSize, wxFONTFAMILY_DEFAULT, wxFONTWEIGHT_NORMAL, wxFONTSTYLE_NORMAL));
        // center the text
        wxSize extent = dc.GetTextExtent(wxString(1, drawChar));
        int left = (width - extent.GetWidth()) / 2;
        int top = (height - extent.GetHeight()) / 2;
        // set text color
        dc.SetTextForeground(mode.m_color);
        dc.DrawText(wxString(1, drawChar), left, top);
    }
    return bmp;
}

void ImageSet::dump(std::ostream& os) const {
    os << "ImageSet: " << m_text << " " << m_artId << " " << m_asset->str() << std::endl;
    for (const auto& [size, path] : m_scaledAssets) {
        os << "  " << size.first << "x" << size.second << " " << path.str() << std::endl;
    }
}
