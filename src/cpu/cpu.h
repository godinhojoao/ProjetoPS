#pragma once
#include <cstdint>
#include <utility>
#include "../instruction_set/instruction.h"
#include "../constants.h"

struct CPUState {
    uint8_t A, B, C, D, E, H, L, F;
    uint16_t PC, SP, IX, IY;
};

class Memory;

class CPU
{
public:
    uint16_t getPC() const;
    void setPC(uint16_t pc);
    CPUState getState() const;
    void setState(uint8_t A, uint8_t B, uint8_t C, uint8_t D, uint8_t E, uint8_t H, uint8_t L, uint8_t F, uint16_t PC, uint16_t SP, uint16_t IX = 0, uint16_t IY = 0);
    void resetCpu();
    bool getFlag(uint8_t mask) const;
    bool cycle(const Instruction& inst, Memory& mem);

private:
    uint8_t A = 0, B = 0, C = 0, D = 0, E = 0, H = 0, L = 0;
    uint8_t F = 0;
    uint16_t PC = 0;
    uint16_t SP = VM_MEMORY_IN_BYTES - 1; // start on top of memory
    uint16_t IX = 0;
    uint16_t IY = 0;
    std::pair<uint8_t*, uint8_t*> getStackRegistersPair(uint8_t code); // (BC/DE/HL/AF)

    /**
    * @brief  Obtém o ponteiro para um registrador de 8 bits a partir do seu código
    *
    * @param  code  Código numérico do registrador (0 a 7, exceto 6)
    * @return       Ponteiro para a variável interna do registrador
    */
    uint8_t* getRegister8(uint8_t code);

    // Máscaras para a flag F
    static constexpr uint8_t FLAG_C  = 1 << 0; // Carry
    static constexpr uint8_t FLAG_N  = 1 << 1; // Add/Subtract
    static constexpr uint8_t FLAG_PV = 1 << 2; // Parity/Overflow
    static constexpr uint8_t FLAG_H  = 1 << 4; // Half Carry
    static constexpr uint8_t FLAG_Z  = 1 << 6; // Zero
    static constexpr uint8_t FLAG_S  = 1 << 7; // Sign

    void setFlagMask(uint8_t mask, bool set);
};
