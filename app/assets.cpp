#include <bas/locale/i18n.h>
#include <bas/proc/AssetsRegistry.hpp>
#include <bas/ui/arch/ImageSet.hpp>
#include <bas/volume/overlay_ls.hpp>

#include <wx/artprov.h>
#include <wx/image.h>

#include <iostream>

void testLoadBitmap() {
    std::string dir = "streamline-vectors/core/pop/interface-essential";
    ImageSet icon(wxART_NEW, dir, "new-file.svg");
    // icon.detect();
    icon.dump(std::cout);

    std::cout << "load bitmap " << icon.getAsset()->str() << std::endl;
    wxInitAllImageHandlers();

    auto path = icon.findBestMatchAssetPath(32, 32);
    std::cout << "best match 32 path: " << *path << std::endl;

    auto bmp = icon.toBitmap(32, 32);
    if (bmp.isOk()) {
        std::cout << "Bitmap loaded from Path: " << *path << std::endl;
    } else {
        std::cout << "Failed to convert to bitmap: " << bmp.getReason() << std::endl;
    }
}

int main(int argc, char** argv) {
    init_i18n(argv[0],
        "bas-ui"
    );
    OverlayVolume* vol = AssetsRegistry::instance().get();
    overlay_ls(vol, argc, argv);
    return 0;
}
