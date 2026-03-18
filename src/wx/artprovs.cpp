#ifndef WX_ARTPROVS_HPP
#define WX_ARTPROVS_HPP

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/string.h>

#include <map>
#include <string>

namespace wxArtProviders {

static const std::map<std::string, std::string> alternativeArtIdMap = {
    {"wxART_REFRESH", "interface-essential/line-arrow-reload-horizontal-1.svg"},
    {"wxART_FULL_SCREEN", "interface-essential/line-arrow-expand-window-1.svg"},
    {"wxART_REPORT_VIEW", "interface-essential/dashboard-3.svg"},
    {"wxART_HELP_SETTINGS", "interface-essential/cog-1.svg"},
    {"wxART_USER_MANAGEMENT", "interface-essential/user-multiple-group.svg"},
    {"wxART_ACL_MANAGEMENT", "interface-essential/shield-check.svg"},
    {"wxART_STORAGE_ANALYSIS", "computer-devices/hard-disk.svg"},
};

/// Returns asset path for a wxArtID when using embedded fallback (wx 3.2+).
std::string getAlternativeArtAssetPath(const wxString& id) {

    std::string key(id.ToStdString());
    auto it = alternativeArtIdMap.find(key);
    if (it == alternativeArtIdMap.end())
        return "";
    std::string path = it->second;
    std::string style = "streamline-vectors/core/pop";
    path = style + "/" + path;
    return path;
}

} // namespace wxArtProviders

#endif // WX_ARTPROVS_HPP