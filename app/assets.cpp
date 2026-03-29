#include "ui/arch/ImageSet.hpp"

#include <bas/proc/UseAssets.hpp>

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

    if (options) {
        std::cout << "Assets list:" << std::endl;
        bas_ui_assets->ls(options, path);
    } else {
        std::cout << "Assets tree:" << std::endl;
        bas_ui_assets->tree(path);
    }
    return 0;
}
