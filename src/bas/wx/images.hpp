#ifndef WX_IMAGES_H
#define WX_IMAGES_H

#include <wx/bitmap.h>
#include <wx/image.h>

#include <cstdint>
#include <optional>

std::optional<wxBitmap> imageLoadAsset(const std::string& path, int width, int height);

std::optional<wxBitmap> imageLoadFile(const wxString& path, int width, int height);

std::optional<wxBitmap> imageLoad(const uint8_t* data, size_t size, std::string_view format,
                                  int width, int height,
                                  const std::string_view path_info = "path unknown");

std::optional<wxImage> renderSvgToWxImage(const uint8_t* data, size_t size, int w, int h);

#endif // WX_IMAGES_H