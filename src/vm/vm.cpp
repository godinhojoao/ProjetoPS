#include "vm.h"
#include "../loader/binary_loader.h"
#include "../loader/object_loader.h"
#include <iostream>

bool VM::load(const std::string& binPath, uint16_t startAddr, uint32_t codeSize) {
    reset(); //reseta cpu
    BinaryLoader loader;

    uint32_t bytesLoaded = loader.load(binPath, mem, startAddr);

    if (bytesLoaded == 0) { return false; }

    // os limites de segmento e o PC sao absolutos: quando startAddr != 0 o codigo
    // comeca em startAddr, nao em 0x0000
    mem.setCodeEnd(startAddr + (codeSize > 0 ? codeSize : bytesLoaded));
    mem.setDataEnd(startAddr + bytesLoaded);
    cpu.setPC(startAddr);

    std::cout << "VM: " << bytesLoaded << " bytes carregados de '" << binPath << "'\n";
    return true;
}

bool VM::loadObject(const std::string& objPath, uint16_t loadAddr) {
    reset(); //reseta cpu

    // reset() zera os segmentos, entao Memory::write aceita a escrita do loader.
    // setCodeEnd so depois da carga, senao ele barra o proprio codigo sendo gravado
    ObjectLoader loader;
    uint32_t bytesLoaded = loader.load(objPath, mem, loadAddr);

    if (bytesLoaded == 0) { return false; }

    mem.setCodeEnd(loadAddr + bytesLoaded);
    mem.setDataEnd(loadAddr + bytesLoaded);
    cpu.setPC(loadAddr); //entry point realocado

    std::cout << "VM: " << bytesLoaded << " bytes de '" << objPath
              << "' carregados e realocados em 0x" << std::hex << loadAddr << std::dec << "\n";
    return true;
}

void VM::run() {
    while (cpu.getPC() < mem.getCodeEnd()) {
        Instruction inst = Decoder::decode(mem, cpu.getPC());
        if (!cpu.cycle(inst, mem)) break;
    }
}

bool VM::step() {
    if (cpu.getPC() < mem.getCodeEnd()) {
        Instruction inst = Decoder::decode(mem, cpu.getPC());
        return cpu.cycle(inst, mem);
    }
    return false;
}

void VM::reset() {
    cpu.resetCpu();
    mem.resetSegments();
    return;
}

VMState VM::getState() const {
    CPUState cpuState = cpu.getState();
    VMState state;

    //copia registradores
    state.A = cpuState.A;
    state.B = cpuState.B;
    state.C = cpuState.C;
    state.D = cpuState.D;
    state.E = cpuState.E;
    state.H = cpuState.H;
    state.L = cpuState.L;
    state.F = cpuState.F;

    state.PC = cpuState.PC;
    state.SP = cpuState.SP;

    // copia flags pra lugares separados
    /* explicação
     * o & faz um and bit a bit com a posição da flag q a gente quer
     * e.g: 0b1010 0001 and 0x01(0b0000 0001) fica só com o 0x01 se aquele bit(flag) estiver ativa.
     * caso a flag q eu to extraindo estiver ativa (diferente de zero), a conversão pra bool é true.
     */
    state.flagC  = cpuState.F & 0x01;
    state.flagN  = cpuState.F & 0x02;
    state.flagPV = cpuState.F & 0x04;
    state.flagH  = cpuState.F & 0x10;
    state.flagZ  = cpuState.F & 0x40;
    state.flagS  = cpuState.F & 0x80;

    return state;
}

bool VM::isLoaded() const {
    return mem.getDataEnd() > 0;
}
