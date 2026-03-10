#include "ImageSet.hpp"

#include "../../proc/wx_assets.hpp"

#include <wx/artprov.h>
#include <wx/string.h>

#include <optional>

const ImageSet ImageSet::EMPTY = ImageSet(wxString(), std::nullopt);

ImageSet::ImageSet(wxArtID artId, std::optional<Path> asset)
    : m_artId(artId), m_asset(asset)
{
}

ImageSet::ImageSet(std::optional<Path> asset) : m_artId(wxString()), m_asset(asset) {}

bool ImageSet::empty() const {
    return m_artId.empty() && !m_asset && m_scaledAssets.empty();
}

bool ImageSet::isSet() const {
    return !empty();
}

bool ImageSet::operator==(const ImageSet& other) const {
    return m_artId == other.m_artId
        && m_asset == other.m_asset
        && m_asset_width == other.m_asset_width
        && m_asset_height == other.m_asset_height
        && m_scaledAssets == other.m_scaledAssets;
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

std::optional<ScaledAsset> ImageSet::findAsset(int w, int h) const {
    std::optional<ScaledAsset> result = std::nullopt;
    bool found = false;
    int best_diff = size_diff(w, h, m_asset_width, m_asset_height);
    for (const auto& [size, path] : m_scaledAssets) {
        int diff = size_diff(w, h, size.first, size.second);
        if (diff < best_diff) {
            result = std::make_optional<ScaledAsset>(size.first, size.second, path);
            best_diff = diff;
            found = true;
        }
    }
    if (!found && m_asset) {
        result = std::make_optional<ScaledAsset>(m_asset_width, m_asset_height, *m_asset);
        found = true;
    }
    return result;
}

wxBitmap ImageSet::loadBitmap(int w, int h) const {
    if (!m_artId.empty()) {
        wxBitmap bmp = wxArtProvider_GetBitmap(m_artId, wxART_TOOLBAR, wxSize(w, h));
        if (bmp.IsOk())
            return bmp;
    }
    std::optional<ScaledAsset> sa = findAsset(w, h);
    if (!sa) {
        return wxBitmap();
    }
    wxBitmap bmp = loadBitmapFromAsset(sa->path.str(), w, h);
    if (bmp.IsOk())
        return bmp;

    // draw a blank bitmap
    wxBitmap blank = wxBitmap(w, h);
    return blank;
}
