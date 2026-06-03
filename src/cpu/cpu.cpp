/*
CPU:
guarda estado da maquina (registradores, flags)
executa operacoes basicas (ADD, SUB, MOV, etc)
e onde as instruções realmente mexem nos dados
*/
#include <iostream>
#include "cpu.h"
#include "../memory/memory.h"
#include <utility>

uint16_t CPU::getPC() const
{
  return PC;
}

std::pair<uint8_t*, uint8_t*> CPU::getStackRegistersPair(uint8_t code)
{
  uint8_t* hi[] = { &B, &D, &H, &A };
  uint8_t* lo[] = { &C, &E, &L, &F };
  return { hi[code], lo[code] };
}

// return true = keep running, false = stop (invalid instruction or halt)
bool CPU::cycle(const Instruction& inst, Memory& mem)
{
  switch (inst.type) {
    case OpcodeType::HALT:
      return false;

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

    default:
      std::cout << "CPU: instrucao invalida\n";
      return false;
  }

  PC += inst.bytesSize; //next instruction
  return true;
}
