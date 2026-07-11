#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cstdio>
#include "../src/linker/linker.h"

int main() {
    std::cout << "--- Testando Linker Completo (Tarefa 5) ---\n";

    // Criar módulo 1 (A)
    LinkerObjectFile modA;
    modA.moduleName = "MOD_A";
    modA.size = 5;
    modA.extDefs.push_back({"VALOR_A", 0x0002});
    modA.extRefs.push_back({"VALOR_B", {0x0003}});
    modA.relocs.push_back(0x0000);
    // Código A: 01 02 [VALOR_A=03] [EXTREF_VALOR_B=00 00]
    modA.code = {0x01, 0x02, 0x03, 0x00, 0x00};

    // Criar módulo 2 (B)
    LinkerObjectFile modB;
    modB.moduleName = "MOD_B";
    modB.size = 4;
    modB.extDefs.push_back({"VALOR_B", 0x0000});
    modB.extRefs.push_back({"VALOR_A", {0x0001}});
    modB.relocs.push_back(0x0001);
    // Código B: [VALOR_B=0A] [EXTREF_VALOR_A=00 00] 0B
    modB.code = {0x0A, 0x00, 0x00, 0x0B};

    // Escrever módulos temporários
    Linker::writeObj("test_modA.o", modA);
    Linker::writeObj("test_modB.o", modB);

    // Linkar!
    std::vector<std::string> inputs = {"test_modA.o", "test_modB.o"};
    bool success = Linker::link(inputs, "test_output.o");
    assert(success && "O processo de linkedicao falhou!");

    // Ler e verificar o arquivo de saída
    LinkerObjectFile out;
    success = Linker::readObj("test_output.o", out);
    assert(success && "Falha ao ler o arquivo gerado pelo linker!");

    // Verificar Tamanho
    assert(out.size == 9 && "Tamanho combinado incorreto!");

    // Verificar GTS Resolvido e Reexportado (opcional, dependendo de como testamos)
    // VALOR_A devia estar em 0x0000 + 0x0002 = 0x0002
    // VALOR_B devia estar em 0x0005 + 0x0000 = 0x0005
    bool foundA = false, foundB = false;
    for (const auto& def : out.extDefs) {
        if (def.symbolName == "VALOR_A") {
            assert(def.relativeAddress == 0x0002);
            foundA = true;
        } else if (def.symbolName == "VALOR_B") {
            assert(def.relativeAddress == 0x0005);
            foundB = true;
        }
    }
    assert(foundA && foundB && "EXTDEFs consolidados estao incorretos!");

    // Verificar Realocação Unificada
    // MOD_A.relocs = {0x0000} -> {0x0000}
    // MOD_B.relocs = {0x0001} -> {0x0001 + 0x0005} = {0x0006}
    assert(out.relocs.size() == 2);
    assert(out.relocs[0] == 0x0000);
    assert(out.relocs[1] == 0x0006);

    // Verificar Resolução de EXTREFs no Código
    // Código A Original: 01 02 03 00 00
    // Código B Original: 0A 00 00 0B
    // EXTREFs A: "VALOR_B" em 0x0003
    // EXTREFs B: "VALOR_A" em 0x0001
    // Endereço Absoluto VALOR_B = 0x0005 -> Patch em offset 0x0003 do código total.
    // Endereço Absoluto VALOR_A = 0x0002 -> Patch em offset 0x0001 (relativo ao módulo B), que é 0x0006 do código total.

    // Código Esperado:
    // 01 02 03 05 00 | 0A 02 00 0B
    assert(out.code.size() == 9);
    assert(out.code[0] == 0x01);
    assert(out.code[1] == 0x02);
    assert(out.code[2] == 0x03);
    assert(out.code[3] == 0x05); // LSB de VALOR_B
    assert(out.code[4] == 0x00); // MSB de VALOR_B
    assert(out.code[5] == 0x0A);
    assert(out.code[6] == 0x02); // LSB de VALOR_A
    assert(out.code[7] == 0x00); // MSB de VALOR_A
    assert(out.code[8] == 0x0B);

    std::cout << "-> Todos os testes do Linker passaram (Unificacao, GTS, Relocacao, ExtRef)!\n";

    // Limpeza
    std::remove("test_modA.o");
    std::remove("test_modB.o");
    std::remove("test_output.o");

    return 0;
}
