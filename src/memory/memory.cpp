#include "memory.h"

uint8_t Memory::read(uint16_t addr) const
{
  return data[addr];
}

bool Memory::write(uint16_t addr, uint8_t value)
{
  if (addr < codeEnd) {
    return false;
  }
  data[addr] = value;
  return true;
}

bool Memory::stackWrite(uint16_t addr, uint8_t value)
{
  if (addr < dataEnd) {
    return false;
  }
  data[addr] = value;
  return true;
}

uint32_t Memory::getCodeEnd() const
{
  return codeEnd;
}

void Memory::setCodeEnd(uint32_t end)
{
  codeEnd = end;
}

uint32_t Memory::getDataEnd() const
{
  return dataEnd;
}

void Memory::setDataEnd(uint32_t end)
{
  dataEnd = end;
}

void Memory::resetSegments() {
    codeEnd = 0;
    dataEnd = 0;
}
