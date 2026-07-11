/*
GlobalSymbolTable (GTS):
Tabela global de símbolos do Ligador.
Responsável por registrar os símbolos exportados (EXTDEF) de cada módulo .o,
calcular seus endereços absolutos, e disponibilizá-los para a resolução
de referências externas (EXTREF) feita pelo Inácio.
*/

#include "global_symbol_table.h"
#include "../shared/shared.h"
#include <fstream>
#include <sstream>
#include <iostream>

void GlobalSymbolTable::registerModuleSymbols(
    const std::string& moduleName,
    uint16_t moduleStartAddress,
    const std::vector<ExtDef>& definitions
) {
    for (const auto& def : definitions) {
        // verifica se o símbolo já foi registrado por outro módulo
        if (symbolTable.count(def.symbolName)) {
            throw LinkerException(
                "Ligador: simbolo duplicado '" + def.symbolName +
                "' no modulo '" + moduleName +
                "' (ja definido por outro modulo)"
            );
        }

        // endereço absoluto = base do módulo + offset relativo do símbolo
        uint16_t absoluteAddress = moduleStartAddress + def.relativeAddress;
        symbolTable[def.symbolName] = absoluteAddress;
    }
}

bool GlobalSymbolTable::lookupSymbol(const std::string& symbolName, uint16_t& outAddress) const {
    auto it = symbolTable.find(symbolName);
    if (it == symbolTable.end()) {
        return false;
    }

    outAddress = it->second;
    return true;
}

std::vector<ExtDef> GlobalSymbolTable::parseExtDefsFromFile(const std::string& objectFilePath) {
    std::vector<ExtDef> definitions;

    std::ifstream file(objectFilePath);
    if (!file) {
        std::cerr << "GTS: nao foi possivel abrir '" << objectFilePath << "'\n";
        return definitions;
    }

    std::string line;
    bool insideExtDef = false;

    while (std::getline(file, line)) {
        line = Shared::trim(line);

        // ignora linhas vazias e comentários
        if (line.empty() || line[0] == ';') {
            continue;
        }

        // encontrou a seção EXTDEF, começa a processar
        if (line == "EXTDEF") {
            insideExtDef = true;
            continue;
        }

        // se estamos dentro de EXTDEF e encontramos outra seção, paramos
        if (insideExtDef && Shared::isSectionKeyword(line)) {
            break;
        }

        // dentro da seção EXTDEF: parseia "NOME 0xOFFSET"
        if (insideExtDef) {
            std::istringstream stream(line);
            std::string name;
            std::string addressToken;

            if (stream >> name >> addressToken) {
                ExtDef def;
                def.symbolName = name;
                def.relativeAddress = Shared::parseAddress(addressToken);
                definitions.push_back(def);
            }
        }
    }

    file.close();
    return definitions;
}

std::unordered_map<std::string, uint16_t> GlobalSymbolTable::getTable() const {
    return symbolTable;
}

void GlobalSymbolTable::clear() {
    symbolTable.clear();
}

void GlobalSymbolTable::printTable() const {
    std::cout << "\n=== Tabela Global de Simbolos (GTS) ===" << std::endl;

    if (symbolTable.empty()) {
        std::cout << "(vazia)" << std::endl;
        return;
    }

    for (const auto& entry : symbolTable) {
        std::cout << "  " << entry.first << " -> 0x"
                  << std::hex << std::uppercase << entry.second
                  << std::dec << std::endl;
    }

    std::cout << "=======================================" << std::endl;
}
