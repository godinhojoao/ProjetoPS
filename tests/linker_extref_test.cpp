#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include "../src/linker/extref_resolver.h"
#include "../src/linker/global_symbol_table.h"

void createMockObjectFile(const std::string& path) {
    std::ofstream file(path);
    file << "; Mock arquivo objeto para testes de EXTREF\n";
    file << "HEADER\n";
    file << "MODULE: test_extref\n";
    file << "SIZE: 10\n\n";
    file << "EXTDEF\n\n";
    file << "EXTREF\n";
    file << "SOMA 0x0002\n";
    file << "MULTIPLICA 0x0005 0x0008\n";
    file << "NAO_EXISTE 0x000A\n";
    file << "\nREALOC\n";
    file << "CODE\n";
    file.close();
}

void testParsing() {
    std::string path = "mock_extref.o";
    createMockObjectFile(path);

    std::vector<ExtRef> refs = ExtRefResolver::parseExtRefsFromFile(path);

    assert(refs.size() == 3);
    
    assert(refs[0].symbolName == "SOMA");
    assert(refs[0].offsets.size() == 1);
    assert(refs[0].offsets[0] == 0x0002);

    assert(refs[1].symbolName == "MULTIPLICA");
    assert(refs[1].offsets.size() == 2);
    assert(refs[1].offsets[0] == 0x0005);
    assert(refs[1].offsets[1] == 0x0008);
    
    assert(refs[2].symbolName == "NAO_EXISTE");
    assert(refs[2].offsets.size() == 1);
    assert(refs[2].offsets[0] == 0x000A);

    std::cout << "[OK] testParsing passou.\n";
    remove(path.c_str());
}

void testResolution() {
    GlobalSymbolTable gts;
    std::vector<ExtDef> defs = {
        {"SOMA", 0x1000},
        {"MULTIPLICA", 0x1234}
    };
    gts.registerModuleSymbols("mock_math", 0x0000, defs); // Mapeado base 0, absoluto = relativo

    std::vector<ExtRef> refs = {
        {"SOMA", {0x0002}},
        {"MULTIPLICA", {0x0005}}
    };

    std::vector<uint8_t> codeBuffer(10, 0x00);
    std::vector<std::string> errors;

    bool result = ExtRefResolver::resolve(refs, gts, codeBuffer, errors);
    
    assert(result == true);
    assert(errors.empty());

    // SOMA: addr 0x1000 -> low 0x00, high 0x10 -> little endian
    assert(codeBuffer[0x0002] == 0x00);
    assert(codeBuffer[0x0003] == 0x10);

    // MULTIPLICA: addr 0x1234 -> low 0x34, high 0x12 -> little endian
    assert(codeBuffer[0x0005] == 0x34);
    assert(codeBuffer[0x0006] == 0x12);

    std::cout << "[OK] testResolution passou.\n";
}

void testResolutionErrors() {
    GlobalSymbolTable gts;
    
    std::vector<ExtRef> refs = {
        {"INEXISTENTE", {0x0000}}
    };

    std::vector<uint8_t> codeBuffer(10, 0x00);
    std::vector<std::string> errors;

    bool result = ExtRefResolver::resolve(refs, gts, codeBuffer, errors);
    
    assert(result == false);
    assert(errors.size() == 1);
    
    std::cout << "[OK] testResolutionErrors passou.\n";
}

int main() {
    std::cout << "Executando testes EXTREF...\n";
    testParsing();
    testResolution();
    testResolutionErrors();
    std::cout << "Todos os testes passaram.\n";
    return 0;
}
