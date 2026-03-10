#ifndef WX_ASSETS_H
#define WX_ASSETS_H

#include <wx/artprov.h>
#include <wx/bitmap.h>

/// Load a PNG from embedded zip; path is full path within assets/.
/// Returns bitmap (placeholder if load fails).
wxBitmap loadBitmapFromAsset(std::string_view path, int width = 24, int height = 24);

wxBitmap wxArtProvider_GetBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size = wxSize(24, 24));

#endif // WX_ASSETS_H
