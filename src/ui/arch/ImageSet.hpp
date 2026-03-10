#ifndef UI_IMAGESET_H
#define UI_IMAGESET_H

#include <bas/util/Path.hpp>

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/defs.h>
#include <wx/string.h>

#include <map>
#include <optional>

class ScaledAsset {
public:
    int width;
    int height;
    Path path;

    ScaledAsset(int width, int height, const Path& path)
        : width(width), height(height), path(path) {}
    
    ScaledAsset& operator=(const ScaledAsset& other) {
        width = other.width;
        height = other.height;
        path = other.path;
        return *this;
    }

    bool operator==(const ScaledAsset& other) const {
        return width == other.width && height == other.height && path == other.path;
    }
    bool operator!=(const ScaledAsset& other) const { return !(*this == other); }
};

/**
 * Icon/image set: optional wx art id, optional default asset path,
 * and optional scaled assets (w,h) -> Path for size-specific images.
 */
class ImageSet {
public:
    static const ImageSet EMPTY;

    ImageSet(wxArtID artId = wxString(), std::optional<Path> asset = std::nullopt);
    explicit ImageSet(std::optional<Path> asset);

    bool empty() const;
    bool isSet() const;

    bool operator==(const ImageSet& other) const;
    bool operator!=(const ImageSet& other) const { return !(*this == other); }

    ImageSet& artId(wxArtID id);
    ImageSet& asset(std::optional<Path> p);
    ImageSet& scale(int w, int h, const Path& p);

    wxArtID getArtId() const { return m_artId; }
    std::optional<Path> getAsset() const { return m_asset; }
    /** Find best asset path for size (w,h): exact match, else default asset. */
    std::optional<ScaledAsset> findAsset(int w, int h) const;
    /** Load bitmap at size (w,h): art id, then scaled asset, then default asset. Returns empty bitmap on failure. */
    wxBitmap loadBitmap(int w, int h) const;

private:
    wxArtID m_artId;
    std::optional<Path> m_asset;
    int m_asset_width;
    int m_asset_height;
    std::map<std::pair<int, int>, Path> m_scaledAssets;
};

#endif // UI_IMAGESET_H
