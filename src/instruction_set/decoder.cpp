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

        // LD (nn), A
        case 0x32:
            inst.type = OpcodeType::LD_NN_A;
            inst.operand = (memory.read(pc + 2) << 8) | memory.read(pc + 1);
            inst.bytesSize = 3;
            return inst;

        // LD A, (nn)
        case 0x3A:
            inst.type = OpcodeType::LD_A_NN;
            inst.operand = (memory.read(pc + 2) << 8) | memory.read(pc + 1);
            inst.bytesSize = 3;
            return inst;

        // Prefixo DD -> instruções com IX
        case 0xDD: {
            uint8_t op2 = memory.read(pc + 1);
            int8_t d = static_cast<int8_t>(memory.read(pc + 2));

            // LD (IX+d), n  -> DD 36 d n (4 bytes)
            if (op2 == 0x36) {
                inst.type = OpcodeType::LD_IX_N;
                inst.operand = (static_cast<uint16_t>(static_cast<uint8_t>(d)) << 8) | memory.read(pc + 3);
                inst.bytesSize = 4;
                return inst;
            }

            // LD (IX+d), r  -> DD 01110rrr d (3 bytes)
            if ((op2 & 0b11111000) == 0b01110000 && (op2 & 0b111) != 0b110) {
                inst.type = OpcodeType::LD_IX_R;
                inst.sourceReg = op2 & 0b111;
                inst.operand = static_cast<uint8_t>(d);
                inst.bytesSize = 3;
                return inst;
            }

            // LD r, (IX+d)  -> DD 01rrr110 d (3 bytes)
            if ((op2 & 0b11000111) == 0b01000110) {
                inst.type = OpcodeType::LD_R_IX;
                inst.destReg = (op2 >> 3) & 0b111;
                inst.operand = static_cast<uint8_t>(d);
                inst.bytesSize = 3;
                return inst;
            }

            // Aritméticas/Lógicas com (IX+d): ADD/SUB/AND/OR/XOR/CP
            if ((op2 & 0b11000111) == 0b10000110) {
                uint8_t aluOp = (op2 >> 3) & 0b111;
                switch (aluOp) {
                    case 0: inst.type = OpcodeType::ADD_A_IX; break;
                    case 2: inst.type = OpcodeType::SUB_IX;   break;
                    case 4: inst.type = OpcodeType::AND_IX;   break;
                    case 5: inst.type = OpcodeType::XOR_IX;   break;
                    case 6: inst.type = OpcodeType::OR_IX;    break;
                    case 7: inst.type = OpcodeType::CP_IX;    break;
                    default: inst.type = OpcodeType::UNKNOWN; break;
                }
                inst.operand = static_cast<uint8_t>(d);
                inst.bytesSize = 3;
                return inst;
            }

            inst.type = OpcodeType::UNKNOWN;
            inst.bytesSize = 2;
            return inst;
        }

        // Prefixo FD -> instruções com IY (mesma lógica do DD)
        case 0xFD: {
            uint8_t op2 = memory.read(pc + 1);
            int8_t d = static_cast<int8_t>(memory.read(pc + 2));

            // LD (IY+d), n  -> FD 36 d n (4 bytes)
            if (op2 == 0x36) {
                inst.type = OpcodeType::LD_IY_N;
                inst.operand = (static_cast<uint16_t>(static_cast<uint8_t>(d)) << 8) | memory.read(pc + 3);
                inst.bytesSize = 4;
                return inst;
            }

            // LD (IY+d), r  -> FD 01110rrr d (3 bytes)
            if ((op2 & 0b11111000) == 0b01110000 && (op2 & 0b111) != 0b110) {
                inst.type = OpcodeType::LD_IY_R;
                inst.sourceReg = op2 & 0b111;
                inst.operand = static_cast<uint8_t>(d);
                inst.bytesSize = 3;
                return inst;
            }

            // LD r, (IY+d)  -> FD 01rrr110 d (3 bytes)
            if ((op2 & 0b11000111) == 0b01000110) {
                inst.type = OpcodeType::LD_R_IY;
                inst.destReg = (op2 >> 3) & 0b111;
                inst.operand = static_cast<uint8_t>(d);
                inst.bytesSize = 3;
                return inst;
            }

            // Aritméticas/Lógicas com (IY+d)
            if ((op2 & 0b11000111) == 0b10000110) {
                uint8_t aluOp = (op2 >> 3) & 0b111;
                switch (aluOp) {
                    case 0: inst.type = OpcodeType::ADD_A_IY; break;
                    case 2: inst.type = OpcodeType::SUB_IY;   break;
                    case 4: inst.type = OpcodeType::AND_IY;   break;
                    case 5: inst.type = OpcodeType::XOR_IY;   break;
                    case 6: inst.type = OpcodeType::OR_IY;    break;
                    case 7: inst.type = OpcodeType::CP_IY;    break;
                    default: inst.type = OpcodeType::UNKNOWN; break;
                }
                inst.operand = static_cast<uint8_t>(d);
                inst.bytesSize = 3;
                return inst;
            }

            inst.type = OpcodeType::UNKNOWN;
            inst.bytesSize = 2;
            return inst;
        }

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
