#include "linker.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "../shared/shared.h"

static constexpr uint8_t OBJ_MAGIC[4] = {0x5A, 0x38, 0x30, 0x4C}; // "Z80L"

std::vector<RelocationEntry> Linker::scan(const uint8_t* code, uint32_t codeSize) {
    std::vector<RelocationEntry> table;
    uint32_t pc = 0;

    while (pc < codeSize) {
        uint8_t opcode = code[pc];

        switch (opcode) {
            case 0xC3: // JP addr
                table.push_back({static_cast<uint16_t>(pc + 1)});
                pc += 3;
                break;

            case 0xCD: // CALL addr
                table.push_back({static_cast<uint16_t>(pc + 1)});
                pc += 3;
                break;

            case 0x3A: // LD A, (nn)
                table.push_back({static_cast<uint16_t>(pc + 1)});
                pc += 3;
                break;

            case 0x32: // LD (nn), A
                table.push_back({static_cast<uint16_t>(pc + 1)});
                pc += 3;
                break;

            case 0xDD: // prefixo IX
            case 0xFD: // prefixo IY
                pc += 3;
                break;

            case 0x18: // JR offset
            case 0xC6: // ADD A, n
            case 0xD6: // SUB n
            case 0xE6: // AND n
            case 0xEE: // XOR n
            case 0xF6: // OR n
            case 0xFE: // CP n
                pc += 2;
                break;

            case 0x76: // HALT
                pc += 1;
                break;

            default:
                if ((opcode & 0b11000111) == 0b00000110) {
                    pc += 2; // LD r, n
                } else {
                    pc += 1;
                }
                break;
        }
    }
    return table;
}

bool Linker::writeObj(const std::string& path, const LinkerObjectFile& obj) {
    std::ofstream file(path);
    if (!file) {
        std::cerr << "Linker: nao foi possivel criar '" << path << "'\n";
        return false;
    }

    file << "HEADER\n";
    file << "MODULE: " << obj.moduleName << "\n";
    file << "SIZE: " << obj.size << "\n\n";

    if (!obj.extDefs.empty()) {
        file << "EXTDEF\n";
        for (const auto& def : obj.extDefs) {
            file << def.symbolName << " 0x" 
                 << std::hex << std::uppercase << std::setfill('0') << std::setw(4) 
                 << def.relativeAddress << std::dec << "\n";
        }
        file << "\n";
    }

    if (!obj.extRefs.empty()) {
        file << "EXTREF\n";
        for (const auto& ref : obj.extRefs) {
            file << ref.symbolName;
            for (uint16_t offset : ref.offsets) {
                file << " 0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) 
                     << offset << std::dec;
            }
            file << "\n";
        }
        file << "\n";
    }

    if (!obj.relocs.empty()) {
        file << "REALOC\n";
        for (uint16_t offset : obj.relocs) {
            file << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) 
                 << offset << std::dec << "\n";
        }
        file << "\n";
    }

    file << "CODE\n";
    for (size_t i = 0; i < obj.code.size(); ++i) {
        file << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
             << static_cast<int>(obj.code[i]) << std::dec;
        if (i + 1 < obj.code.size()) {
            file << " ";
        }
    }
    file << "\n";

    return file.good();
}

bool Linker::readObj(const std::string& path, LinkerObjectFile& objOut) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Linker: nao foi possivel abrir '" << path << "'\n";
        return false;
    }

    objOut = LinkerObjectFile{};
    ObjSection current = ObjSection::None;
    std::string rawLine;

    while (std::getline(file, rawLine)) {
        const std::string line = Shared::trim(rawLine);
        if (line.empty() || line[0] == ';') continue;

        const ObjSection keyword = Shared::toObjSection(line);
        if (keyword != ObjSection::None) {
            current = keyword;
            continue;
        }

        switch (current) {
            case ObjSection::Header: {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string field = Shared::trim(line.substr(0, colon));
                    std::string value = Shared::trim(line.substr(colon + 1));
                    if (field == "MODULE") objOut.moduleName = value;
                    else if (field == "SIZE") {
                        unsigned long size = 0;
                        if (Shared::tryParseNumber(value, 0, size)) objOut.size = static_cast<uint16_t>(size);
                    }
                }
                break;
            }
            case ObjSection::ExtDef: {
                std::istringstream stream(line);
                std::string name, addrToken;
                if (stream >> name >> addrToken) {
                    ExtDef def;
                    def.symbolName = name;
                    def.relativeAddress = Shared::parseAddress(addrToken);
                    objOut.extDefs.push_back(def);
                }
                break;
            }
            case ObjSection::ExtRef: {
                std::istringstream stream(line);
                std::string name;
                if (stream >> name) {
                    ExtRef ref;
                    ref.symbolName = name;
                    std::string offsetToken;
                    while (stream >> offsetToken) {
                        ref.offsets.push_back(Shared::parseAddress(offsetToken));
                    }
                    objOut.extRefs.push_back(ref);
                }
                break;
            }
            case ObjSection::Realoc: {
                std::istringstream stream(line);
                std::string token;
                while (stream >> token) {
                    unsigned long offset = 0;
                    if (Shared::tryParseNumber(token, 0, offset)) {
                        objOut.relocs.push_back(static_cast<uint16_t>(offset));
                    }
                }
                break;
            }
            case ObjSection::Code: {
                std::istringstream stream(line);
                std::string token;
                while (stream >> token) {
                    unsigned long byte = 0;
                    if (Shared::tryParseNumber(token, 16, byte)) {
                        objOut.code.push_back(static_cast<uint8_t>(byte));
                    }
                }
                break;
            }
            case ObjSection::None:
                break;
        }
    }
    return true;
}

bool Linker::link(const std::vector<std::string>& inputPaths, const std::string& outputPath) {
    std::vector<LinkerObjectFile> modules;
    for (const auto& path : inputPaths) {
        LinkerObjectFile mod;
        if (!readObj(path, mod)) {
            return false;
        }
        modules.push_back(mod);
    }

    GlobalSymbolTable gts;
    uint16_t currentOffset = 0;
    
    std::vector<uint16_t> moduleOffsets;
    for (const auto& mod : modules) {
        moduleOffsets.push_back(currentOffset);
        
        try {
            gts.registerModuleSymbols(mod.moduleName, currentOffset, mod.extDefs);
        } catch (const LinkerException& e) {
            std::cerr << "Erro no Linker (GTS): " << e.what() << "\n";
            return false;
        }
        
        currentOffset += mod.size;
    }

    LinkerObjectFile outputObj;
    outputObj.moduleName = "linked_output";
    outputObj.size = currentOffset;
    
    for (size_t i = 0; i < modules.size(); ++i) {
        auto& mod = modules[i];
        uint16_t baseOffset = moduleOffsets[i];
        
        std::vector<std::string> errors;
        if (!ExtRefResolver::resolve(mod.extRefs, gts, mod.code, errors)) {
            for (const auto& err : errors) {
                std::cerr << "Erro no modulo '" << mod.moduleName << "': " << err << "\n";
            }
            return false;
        }
        
        for (uint16_t reloc : mod.relocs) {
            outputObj.relocs.push_back(reloc + baseOffset);
        }
        
        outputObj.code.insert(outputObj.code.end(), mod.code.begin(), mod.code.end());
    }
    
    auto globalTable = gts.getTable();
    for (const auto& pair : globalTable) {
        outputObj.extDefs.push_back({pair.first, pair.second});
    }

    return writeObj(outputPath, outputObj);
}
