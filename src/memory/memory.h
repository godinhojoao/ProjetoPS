#pragma once
#include <cstdint>
#include <constants.h>

class Memory
{
public:
  uint8_t read(uint16_t addr) const;
  bool write(uint16_t addr, uint8_t value);

  uint32_t getLoadedAreaEnd() const;
  void setLoadedAreaEnd(uint32_t end);

private:
  uint8_t data[VM_MEMORY_IN_BYTES] = {};
  uint32_t loadedAreaEnd = 0;
};
