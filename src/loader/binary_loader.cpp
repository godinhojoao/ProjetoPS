/*
aqui a gente le um arquivo binario (.bin) contendo código JA montado pelo assembler

exemplo: [opcode][operand1][operand2]
ADD A, B  -> (assembler) -> 00000001 00000000 00000001

Loader: lê o .bin e coloca os 0s e 1s direto na memória da VM
*/

#include "binary_loader.h"
#include <fstream>
#include <iostream>
#include "../constants.h"

// memoria total da VM Z80: 2^16 = 65536 bytes
uint32_t BinaryLoader::load(const std::string& path, Memory& memory, uint16_t startAddr)
{
  // abre em modo binario e ja posiciona no fim pra descobrir o tamanho
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file)
  {
    std::cerr << "Loader: nao foi possivel abrir '" << path << "'\n";
    return 0;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  if (size <= 0)
  {
    std::cerr << "Loader: arquivo '" << path << "' vazio ou ilegivel\n";
    return 0;
  }

  // verifica se o conteudo cabe na memoria a partir de startAddr
  const uint32_t available = VM_MEMORY_IN_BYTES - startAddr;
  if (static_cast<uint32_t>(size) > available)
  {
    std::cerr << "Loader: arquivo nao cabe na memoria (size=" << size
              << " bytes, disponivel a partir de 0x" << std::hex << startAddr
              << " = " << std::dec << available << " bytes)\n";
    return 0;
  }

  // le byte a byte e escreve direto na memoria da VM
  for (uint32_t i = 0; i < static_cast<uint32_t>(size); ++i)
  {
    char byte;
    if (!file.read(&byte, 1))
    {
      std::cerr << "Loader: falha ao ler byte " << i << " de '" << path << "'\n";
      return 0;
    }
    memory.write(static_cast<uint16_t>(startAddr + i), static_cast<uint8_t>(byte));
  }

  return static_cast<uint32_t>(size);
}
