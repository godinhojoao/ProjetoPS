/*
Teste standalone da GlobalSymbolTable (GTS) do Ligador.

Cobre os seguintes casos:
  1. Parser de EXTDEF: lê arquivo .o mock e extrai os símbolos corretamente.
  2. Registro e lookup: calcula endereços absolutos com base no endereço do módulo.
  3. Múltiplos módulos: registra símbolos de módulos distintos com bases diferentes.
  4. Símbolo duplicado: verifica se LinkerException é disparada.
  5. Símbolo inexistente: lookupSymbol retorna false.

Não depende de CPU, VM, Memory ou Assembler — só da GTS.
*/

#include "../src/linker/global_symbol_table.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool writeTestObjectFile(const std::string& path, const std::string& content) {
    std::ofstream out(path);
    if (!out) return false;
    out << content;
    return out.good();
}

static int fail(const std::string& msg) {
    std::cerr << "[FAIL] " << msg << "\n";
    return 1;
}

int main() {
    // -------------------------------------------------------
    // caso 1: parser de EXTDEF a partir de arquivo .o mock
    // -------------------------------------------------------
    const std::string tmpPath = "tests/test_gts_mock.o";
    const std::string mockContent =
        "; Arquivo objeto de teste\n"
        "HEADER\n"
        "MODULE: test_mod\n"
        "SIZE: 20\n"
        "\n"
        "EXTDEF\n"
        "SOMA 0x0002\n"
        "SUBTRAI 0x0010\n"
        "\n"
        "EXTREF\n"
        "DIVIDE 0x0006\n"
        "\n"
        "REALOC\n"
        "0x000B\n"
        "\n"
        "CODE\n"
        "3E 05 CD 00 00 06 0C 76\n";

    if (!writeTestObjectFile(tmpPath, mockContent)) {
        return fail("nao foi possivel escrever arquivo .o mock em " + tmpPath);
    }

    std::vector<ExtDef> defs = GlobalSymbolTable::parseExtDefsFromFile(tmpPath);

    if (defs.size() != 2) {
        return fail("parser deveria retornar 2 EXTDEFs, retornou " + std::to_string(defs.size()));
    }

    if (defs[0].symbolName != "SOMA" || defs[0].relativeAddress != 0x0002) {
        return fail("primeiro EXTDEF incorreto: esperado SOMA 0x0002");
    }

    if (defs[1].symbolName != "SUBTRAI" || defs[1].relativeAddress != 0x0010) {
        return fail("segundo EXTDEF incorreto: esperado SUBTRAI 0x0010");
    }

    std::cout << "[OK] parser de EXTDEF OK (2 simbolos lidos)\n";

    // -------------------------------------------------------
    // caso 2: registro de símbolos e cálculo de endereço absoluto
    // -------------------------------------------------------
    GlobalSymbolTable gts;

    // módulo alocado em 0x0000
    gts.registerModuleSymbols("test_mod", 0x0000, defs);

    uint16_t addr = 0;
    if (!gts.lookupSymbol("SOMA", addr) || addr != 0x0002) {
        return fail("SOMA deveria estar em 0x0002, obteve 0x" + std::to_string(addr));
    }

    if (!gts.lookupSymbol("SUBTRAI", addr) || addr != 0x0010) {
        return fail("SUBTRAI deveria estar em 0x0010, obteve 0x" + std::to_string(addr));
    }

    std::cout << "[OK] registro e lookup com base 0x0000 OK\n";

    // -------------------------------------------------------
    // caso 3: múltiplos módulos com bases diferentes
    // -------------------------------------------------------
    gts.clear();

    // módulo A em 0x0000: exporta FUNC_A no offset 0x0005
    std::vector<ExtDef> defsA = {{"FUNC_A", 0x0005}};
    gts.registerModuleSymbols("mod_a", 0x0000, defsA);

    // módulo B em 0x0100: exporta FUNC_B no offset 0x000A
    std::vector<ExtDef> defsB = {{"FUNC_B", 0x000A}};
    gts.registerModuleSymbols("mod_b", 0x0100, defsB);

    if (!gts.lookupSymbol("FUNC_A", addr) || addr != 0x0005) {
        return fail("FUNC_A deveria estar em 0x0005");
    }

    // 0x0100 + 0x000A = 0x010A
    if (!gts.lookupSymbol("FUNC_B", addr) || addr != 0x010A) {
        return fail("FUNC_B deveria estar em 0x010A");
    }

    std::cout << "[OK] multiplos modulos com bases diferentes OK\n";

    // -------------------------------------------------------
    // caso 4: símbolo duplicado deve lançar exceção
    // -------------------------------------------------------
    bool exceptionThrown = false;
    try {
        // tenta registrar FUNC_A novamente em outro módulo
        std::vector<ExtDef> defsC = {{"FUNC_A", 0x0003}};
        gts.registerModuleSymbols("mod_c", 0x0200, defsC);
    } catch (const LinkerException&) {
        exceptionThrown = true;
    }

    if (!exceptionThrown) {
        return fail("deveria ter lancado LinkerException para simbolo duplicado FUNC_A");
    }

    std::cout << "[OK] simbolo duplicado detectado corretamente\n";

    // -------------------------------------------------------
    // caso 5: busca por símbolo inexistente deve retornar false
    // -------------------------------------------------------
    if (gts.lookupSymbol("NAO_EXISTE", addr)) {
        return fail("lookupSymbol deveria retornar false para simbolo inexistente");
    }

    std::cout << "[OK] simbolo inexistente tratado corretamente\n";

    // -------------------------------------------------------
    // caso 6: clear limpa a tabela
    // -------------------------------------------------------
    gts.clear();

    if (gts.lookupSymbol("FUNC_A", addr)) {
        return fail("apos clear, tabela deveria estar vazia");
    }

    std::cout << "[OK] clear limpa a tabela corretamente\n";

    // -------------------------------------------------------
    // caso 7: arquivo .o inexistente retorna lista vazia sem crash
    // -------------------------------------------------------
    std::vector<ExtDef> emptyDefs = GlobalSymbolTable::parseExtDefsFromFile("arquivo_que_nao_existe.o");
    if (!emptyDefs.empty()) {
        return fail("parser deveria retornar lista vazia para arquivo inexistente");
    }

    std::cout << "[OK] arquivo inexistente tratado sem crash\n";

    // limpeza
    std::remove(tmpPath.c_str());

    std::cout << "\nTodos os testes da GTS do Ligador passaram.\n";
    return 0;
}
