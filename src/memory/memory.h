#pragma once
#include <cstdint>
#include <constants.h>

class Memory
{
public:
  uint8_t read(uint16_t addr) const;
  void write(uint16_t addr, uint8_t value);

private:
  uint8_t data[VM_MEMORY_IN_BYTES] = {};
};
