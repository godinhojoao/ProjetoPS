#include "../src/assembler/assembler.h"
#include "../src/loader/object_loader.h"
#include "../src/linker/linker.h"
#include "../src/memory/memory.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <cstdio>
#include <sstream>

static bool writeTempFile(const std::string& path, const std::string& content) {
    std::ofstream out(path);
    if (!out) return false;
    out << content;
    return out.good();
}

static const std::string TEST_CODE =
    "MODULE modulo_teste\n"
    "PUBLIC SOMA, SUBTRAI\n"
    "EXTERN MULTIPLICA, DIVIDE\n"
    "VALOR: EQU 42\n"
    "\n"
    "START: LD A, VALOR\n"      // 3E 2A  (2 bytes)
    "       CALL MULTIPLICA\n"  // CD 00 00 (3 bytes, external ref at start + 3)
    "loop:  JP loop\n"          // C3 05 00 (3 bytes, relocatable at start + 6)
    "SOMA:  ADD A, B\n"         // 80 (1 byte)
    "SUBTRAI: SUB C\n"          // 91 (1 byte)
    "       DW SOMA\n"          // relocatable word at start + 10
    "       DW DIVIDE\n"        // external ref word at start + 12
    "       END\n";

int main() {
    std::cout << "Executando testes do Assembler Modular...\n";

    const std::string asmPath = "tests/test_assembler_modular.asm";
    if (!writeTempFile(asmPath, TEST_CODE)) {
        std::cerr << "[FAIL] Nao foi possivel escrever arquivo .asm de teste\n";
        return 1;
    }

    Assembler assembler;
    std::string binPath = assembler.assemble(asmPath);

    if (binPath.empty()) {
        std::cerr << "[FAIL] Erro ao compilar com o Assembler:\n";
        for (const auto& err : assembler.getErrors()) {
            std::cerr << "  " << err << "\n";
        }
        return 1;
    }

    std::cout << "[OK] Codigo compilado com sucesso. Arquivo gerado: " << binPath << "\n";

    // O arquivo objeto correspondente deve ser: tests/test_assembler_modular.o
    const std::string objPath = "tests/test_assembler_modular.o";
    std::ifstream objFile(objPath);
    if (!objFile) {
        std::cerr << "[FAIL] Arquivo objeto " << objPath << " nao foi gerado.\n";
        return 1;
    }

    std::stringstream ss;
    ss << objFile.rdbuf();
    std::string objContent = ss.str();
    objFile.close();

    std::cout << "Conteudo de " << objPath << ":\n" << objContent << "\n";

    // Verificações no .o
    assert(objContent.find("HEADER") != std::string::npos);
    assert(objContent.find("MODULE: modulo_teste") != std::string::npos);
    assert(objContent.find("SIZE: 14") != std::string::npos); // 2 + 3 + 3 + 1 + 1 + 2 + 2 = 14 bytes
    assert(objContent.find("EXTDEF") != std::string::npos);
    assert(objContent.find("SOMA 0x0008") != std::string::npos);
    assert(objContent.find("SUBTRAI 0x0009") != std::string::npos);
    assert(objContent.find("EXTREF") != std::string::npos);
    assert(objContent.find("MULTIPLICA 0x0003") != std::string::npos);
    assert(objContent.find("DIVIDE 0x000C") != std::string::npos);
    assert(objContent.find("REALOC") != std::string::npos);
    assert(objContent.find("0x0006") != std::string::npos);
    assert(objContent.find("0x000A") != std::string::npos);

    std::cout << "[OK] Verificacoes do arquivo .o concluidas com sucesso.\n";

    // Agora tenta ler o arquivo gerado usando o ObjectLoader do projeto
    ObjectLoader loader;
    ObjectFile obj;
    if (!loader.parse(objPath, obj)) {
        std::cerr << "[FAIL] ObjectLoader falhou em fazer o parse do .o gerado pelo nosso montador\n";
        return 1;
    }

    std::cout << "[OK] ObjectLoader conseguiu fazer o parse do .o com sucesso.\n";
    assert(obj.moduleName == "modulo_teste");
    assert(obj.declaredSize == 14);
    assert(obj.relocOffsets.size() == 2);
    assert(obj.relocOffsets[0] == 0x0006);
    assert(obj.relocOffsets[1] == 0x000A);

    // -------------------------------------------------------------
    // Teste de Ligação (Linker): Junta modulo_teste e modulo_lib
    // -------------------------------------------------------------
    std::cout << "\nTestando integracao com o Linker...\n";

    const std::string libAsmPath = "tests/test_lib.asm";
    const std::string libCode =
        "MODULE modulo_lib\n"
        "PUBLIC MULTIPLICA, DIVIDE\n"
        "\n"
        "MULTIPLICA: NOP\n" // 00 (1 byte)
        "            RET\n" // C9 (1 byte)
        "DIVIDE:     NOP\n" // 00 (1 byte)
        "            RET\n" // C9 (1 byte)
        "            END\n";

    if (!writeTempFile(libAsmPath, libCode)) {
        std::cerr << "[FAIL] Nao foi possivel escrever o arquivo da biblioteca .asm\n";
        return 1;
    }

    std::string libBinPath = assembler.assemble(libAsmPath);
    if (libBinPath.empty()) {
        std::cerr << "[FAIL] Erro ao compilar modulo_lib com o Assembler\n";
        return 1;
    }

    std::vector<std::string> inputs = { "tests/test_assembler_modular.o", "tests/test_lib.o" };
    const std::string linkedObjPath = "tests/test_linked_output.o";

    if (!Linker::link(inputs, linkedObjPath)) {
        std::cerr << "[FAIL] Falha ao ligar os arquivos objeto\n";
        return 1;
    }
    std::cout << "[OK] Arquivos objeto ligados com sucesso em: " << linkedObjPath << "\n";

    // O loader do projeto agora deve carregar o objeto ligado com sucesso!
    ObjectFile linkedObj;
    if (!loader.parse(linkedObjPath, linkedObj)) {
        std::cerr << "[FAIL] ObjectLoader falhou ao fazer parse do arquivo ligado final\n";
        return 1;
    }

    std::cout << "[OK] ObjectLoader leu o arquivo ligado final com sucesso.\n";
    std::cout << "Nome do modulo ligado: " << linkedObj.moduleName << "\n";
    std::cout << "Tamanho declarado: " << linkedObj.declaredSize << " bytes\n";
    // modulo_teste (14) + modulo_lib (4) = 18 bytes
    assert(linkedObj.declaredSize == 18);

    // O código ligado final deve ter:
    // Do modulo_teste: 3E 2A CD 0E 00 C3 05 00 80 91 08 00 0E 00
    // (O CALL em 0x0003 deve apontar para 0x000E que é MULTIPLICA)
    // (O DW em 0x000C deve apontar para 0x0010 que é DIVIDE)
    // Do modulo_lib: 00 C9 00 C9
    std::cout << "Bytes ligados:\n";
    for (size_t i = 0; i < linkedObj.code.size(); ++i) {
        printf("%02X ", linkedObj.code[i]);
    }
    printf("\n");

    // Valida os patches de resolucao de simbolos aplicados pelo linker
    // MULTIPLICA esta no offset 14 (0x000E)
    assert(linkedObj.code[3] == 0x0E);
    assert(linkedObj.code[4] == 0x00);
    // DIVIDE esta no offset 16 (0x0010)
    assert(linkedObj.code[12] == 0x10);
    assert(linkedObj.code[13] == 0x00);

    std::cout << "[OK] Verificacoes do binario ligado/resolvido corretas!\n";

    // Limpa arquivos temporários
    std::remove(asmPath.c_str());
    std::remove("tests/test_assembler_modular.bin");
    std::remove(objPath.c_str());
    std::remove(libAsmPath.c_str());
    std::remove("tests/test_lib.bin");
    std::remove("tests/test_lib.o");
    std::remove(linkedObjPath.c_str());

    std::cout << "\nTodos os testes do Assembler Modular passaram com sucesso!\n";
    return 0;
}
