#include "ui/arch/ImageSet.hpp"

#include <bas/proc/AssetsRegistry.hpp>
#include <bas/volume/Volume.hpp>

#include <iostream>

int main(int argc, char** argv) {
    argc--;
    argv++;

    std::string dir = "streamline-vectors/core/pop/interface-essential";
    ImageSet icon(wxART_NEW, dir, "new-file.svg");
    // icon.detect();
    icon.dump(std::cout);

    const char* options = NULL;
    if (argc > 0 && argv[0][0] == '-') {
        options = argv[0] + 1;
        argc--;
        argv++;
    }

    const char* path = "/";
    if (argc > 0) {
        path = argv[0];
        argc--;
        argv++;
    }

    Volume* vol = AssetsRegistry::instance().get();
    if (!vol) {
        std::cerr << "No asset volume (g_assets / bas_ui_assets)\n";
        return 1;
    }
    if (options) {
        std::cout << "Assets list:" << std::endl;
        vol->ls(options, path);
    } else {
        std::cout << "Assets tree:" << std::endl;
        vol->tree(path);
    }
    return 0;
}
