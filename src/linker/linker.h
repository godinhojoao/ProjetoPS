#ifndef LINKER_H
#define LINKER_H

#include <cstdint>
#include <vector>
#include <string>

struct RelocationEntry {
    uint16_t offset;
};

class Linker {
public:
    static std::vector<RelocationEntry> scan(const uint8_t* code, uint32_t codeSize);

    static bool writeObj(const std::string& path,
                         const uint8_t* code, uint32_t codeSize,
                         const std::vector<RelocationEntry>& relocTable);

    static bool readObj(const std::string& path,
                        std::vector<uint8_t>& codeOut,
                        std::vector<RelocationEntry>& relocOut);
};

#endif
