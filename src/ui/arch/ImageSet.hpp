#ifndef UI_IMAGESET_H
#define UI_IMAGESET_H

#include <bas/util/Path.hpp>
#include <bas/volume/Volume.hpp>

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/defs.h>
#include <wx/string.h>

#include <functional>
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

struct BitmapMode {
    using translate_fn = std::function<std::string(const std::string& path)>;
    using fallback_fn = std::function<wxBitmap(int width, int height, const wxArtClient& client)>;

    // BitmapMode() = default;
    // BitmapMode(bool no_stockart, bool no_asset, bool assets_preferred, bool exactly,
    //            translate_fn translate, fallback_fn fallback)
    //     : no_stockart(no_stockart), no_asset(no_asset), assets_preferred(assets_preferred),
    //       exactly(exactly), translate(translate), fallback(fallback) {}

    // BitmapMode& operator=(const BitmapMode& other) {
    //     no_stockart = other.no_stockart;
    //     no_asset = other.no_asset;
    //     assets_preferred = other.assets_preferred;
    //     exactly = other.exactly;
    //     translate = other.translate;
    //     fallback = other.fallback;
    //     return *this;
    // }

    bool no_stockart = false;
    bool no_asset = false;
    bool assets_preferred = false;
    bool exactly = false;

    translate_fn translate = nullptr;
    fallback_fn fallback = nullptr;

    static const BitmapMode DEFAULT;
};

/**
 * Icon/image set: optional wx art id, optional default asset path,
 * and optional scaled assets (w,h) -> Path for size-specific images.
 */
class ImageSet {
  public:
    static const ImageSet EMPTY;

    ImageSet(wxArtID artId = wxString(), std::optional<Path> asset = std::nullopt,
             std::string text = "");
    ImageSet(wxArtID artId, std::string dir, std::string name, std::string text = "");
    explicit ImageSet(std::optional<Path> asset, std::string text = "");

    ImageSet& detect(Volume* volume = nullptr);

    bool empty() const;
    bool isSet() const;

    bool operator==(const ImageSet& other) const;
    bool operator!=(const ImageSet& other) const { return !(*this == other); }

    ImageSet& artId(wxArtID id);
    ImageSet& asset(std::optional<Path> p);
    ImageSet& scale(int w, int h, const Path& p);

    wxArtID getArtId() const { return m_artId; }
    std::optional<Path> getAsset() const { return m_asset; }

    std::optional<ScaledAsset> findBestMatch(int width, int height,
                                             const wxArtClient& client = wxART_FRAME_ICON) const;

    std::optional<ScaledAsset> findExactly(int width, int height,
                                           const wxArtClient& client = wxART_FRAME_ICON) const;

    std::optional<std::string>
    findBestMatchAssetPath(int width, int height,
                           const wxArtClient& client = wxART_FRAME_ICON) const;

    std::optional<std::string>
    findExactlyAssetPath(int width, int height, const wxArtClient& client = wxART_FRAME_ICON) const;

    std::optional<wxBitmap> toBitmap(int width, int height,
                                     const wxArtClient& client = wxART_FRAME_ICON,
                                     const BitmapMode& mode = BitmapMode::DEFAULT) const;

    std::optional<wxBitmap> bitmapFromArt(int width, int height,
                                          const wxArtClient& client = wxART_FRAME_ICON,
                                          const BitmapMode& mode = BitmapMode::DEFAULT) const;

    wxBitmap toBitmap1(int width, int height, const wxArtClient& client = wxART_FRAME_ICON,
                       const BitmapMode& mode = BitmapMode::DEFAULT) const;

    wxBitmap bitmapFromArt1(int width, int height, const wxArtClient& client = wxART_FRAME_ICON,
                            const BitmapMode& mode = BitmapMode::DEFAULT) const;

    void dump(std::ostream& os) const;

  private:
    std::string m_text;
    wxArtID m_artId;
    std::optional<Path> m_asset;
    int m_asset_width;
    int m_asset_height;
    std::map<std::pair<int, int>, Path> m_scaledAssets;

    void init();
};

#endif // UI_IMAGESET_H
