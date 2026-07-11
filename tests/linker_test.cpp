#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cstdio>
#include "../src/linker/linker.h"

int main() {
    std::cout << "--- Testando Linker Completo (Tarefa 5) ---\n";

    LinkerObjectFile modA;
    modA.moduleName = "MOD_A";
    modA.size = 5;
    modA.extDefs.push_back({"VALOR_A", 0x0002});
    modA.extRefs.push_back({"VALOR_B", {0x0003}});
    modA.relocs.push_back(0x0000);
    modA.code = {0x01, 0x02, 0x03, 0x00, 0x00};

    LinkerObjectFile modB;
    modB.moduleName = "MOD_B";
    modB.size = 4;
    modB.extDefs.push_back({"VALOR_B", 0x0000});
    modB.extRefs.push_back({"VALOR_A", {0x0001}});
    modB.relocs.push_back(0x0001);
    modB.code = {0x0A, 0x00, 0x00, 0x0B};

    Linker::writeObj("test_modA.o", modA);
    Linker::writeObj("test_modB.o", modB);

    std::vector<std::string> inputs = {"test_modA.o", "test_modB.o"};
    bool success = Linker::link(inputs, "test_output.o");
    assert(success);

    LinkerObjectFile out;
    success = Linker::readObj("test_output.o", out);
    assert(success);

    assert(out.size == 9);

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
    assert(foundA && foundB);

    assert(out.relocs.size() == 2);
    assert(out.relocs[0] == 0x0000);
    assert(out.relocs[1] == 0x0006);

    assert(out.code.size() == 9);
    assert(out.code[0] == 0x01);
    assert(out.code[1] == 0x02);
    assert(out.code[2] == 0x03);
    assert(out.code[3] == 0x05);
    assert(out.code[4] == 0x00);
    assert(out.code[5] == 0x0A);
    assert(out.code[6] == 0x02);
    assert(out.code[7] == 0x00);
    assert(out.code[8] == 0x0B);

    std::cout << "-> Todos os testes do Linker passaram (Unificacao, GTS, Relocacao, ExtRef)!\n";

    std::remove("test_modA.o");
    std::remove("test_modB.o");
    std::remove("test_output.o");

    return 0;
}
