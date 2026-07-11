#ifndef LINKER_H
#define LINKER_H

#include <cstdint>
#include <vector>
#include <string>
#include "global_symbol_table.h"
#include "extref_resolver.h"

struct RelocationEntry {
    uint16_t offset;
};

struct LinkerObjectFile {
    std::string moduleName;
    uint16_t size = 0;
    std::vector<ExtDef> extDefs;
    std::vector<ExtRef> extRefs;
    std::vector<uint16_t> relocs;
    std::vector<uint8_t> code;
};

class Linker {
public:
    static std::vector<RelocationEntry> scan(const uint8_t* code, uint32_t codeSize);

    static bool readObj(const std::string& path, LinkerObjectFile& objOut);
    static bool writeObj(const std::string& path, const LinkerObjectFile& obj);

    static bool link(const std::vector<std::string>& inputPaths, const std::string& outputPath);
};

#endif
