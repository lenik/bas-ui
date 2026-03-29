#include "images.hpp"

#include <bas/log/uselog.h>
#include <bas/proc/UseAssets.hpp>
#include <bas/util/Path.hpp>

#define NANOSVG_IMPLEMENTATION
#include <nanosvg/nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg/nanosvgrast.h>

#include <wx/bitmap.h>
#include <wx/file.h>
#include <wx/image.h>
#include <wx/log.h>
#include <wx/mstream.h>
#include <wx/string.h>

#include <optional>
#include <vector>

std::optional<wxBitmap> imageLoadAsset(const std::string& path, int width, int height) {
    logdebug_fmt("Loading bitmap from asset: %s", path.c_str());
    std::vector<uint8_t> data = bas_ui_assets->readFile(path);
    if (data.empty()) {
        logerror_fmt("No asset: %s", path.c_str());
        return std::nullopt;
    }

    std::string extension;
    int last_dot = path.find_last_of('.');
    if (last_dot != std::string::npos) {
        extension = path.substr(last_dot + 1);
    }

    if (data.empty()) {
        logerror_fmt("No asset: %s", path.c_str());
        return std::nullopt;
    }
    return imageLoad(data.data(), data.size(), extension, width, height, path);
}

std::optional<wxBitmap> imageLoadFile(const wxString& path, int width, int height) {
    std::string path_str = path.ToStdString();
    wxFile file(path);
    if (!file.IsOpened()) {
        logerror_fmt("Failed to open file: %s", path_str.c_str());
        return std::nullopt;
    }

    std::string extension;
    int last_dot = path_str.find_last_of('.');
    if (last_dot != std::string::npos) {
        extension = path_str.substr(last_dot + 1);
    }

    bool canRead = false;
    for (const auto& _handler : wxImage::GetHandlers()) {
        wxImageHandler* handler = dynamic_cast<wxImageHandler*>(_handler);
        if (handler && handler->CanRead(path)) {
            canRead = true;
            break;
        }
    }

    if (canRead) {
        wxImage image;
        if (!image.LoadFile(path)) {
            logerror_fmt("Failed to load file: %s", path_str.c_str());
            return std::nullopt;
        }
        if (!image.IsOk()) {
            logerror_fmt("Failed to load file: %s", path_str.c_str());
            return std::nullopt;
        }
        if ((width > 0 && height > 0) &&
            (image.GetWidth() != width || image.GetHeight() != height)) {
            loglog_fmt("Bitmap %s scaled from %dx%d to %dx%d", path_str.c_str(), //
                       image.GetWidth(), image.GetHeight(), width, height);
            image = image.Rescale(width, height, wxIMAGE_QUALITY_BILINEAR);
        }
        return std::make_optional(wxBitmap(image));
    }

    std::vector<uint8_t> data(file.Length());
    size_t read = file.Read(data.data(), data.size());
    if (read != data.size()) {
        logerror_fmt("Failed to read file: %s", path_str.c_str());
        return std::nullopt;
    }
    return imageLoad(data.data(), data.size(), extension, width, height, path_str.data());
}

std::optional<wxBitmap> imageLoad(const uint8_t* data, size_t size, std::string_view format,
                                  int width, int height, const std::string_view path_info) {
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("width and height must be positive");

    if (format == "svg") {
        logdebug_fmt("Rasterizing SVG: %s", path_info.data());
        std::optional<wxImage> img = renderSvgToWxImage(data, size, width, height);
        if (img && img->IsOk()) {
            wxBitmap bmp(*img);
            if (bmp.IsOk())
                return bmp;
        }
        logerror_fmt("Failed to rasterize SVG: %s", path_info.data());
        return std::nullopt;
    }

    else {
        logerror_fmt("Loading bitmap from asset: %s", path_info.data());
        wxMemoryInputStream stream(data, size);

        wxImage image;
        wxLogBuffer logBuf;
        wxLog* oldLog = wxLog::SetActiveTarget(&logBuf);
        bool loadStatus = image.LoadFile(stream);
        wxLog::SetActiveTarget(oldLog); // Restore original logger

        if (!loadStatus) {
            // This will contain the specific reason (e.g., "no handler for this type")
            wxString reason = logBuf.GetBuffer();
            logerror_fmt("Failed to load bitmap[%dx%d] from asset: %s, reason %s", //
                         width, height,
                         path_info.data(), //
                         reason.ToStdString().c_str());

            // dump header of data
            // loginfo_fmt("file %s size %d", path.data(), data.size());
            // size_t hdrsize = std::min((size_t)1024, data.size());
            // hexdump(data, hdrsize);
            return std::nullopt;
        }
        if (!image.IsOk()) {
            logerror_fmt("error loaded bitmap from asset: %s, use empty image", path_info.data());
            return std::nullopt;
        }

        if ((width > 0 && height > 0) &&
            (image.GetWidth() != width || image.GetHeight() != height)) {
            loglog_fmt("Bitmap %s scaled from %dx%d to %dx%d", path_info.data(), //
                       image.GetWidth(), image.GetHeight(), width, height);
            image = image.Rescale(width, height, wxIMAGE_QUALITY_BILINEAR);
        }

        wxBitmap bmp(image);
        return bmp;
    }
}

// Rasterize SVG to RGBA then to wxImage (for wx 3.0 without built-in SVG support).
std::optional<wxImage> renderSvgToWxImage(const uint8_t* data, size_t size, int w, int h) {
    if (w <= 0 || h <= 0)
        return std::nullopt;
    std::vector<char> buf(data, data + size);
    buf.push_back('\0');
    NSVGimage* image = nsvgParse(buf.data(), "px", 96.f);
    if (!image)
        return std::nullopt;
    float scale = 1.f;
    float tx = 0.f, ty = 0.f;
    if (image->width > 0 && image->height > 0) {
        scale =
            std::min(static_cast<float>(w) / image->width, static_cast<float>(h) / image->height);
        tx = (w - image->width * scale) * 0.5f;
        ty = (h - image->height * scale) * 0.5f;
    }
    std::vector<unsigned char> rgba(static_cast<size_t>(w) * h * 4);
    NSVGrasterizer* rast = nsvgCreateRasterizer();
    nsvgRasterize(rast, image, tx, ty, scale, rgba.data(), w, h, w * 4);
    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);

    unsigned char* rgb = static_cast<unsigned char*>(std::malloc(static_cast<size_t>(w) * h * 3));
    unsigned char* alpha = static_cast<unsigned char*>(std::malloc(static_cast<size_t>(w) * h));
    if (!rgb || !alpha) {
        std::free(rgb);
        std::free(alpha);
        return std::nullopt;
    }
    for (int i = 0; i < w * h; i++) {
        rgb[i * 3 + 0] = rgba[i * 4 + 0];
        rgb[i * 3 + 1] = rgba[i * 4 + 1];
        rgb[i * 3 + 2] = rgba[i * 4 + 2];
        alpha[i] = rgba[i * 4 + 3];
    }
    wxImage img(w, h);
    img.SetData(rgb);
    img.SetAlpha(alpha);
    return img;
}
