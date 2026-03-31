#include "mem.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

void hexdump(const std::vector<uint8_t>& data, size_t size) {
    if (size == -1)
        size = data.size();
    // like hexdump: hex row and ascii preview
    for (size_t offset = 0; offset < size; offset += 16) {
        size_t num = std::min((size_t)16, size - offset);
        printf("%08lx ", offset);
        for (size_t j = 0; j < num; j++) {
            printf("%02x ", data[offset + j]);
        }
        for (int pad = num; pad < 16; pad++)
            printf("   ");
        printf(" |");
        for (size_t j = 0; j < num; j++) {
            printf("%c", isprint(data[offset + j]) ? (char)data[offset + j] : '.');
        }
        printf("|\n");
    }
}
