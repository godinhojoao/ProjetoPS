#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>

// TODO: remove when all registers are on cpu
enum class Register : uint8_t {
    B = 0b000,
    C = 0b001,
    D = 0b010,
    E = 0b011,
    H = 0b100,
    L = 0b101,
    HL_INDIRECT = 0b110, // O famoso "6" - Acesso indireto à memória via HL
    A = 0b111
};
// enum class RegisterPair : uint8_t {
//     BC = 0b00,
//     DE = 0b01,
//     HL = 0b10,
//     AF = 0b11
// };

// Tipos de operações suportadas pela VM
enum class OpcodeType : uint8_t {
    NOP = 0,
    HALT,
    LD_R_R,
    LD_R_N,
    LD_HL_R,
    LD_R_HL,
    ADD_A_R,
    SUB_R,
    INC_R,
    DEC_R,
    CP_R,
    AND_R,
    OR_R,
    XOR_R,
    JP_ADDR,
    JR_OFFSET,
    CALL_ADDR,
    RET,
    PUSH_RP,
    POP_RP,
    UNKNOWN
};

// Estrutura empacotada pelo Decoder para o Executor
struct Instruction {
    OpcodeType type;
    uint8_t destReg;     // Destino da operação
    uint8_t sourceReg;   // Origem da operação
    uint16_t operand;    // Valores imediatos, offsets ou endereços
    uint8_t bytesSize;   // Quantidade de bytes consumidos na memória
};

#endif
