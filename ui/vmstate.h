#ifndef VMSTATE_H
#define VMSTATE_H

#include <cstdint>

struct VMState {
    //regs de 8 bit
    uint8_t A, B, C, D, E, H, L, F;

    //regs de 16 bits
    uint16_t PC, SP;

    //flags extraídas de F (facilitando minha vida)
    bool flagZ, flagS, flagC, flagH, flagN, flagPV;

    //status atual da VM
    enum class Status { IDLE, RUNNING, HALTED, ERROR } status;
};

#endif // VMSTATE_H
