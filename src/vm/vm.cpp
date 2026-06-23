#include "vm.h"
#include "../loader/binary_loader.h"
#include <iostream>

bool VM::load(const std::string& binPath, uint16_t startAddr) {
  BinaryLoader loader;

  // startAddr can be used to put a different .asm in memory at the same time.
  // if not informed in load, it will be 0x0000.

  uint32_t bytesLoaded = loader.load(binPath, mem, startAddr); // 64kb 2ˆ16 bytes = 65536 bytes
  // if we use uint16_t we would have only 2^16 from 0 to 65535 but file size (bytes loaded) is from 1 to 65536
  // this single number can overflow our uint16_t, and because of that this bytes loaded is uint32_t

  if (bytesLoaded == 0) { return false; }
  mem.setLoadedAreaEnd(bytesLoaded);

  std::cout << "VM: " << bytesLoaded << " bytes carregados de '" << binPath << "'\n";
  return true;
}

void VM::run() {
  // read entire code from memory, decode instructions, and execute them on CPU
  while (cpu.getPC() < mem.getLoadedAreaEnd())
  {
    Instruction inst = Decoder::decode(mem, cpu.getPC());
    if (!cpu.cycle(inst, mem)) break;
  }
}
