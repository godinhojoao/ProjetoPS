#include "linker.h"
#include <fstream>
#include <iostream>

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

            case 0x18: // JR offset
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

bool Linker::writeObj(const std::string& path,
                      const uint8_t* code, uint32_t codeSize,
                      const std::vector<RelocationEntry>& relocTable) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Linker: nao foi possivel criar '" << path << "'\n";
        return false;
    }

    // header: magic (4) + codeSize (2) + relocCount (2)
    file.write(reinterpret_cast<const char*>(OBJ_MAGIC), 4);

    uint16_t codeSz = static_cast<uint16_t>(codeSize);
    uint16_t relocCount = static_cast<uint16_t>(relocTable.size());
    file.write(reinterpret_cast<const char*>(&codeSz), 2);
    file.write(reinterpret_cast<const char*>(&relocCount), 2);

    // code section
    file.write(reinterpret_cast<const char*>(code), codeSize);

    // relocation table
    for (const auto& entry : relocTable) {
        file.write(reinterpret_cast<const char*>(&entry.offset), 2);
    }

    return file.good();
}

bool Linker::readObj(const std::string& path,
                     std::vector<uint8_t>& codeOut,
                     std::vector<RelocationEntry>& relocOut) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Linker: nao foi possivel abrir '" << path << "'\n";
        return false;
    }

    // valida magic
    uint8_t magic[4];
    file.read(reinterpret_cast<char*>(magic), 4);
    for (int i = 0; i < 4; ++i) {
        if (magic[i] != OBJ_MAGIC[i]) {
            std::cerr << "Linker: magic invalido em '" << path << "'\n";
            return false;
        }
    }

    uint16_t codeSz = 0;
    uint16_t relocCount = 0;
    file.read(reinterpret_cast<char*>(&codeSz), 2);
    file.read(reinterpret_cast<char*>(&relocCount), 2);

    // code section
    codeOut.resize(codeSz);
    file.read(reinterpret_cast<char*>(codeOut.data()), codeSz);

    // relocation table
    relocOut.resize(relocCount);
    for (uint16_t i = 0; i < relocCount; ++i) {
        file.read(reinterpret_cast<char*>(&relocOut[i].offset), 2);
    }

    if (!file.good()) {
        std::cerr << "Linker: erro ao ler '" << path << "'\n";
        return false;
    }

    return true;
}
