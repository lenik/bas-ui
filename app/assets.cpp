#include "proc/Assets.hpp"
#include "ui/arch/ImageSet.hpp"
#include "wx/artprov.h"

#include <iostream>

int main(int argc, char** argv) {

    std::string dir = "streamline-vectors/core/pop/interface-essential";
    ImageSet icon(wxART_NEW, dir, "new-file.svg");
    // icon.detect();
    icon.dump(std::cout);

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            std::string dir = argv[i];
            if (i != 1)
                std::cout << std::endl;
            std::cout << "Listing directory: " << dir << std::endl;
            auto files = assets_listdir(dir);
            for (const auto& file : files) {
                std::cout << file->name << " " << file->size << std::endl;
            }
        }
    } else {
        std::cout << "Assets tree:" << std::endl;
        assets_dump_tree();
    }
    return 0;
}
