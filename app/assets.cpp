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

    ListOptions opts;
    if (argc > 0 && argv[0][0] == '-') {
        opts = ListOptions::parse(argv[0] + 1);
        argc--;
        argv++;
    }

    const char* path = "/";
    if (argc > 0) {
        path = argv[0];
        argc--;
        argv++;
    }

    std::cout << "Assets list:" << std::endl;
    AssetsRegistry::instance()->ls(path, opts);
    return 0;
}
