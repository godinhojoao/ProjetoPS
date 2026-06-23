/*
CPU:
guarda estado da maquina (registradores, flags)
executa operacoes basicas (ADD, SUB, MOV, etc)
e onde as instruções realmente mexem nos dados
*/
#include "cpu.h"
#include "../memory/memory.h"
#include <iostream>
#include <utility>

uint16_t CPU::getPC() const { return PC; }

std::pair<uint8_t *, uint8_t *> CPU::getStackRegistersPair(uint8_t code)
{
  uint8_t *hi[] = {&B, &D, &H, &A};
  uint8_t *lo[] = {&C, &E, &L, &F};
  return {hi[code], lo[code]};
}

uint8_t *CPU::getRegister8(uint8_t code)
{
  // 0: B, 1: C, 2: D, 3: E, 4: H, 5: L, 6: (HL - não usado aqui), 7: A
  uint8_t *regs[] = {&B, &C, &D, &E, &H, &L, nullptr, &A};
  return regs[code];
}

void CPU::setFlagMask(uint8_t mask, bool set)
{
  if (set) {
    F |= mask;
  } else {
    F &= ~mask;
  }
}

// return true = keep running, false = stop (invalid instruction or halt)
bool CPU::cycle(const Instruction &inst, Memory &mem) {
    switch (inst.type) {

        case OpcodeType::HALT: {
            return false;
        }

        case OpcodeType::NOP: {
            break;
        }

        case OpcodeType::ADD_A_R: {
            uint8_t val;
            if (inst.sourceReg == static_cast<uint8_t>(Register::HL_INDIRECT)) {
                val = mem.read((static_cast<uint16_t>(H) << 8) | L);
            } else {
                val = *getRegister8(inst.sourceReg);
            }

            uint16_t sum = A + val;
            uint8_t result = static_cast<uint8_t>(sum);

            setFlagMask(FLAG_S, (result & 0x80) != 0);
            setFlagMask(FLAG_Z, result == 0);
            setFlagMask(FLAG_H, ((A & 0x0F) + (val & 0x0F)) > 0x0F);
            setFlagMask(FLAG_PV, (~(A ^ val) & (A ^ result) & 0x80) != 0);
            setFlagMask(FLAG_N, false);
            setFlagMask(FLAG_C, sum > 0xFF);

            A = result;
            break;
        }

        // not implemented
        case OpcodeType::SUB_R: {
            // break;
        }

        case OpcodeType::CP_R: {
            uint8_t val;
            if (inst.sourceReg == static_cast<uint8_t>(Register::HL_INDIRECT)) {
                val = mem.read((static_cast<uint16_t>(H) << 8) | L);
            } else {
                val = *getRegister8(inst.sourceReg);
            }

            uint16_t diff = A - val;
            uint8_t result = static_cast<uint8_t>(diff);

            setFlagMask(FLAG_S, (result & 0x80) != 0);
            setFlagMask(FLAG_Z, result == 0);
            setFlagMask(FLAG_H, (A & 0x0F) < (val & 0x0F));
            setFlagMask(FLAG_PV, ((A ^ val) & (A ^ result) & 0x80) != 0);
            setFlagMask(FLAG_N, true);
            setFlagMask(FLAG_C, A < val);

            if (inst.type == OpcodeType::SUB_R) {
                A = result;
            }
            break;
        }

        case OpcodeType::INC_R: {
            uint8_t val;
            uint16_t hl = 0;
            bool isHL = (inst.destReg == static_cast<uint8_t>(Register::HL_INDIRECT));

            if (isHL) {
                hl = (static_cast<uint16_t>(H) << 8) | L;
                val = mem.read(hl);
            } else {
                val = *getRegister8(inst.destReg);
            }

            uint8_t result = val + 1;

            setFlagMask(FLAG_S, (result & 0x80) != 0);
            setFlagMask(FLAG_Z, result == 0);
            setFlagMask(FLAG_H, (val & 0x0F) == 0x0F);
            setFlagMask(FLAG_PV, val == 0x7F);
            setFlagMask(FLAG_N, false);

            if (isHL) {
                mem.write(hl, result);
            } else {
                *getRegister8(inst.destReg) = result;
            }
            break;
        }

        case OpcodeType::DEC_R: {
            uint8_t val;
            uint16_t hl = 0;
            bool isHL = (inst.destReg == static_cast<uint8_t>(Register::HL_INDIRECT));

            if (isHL) {
                hl = (static_cast<uint16_t>(H) << 8) | L;
                val = mem.read(hl);
            } else {
                val = *getRegister8(inst.destReg);
            }

            uint8_t result = val - 1;

            setFlagMask(FLAG_S, (result & 0x80) != 0);
            setFlagMask(FLAG_Z, result == 0);
            setFlagMask(FLAG_H, (val & 0x0F) == 0x00);
            setFlagMask(FLAG_PV, val == 0x80);
            setFlagMask(FLAG_N, true);

            if (isHL) {
                mem.write(hl, result);
            } else {
                *getRegister8(inst.destReg) = result;
            }
            break;
        }

        // not implemented
        case OpcodeType::AND_R: {
            // break;
        }

        // not implemented
        case OpcodeType::OR_R: {
            // break;
        }

        case OpcodeType::XOR_R: {
            uint8_t val;
            if (inst.sourceReg == static_cast<uint8_t>(Register::HL_INDIRECT)) {
                val = mem.read((static_cast<uint16_t>(H) << 8) | L);
            } else {
                val = *getRegister8(inst.sourceReg);
            }

            if (inst.type == OpcodeType::AND_R) {
                A &= val;
                setFlagMask(FLAG_H, true); // Z80 behavior: AND sets H
            } else if (inst.type == OpcodeType::OR_R) {
                A |= val;
                setFlagMask(FLAG_H, false);
            } else {
                A ^= val;
                setFlagMask(FLAG_H, false);
            }

            setFlagMask(FLAG_S, (A & 0x80) != 0);
            setFlagMask(FLAG_Z, A == 0);

            // P/V on logic indicates parity (1 = even, 0 = odd)
            uint8_t p = A;
            p ^= p >> 4;
            p ^= p >> 2;
            p ^= p >> 1;
            setFlagMask(FLAG_PV, !(p & 1));

            setFlagMask(FLAG_N, false);
            setFlagMask(FLAG_C, false);
            break;
        }

        // LD r, r' - Copia o valor de um registrador para o outro
        case OpcodeType::LD_R_R: {
            *getRegister8(inst.destReg) = *getRegister8(inst.sourceReg);
            break;
        }

        // LD r, n - Copia um valor imediato de 8 bits para o registrador
        case OpcodeType::LD_R_N: {
            // Verifica se é o endereço na memória pontado por HL (código 6)
            if (inst.destReg == static_cast<uint8_t>(Register::HL_INDIRECT)) {
                uint16_t hl = (static_cast<uint16_t>(H) << 8) | L;

                if (!mem.write(hl, static_cast<uint8_t>(inst.operand & 0xFF))) {
                    std::cout << "CPU: memory write error at (HL) on LD (HL), n\n";
                    return false;
                }
            } else {
                *getRegister8(inst.destReg) = static_cast<uint8_t>(inst.operand & 0xFF);
            }

            break;
        }

        // LD (HL), r - Escreve na memória o valor do registrador
        case OpcodeType::LD_HL_R: {
            // Forma o endereço combinando H (parte alta) e L (parte baixa)
            uint16_t hl = (static_cast<uint16_t>(H) << 8) | L;

            if (!mem.write(hl, *getRegister8(inst.sourceReg))) {
                std::cout << "CPU: memory write error at (HL)\n";
                return false;
            }
            break;
        }

        // LD r, (HL) - Lê da memória e salva no registrador
        case OpcodeType::LD_R_HL: {
            // Forma o endereço combinando H (parte alta) e L (parte baixa)
            uint16_t hl = (static_cast<uint16_t>(H) << 8) | L;

            *getRegister8(inst.destReg) = mem.read(hl);

            break;
        }

        case OpcodeType::PUSH_RP: {
            auto regs = getStackRegistersPair(inst.sourceReg);
            auto hi = regs.first;
            auto lo = regs.second;

            SP--; // -- because stack start on top of memory and grows toward bottom of mem.

            if (!mem.write(SP, *hi)) {
                std::cout << "CPU: stack overflow\n";
                return false;
            }

            SP--;

            if (!mem.write(SP, *lo)) {
                std::cout << "CPU: stack overflow\n";
                return false;
            }

            break;
        }

        case OpcodeType::POP_RP: {
            // trying to pop empty stack

            if (SP == VM_MEMORY_IN_BYTES - 1) {
                std::cout << "CPU: stack underflow\n";
                return false;
            }

            auto regs = getStackRegistersPair(inst.sourceReg);
            auto hi = regs.first;
            auto lo = regs.second;

            *lo = mem.read(SP);
            SP++;

            // read then sp++ to free the space, memory is not cleaned (trash stays on memory - value of register saved on stack and popped now)
            *hi = mem.read(SP);
            SP++;

            break;
        }

        case OpcodeType::JP_ADDR: {
            PC = inst.operand;
            return true; // PC ja setado, nao incrementa
        }

        // JR: offset signed, pode ser negativo (salto pra tras, ex: loops)
        case OpcodeType::JR_OFFSET: {
            int8_t offset = static_cast<int8_t>(inst.operand & 0xFF); // uint16_t -> signed
            PC = PC + inst.bytesSize + offset;
            return true;
        }

        // CALL: salva endereco de retorno na pilha e salta pra subrotina
        case OpcodeType::CALL_ADDR: {
            uint16_t returnAddr = PC + inst.bytesSize; // instrucao seguinte ao CALL

            // empilha hi byte (SP-- antes de escrever, igual ao PUSH)
            SP--;

            if (!mem.write(SP, static_cast<uint8_t>(returnAddr >> 8))) {
                std::cout << "CPU: stack overflow (CALL)\n";
                return false;
            }
            // empilha lo byte
            SP--;

            if (!mem.write(SP, static_cast<uint8_t>(returnAddr & 0xFF))){
                std::cout << "CPU: stack overflow (CALL)\n";
                return false;
            }

            PC = inst.operand;
            return true;
        }

          // RET: pega endereco de retorno da pilha e volta (inverso do CALL)
        case OpcodeType::RET: {
            if (SP == VM_MEMORY_IN_BYTES - 1) { // pilha vazia
                std::cout << "CPU: stack underflow (RET)\n";
                return false;
            }

            // lo primeiro, hi depois (ordem inversa do push do CALL)
            uint8_t lo = mem.read(SP);
            SP++;
            uint8_t hi = mem.read(SP);
            SP++;

            PC = (static_cast<uint16_t>(hi) << 8) | lo;
            return true;
        }

        default: {
            std::cout << "CPU: instrucao invalida\n";
            return false;
        }
    } //end switch

    PC += inst.bytesSize; // next instruction
    return true;
} //end cycle
