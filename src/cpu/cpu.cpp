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

std::pair<uint8_t *, uint8_t *> CPU::getStackRegistersPair(uint8_t code) {
  uint8_t *hi[] = {&B, &D, &H, &A};
  uint8_t *lo[] = {&C, &E, &L, &F};
  return {hi[code], lo[code]};
}

// return true = keep running, false = stop (invalid instruction or halt)
bool CPU::cycle(const Instruction &inst, Memory &mem) {
  switch (inst.type) {
  case OpcodeType::HALT:
    return false;

  case OpcodeType::PUSH_RP: {
    auto [hi, lo] = getStackRegistersPair(inst.sourceReg);
    SP--; // -- because stack start on top of memory and grows toward bottom of
          // mem.
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
    auto [hi, lo] = getStackRegistersPair(inst.destReg);
    *lo = mem.read(SP);
    SP++;
    // read then sp++ to free the space, memory is not cleaned (trash stays on
    // memory - value of register saved on stack and popped now)
    *hi = mem.read(SP);
    SP++;
    break;
  }

  case OpcodeType::JP_ADDR:
    PC = inst.operand;
    return true; // PC ja setado, nao incrementa

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
    if (!mem.write(SP, static_cast<uint8_t>(returnAddr & 0xFF))) {
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
    uint8_t lo = mem.read(SP); SP++;
    uint8_t hi = mem.read(SP); SP++;

    PC = (static_cast<uint16_t>(hi) << 8) | lo;
    return true;
  }

  default:
    std::cout << "CPU: instrucao invalida\n";
    return false;
  }

  PC += inst.bytesSize; // next instruction
  return true;
}
