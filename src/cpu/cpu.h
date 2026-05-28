#pragma once
#include <cstdint>
#include <utility>
#include "../instruction_set/instruction.h"
#include <constants.h>

class Memory;

class CPU
{
public:
  uint16_t getPC() const;
  bool cycle(const Instruction& inst, Memory& mem);

private:
  uint8_t A = 0, B = 0, C = 0, D = 0, E = 0, H = 0, L = 0;
  uint8_t F = 0;
  uint16_t SP = VM_MEMORY_IN_BYTES - 1; // start on top of memory
  uint16_t PC = 0;

  std::pair<uint8_t*, uint8_t*> getStackRegistersPair(uint8_t code); // (BC/DE/HL/AF)
};
