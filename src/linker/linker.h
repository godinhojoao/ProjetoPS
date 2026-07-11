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
    // Mantido para retrocompatibilidade / uso futuro, caso necessário
    static std::vector<RelocationEntry> scan(const uint8_t* code, uint32_t codeSize);

    // Lê e escreve arquivos objeto ASCII (.o)
    static bool readObj(const std::string& path, LinkerObjectFile& objOut);
    static bool writeObj(const std::string& path, const LinkerObjectFile& obj);

    // Une múltiplos arquivos .o, resolve EXTREF com GTS, calcula offsets, e aplica realocações
    static bool link(const std::vector<std::string>& inputPaths, const std::string& outputPath);
};

#endif
