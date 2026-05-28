#include "decoder.h"
#include <iostream>

Instruction Decoder::decode(const Memory& memory, uint16_t pc) {
    Instruction inst = {};
    inst.type = OpcodeType::UNKNOWN;
    inst.bytesSize = 1;
    inst.operand = 0;
    
    uint8_t opcode = memory.read(pc);

    switch (opcode) {
        // NOP
        case 0x00:
            inst.type = OpcodeType::NOP;
            return inst;
            
        // HALT
        case 0x76:
            inst.type = OpcodeType::HALT;
            return inst;
            
        // RET
        case 0xC9:
            inst.type = OpcodeType::RET;
            return inst;
            
        // JP addr
        case 0xC3:
            inst.type = OpcodeType::JP_ADDR;
            inst.operand = (memory.read(pc + 2) << 8) | memory.read(pc + 1);
            inst.bytesSize = 3;
            return inst;
            
        // JR offset
        case 0x18:
            inst.type = OpcodeType::JR_OFFSET;
            inst.operand = memory.read(pc + 1);
            inst.bytesSize = 2;
            return inst;
            
        // CALL addr
        case 0xCD:
            inst.type = OpcodeType::CALL_ADDR;
            inst.operand = (memory.read(pc + 2) << 8) | memory.read(pc + 1);
            inst.bytesSize = 3;
            return inst;

        default:
            // LD r, r' | LD (HL), r | LD r, (HL)
            if ((opcode & 0b11000000) == 0b01000000) {
                uint8_t dest = (opcode >> 3) & 0b111;
                uint8_t src = opcode & 0b111;
                
                if (dest == 0b110) {
                    inst.type = OpcodeType::LD_HL_R;
                    inst.sourceReg = src;
                } else if (src == 0b110) {
                    inst.type = OpcodeType::LD_R_HL;
                    inst.destReg = dest;
                } else {
                    inst.type = OpcodeType::LD_R_R;
                    inst.destReg = dest;
                    inst.sourceReg = src;
                }
                return inst;
            }

            // LD r, n
            if ((opcode & 0b11000111) == 0b00000110) {
                inst.type = OpcodeType::LD_R_N;
                inst.destReg = (opcode >> 3) & 0b111;
                inst.operand = memory.read(pc + 1);
                inst.bytesSize = 2;
                return inst;
            }

            // ADD A, r
            if ((opcode & 0b11111000) == 0b10000000) {
                inst.type = OpcodeType::ADD_A_R;
                inst.sourceReg = opcode & 0b111;
                return inst;
            }

            // SUB r
            if ((opcode & 0b11111000) == 0b10010000) {
                inst.type = OpcodeType::SUB_R;
                inst.sourceReg = opcode & 0b111;
                return inst;
            }

            // CP r
            if ((opcode & 0b11111000) == 0b10111000) {
                inst.type = OpcodeType::CP_R;
                inst.sourceReg = opcode & 0b111;
                return inst;
            }

            // INC r
            if ((opcode & 0b11000111) == 0b00000100) {
                inst.type = OpcodeType::INC_R;
                inst.destReg = (opcode >> 3) & 0b111;
                return inst;
            }

            // DEC r
            if ((opcode & 0b11000111) == 0b00000101) {
                inst.type = OpcodeType::DEC_R;
                inst.destReg = (opcode >> 3) & 0b111;
                return inst;
            }

            // AND r
            if ((opcode & 0b11111000) == 0b10100000) {
                inst.type = OpcodeType::AND_R;
                inst.sourceReg = opcode & 0b111;
                return inst;
            }

            // OR r
            if ((opcode & 0b11111000) == 0b10110000) {
                inst.type = OpcodeType::OR_R;
                inst.sourceReg = opcode & 0b111;
                return inst;
            }

            // XOR r
            if ((opcode & 0b11111000) == 0b10101000) {
                inst.type = OpcodeType::XOR_R;
                inst.sourceReg = opcode & 0b111;
                return inst;
            }

            // PUSH rp
            if ((opcode & 0b11001111) == 0b11000101) {
                inst.type = OpcodeType::PUSH_RP;
                inst.sourceReg = (opcode >> 4) & 0b11;
                return inst;
            }

            // POP rp
            if ((opcode & 0b11001111) == 0b11000001) {
                inst.type = OpcodeType::POP_RP;
                inst.destReg = (opcode >> 4) & 0b11;
                return inst;
            }

            break;
    }

    return inst;
}
