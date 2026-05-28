#include "vm.h"
#include "../loader/binary_loader.h"
#include <iostream>

bool VM::load(const std::string& binPath)
{
  BinaryLoader loader;
  uint32_t bytesLoaded = loader.load(binPath, mem);
  if (bytesLoaded == 0)
  {
    return false;
  }
  codeEnd = static_cast<uint16_t>(bytesLoaded);
  std::cout << "VM: " << bytesLoaded << " bytes carregados de '" << binPath << "'\n";
  return true;
}

void VM::run()
{
  // read entire code from memory, decode instructions, and execute them on CPU
  while (cpu.getPC() < codeEnd)
  {
    Instruction inst = Decoder::decode(mem, cpu.getPC());
    if (!cpu.cycle(inst, mem)) break;
  }
}
