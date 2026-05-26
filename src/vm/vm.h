#pragma once
#include <string>
#include "../memory/memory.h"

class VM
{
public:
  /**
   * @brief  Carrega um programa .bin na memoria da VM (a partir de 0x0000).
   *         Deve ser chamado antes de run() quando se quer executar um programa.
   *
   * @param  binPath  Caminho do arquivo .bin montado pelo assembler.
   * @return          true se carregou ao menos 1 byte; false em caso de erro.
   */
  bool load(const std::string& binPath);

  void run();

private:
  Memory mem;
};