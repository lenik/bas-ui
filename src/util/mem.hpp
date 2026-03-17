#ifndef UTIL_MEM_HPP
#define MEM_HPP

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

void hexdump(const std::vector<uint8_t>& data, size_t size = -1);

#endif // MEM_HPP