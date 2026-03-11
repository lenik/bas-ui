#include "proc/Assets.hpp"

#include <iostream>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    std::cout << "Assets tree:" << std::endl;
    assets_dump_tree();
    return 0;
}

