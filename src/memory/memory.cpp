#include "memory.h"

uint8_t Memory::read(uint16_t addr) const
{
  return data[addr];
}

// false = error writing
bool Memory::write(uint16_t addr, uint8_t value)
{
  // trying to write on code+data area
  if (addr < loadedAreaEnd) {
    return false;
  }
  data[addr] = value;
  return true;
}

uint32_t Memory::getLoadedAreaEnd() const
{
  return loadedAreaEnd;
}

void Memory::setLoadedAreaEnd(uint32_t end)
{
  loadedAreaEnd = end;
}
