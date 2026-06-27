#include "vm.h"
#include "../loader/binary_loader.h"
#include <iostream>

bool VM::load(const std::string& binPath, uint16_t startAddr) {
    reset(); //reseta cpu
    BinaryLoader loader;

    // startAddr can be used to put a different .asm in memory at the same time.
    // if not informed in load, it will be 0x0000.

    uint32_t bytesLoaded = loader.load(binPath, mem, startAddr); // 64kb 2ˆ16 bytes = 65536 bytes
    // if we use uint16_t we would have only 2^16 from 0 to 65535 but file size (bytes loaded) is from 1 to 65536
    // this single number can overflow our uint16_t, and because of that this bytes loaded is uint32_t

    if (bytesLoaded == 0) { return false; }
    mem.setLoadedAreaEnd(bytesLoaded);

    std::cout << "VM: " << bytesLoaded << " bytes carregados de '" << binPath << "'\n";
    return true;
}

void VM::run() {
    // read entire code from memory, decode instructions, and execute them on CPU
    while (cpu.getPC() < mem.getLoadedAreaEnd()) {
        Instruction inst = Decoder::decode(mem, cpu.getPC());
        if (!cpu.cycle(inst, mem)) break;
    }
}

bool VM::step() {
    if (cpu.getPC() < mem.getLoadedAreaEnd()) {
        Instruction inst = Decoder::decode(mem, cpu.getPC());
        return cpu.cycle(inst, mem);
    }
    return false;
}

void VM::reset() {
    cpu.resetCpu();
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
    return mem.getLoadedAreaEnd() > 0 ? true : false;
}
