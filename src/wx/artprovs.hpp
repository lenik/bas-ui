#ifndef WX_ARTPROVS_HPP
#define WX_ARTPROVS_HPP

#include <wx/string.h>

#include <string>

namespace wxArtProviders {

/// Returns asset path for a wxArtID when using embedded fallback (wx 3.2+).
std::string getAlternativeArtAssetPath(const wxString& id);

} // namespace wxArtProviders

#endif // WX_ARTPROVS_HPP