#pragma once
#include <string>
#include "../memory/memory.h"
#include "../cpu/cpu.h"
#include "../instruction_set/decoder.h"

class VM
{
public:
  /**
   * @brief  Carrega um programa .bin na memoria da VM (a partir de 0x0000).
   *         Deve ser chamado antes de run() quando se quer executar um programa.
   *
   * @param  binPath  Caminho do arquivo .bin montado pelo assembler.
   * @param  startAddr Endereço inicial de gravação(padrão 0x0000)
   * @return          true se carregou ao menos 1 byte; false em caso de erro.
   */
  bool load(const std::string& binPath, uint16_t startAddr = 0x0000);
  void run();
  //bool step();
  //void reset();

private:
  Memory mem;
  CPU cpu;
};
