#include "extref_resolver.h"
#include "../shared/shared.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<ExtRef> ExtRefResolver::parseExtRefsFromFile(const std::string& objectFilePath) {
    std::vector<ExtRef> references;

    std::ifstream file(objectFilePath);
    if (!file) {
        return references;
    }

    std::string line;
    bool insideExtRef = false;

    while (std::getline(file, line)) {
        line = Shared::trim(line);

        // ignora linhas vazias e comentários
        if (line.empty() || line[0] == ';') {
            continue;
        }

        // encontrou a seção extref, começa a processar
        if (line == "EXTREF") {
            insideExtRef = true;
            continue;
        }

        // se estamos dentro de extref e encontramos outra seção, paramos
        if (insideExtRef && Shared::isSectionKeyword(line)) {
            break;
        }

        // dentro da seção extref: parseia nome e lista de offsets
        if (insideExtRef) {
            std::istringstream stream(line);
            std::string name;
            
            if (stream >> name) {
                ExtRef ref;
                ref.symbolName = name;
                
                std::string offsetToken;
                while (stream >> offsetToken) {
                    ref.offsets.push_back(Shared::parseAddress(offsetToken));
                }
                
                references.push_back(ref);
            }
        }
    }

    file.close();
    return references;
}

bool ExtRefResolver::resolve(
    const std::vector<ExtRef>& refs,
    const GlobalSymbolTable& gts,
    std::vector<uint8_t>& codeBuffer,
    std::vector<std::string>& errors
) {
    bool allResolved = true;

    for (const auto& ref : refs) {
        uint16_t address = 0;
        
        // busca símbolo na tabela global (gts)
        if (gts.lookupSymbol(ref.symbolName, address)) {
            for (uint16_t offset : ref.offsets) {
                if (offset + 1 < codeBuffer.size()) {
                    // escreve endereço em little-endian (byte baixo, byte alto)
                    codeBuffer[offset] = static_cast<uint8_t>(address & 0xFF);
                    codeBuffer[offset + 1] = static_cast<uint8_t>((address >> 8) & 0xFF);
                } else {
                    // offset fora do limite do buffer
                    errors.push_back("EXTREF: Offset fora dos limites para o simbolo '" + ref.symbolName + "'");
                    allResolved = false;
                }
            }
        } else {
            // símbolo externo não existe em nenhum módulo
            errors.push_back("EXTREF: Simbolo nao resolvido '" + ref.symbolName + "'");
            allResolved = false;
        }
    }

    return allResolved;
}
