#include "wx_assets.hpp"

#include <bas/proc/Assets.hpp>
#include <bas/log/uselog.h>

#include <wx/image.h>
#include <wx/mstream.h>

#include <algorithm>
#include <cstdint>
#include <vector>

static void hexdump(const std::vector<uint8_t>& data, size_t size) {
    // like hexdump: hex row and ascii preview
    for (size_t offset = 0; offset < size; offset += 16) {
        size_t num = std::min((size_t) 16, size - offset);
        printf("%08lx ", offset);
        for (size_t j = 0; j < num; j++) {
            printf("%02x ", data[offset + j]);
        }
        for (int pad = num; pad < 16; pad++)
            printf("   ");
        printf(" |");
        for (size_t j = 0; j < num; j++) {
            printf("%c", isprint(data[offset + j]) ? (char)data[offset + j] : '.');
        }
        printf("|\n");
    }
}

wxBitmap loadBitmapFromAsset(std::string_view path, int width, int height) {
    std::vector<uint8_t> data = assets_get_data(path);
    if (data.empty()) {
        logerror_fmt("No asset: %s", path.data());
        // assets_dump_tree();
        return wxBitmap(width > 0 ? width : 24, height > 0 ? height : 24);
    }

    wxMemoryInputStream stream(data.data(), data.size());
    wxImage image;
    if (!image.LoadFile(stream)) {
        logerror_fmt("Failed to load bitmap from asset: %s, use empty image", path.data());
        
        // dump header of data
        // loginfo_fmt("file %s size %d", path.data(), data.size());
        // size_t hdrsize = std::min((size_t)1024, data.size());
        // hexdump(data, hdrsize);
        
        return wxBitmap(width > 0 ? width : 24, height > 0 ? height : 24);
    }

    if (!image.IsOk()) {
        logerror_fmt("error loaded bitmap from asset: %s, use empty image", path.data());
        return wxBitmap(width > 0 ? width : 24, height > 0 ? height : 24);
    }

    if ((width > 0 && height > 0) && (image.GetWidth() != width || image.GetHeight() != height)) {
        loglog_fmt("Bitmap %s scaled from %dx%d to %dx%d", path.data(),
            image.GetWidth(), image.GetHeight(), width, height);
        image = image.Scale(width, height, wxIMAGE_QUALITY_BILINEAR);
    }

    wxBitmap bmp(image);
    if (!bmp.IsOk()) {
        logwarn_fmt("Failed to create bitmap from asset: %s, fallback to empty image", path.data());
        bmp = wxBitmap(width > 0 ? width : 24, height > 0 ? height : 24);
    }
    return bmp;
}

/// Returns asset path for a wxArtID when using embedded fallback (wx 3.2+).
static std::string _assetPath(const wxString& id) {
    static const std::map<std::string, std::string> map = {
        {"wxART_REFRESH", "interface-essential/line-arrow-reload-horizontal-1.png"},
        {"wxART_FULL_SCREEN", "interface-essential/line-arrow-expand-window-1.png"},
        {"wxART_REPORT_VIEW", "interface-essential/dashboard-3.png"},
        {"wxART_HELP_SETTINGS", "interface-essential/cog-1.png"},
        {"wxART_USER_MANAGEMENT", "interface-essential/user-multiple-group.png"},
        {"wxART_ACL_MANAGEMENT", "interface-essential/shield-check.png"},
        {"wxART_STORAGE_ANALYSIS", "computer-devices/hard-disk.png"},
    };
    std::string key(id.ToStdString());
    auto it = map.find(key);
    if (it == map.end())
        return "";
    std::string path = it->second;
    std::string style = "streamline-vectors/core/pop";
    path = style + "/" + path;
    return path;
}

wxBitmap wxArtProvider_GetBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size) {
    wxBitmap bmp = wxArtProvider::GetBitmap(id, client, size);
    if (bmp.IsOk()) return bmp;
    
    std::string path = _assetPath(id);
    if (!path.empty()) {
        wxBitmap fromAsset = loadBitmapFromAsset(path, size.GetWidth(), size.GetHeight());
        if (fromAsset.IsOk()) return fromAsset;
    }
    return wxBitmap(size.GetWidth(), size.GetHeight());
}
