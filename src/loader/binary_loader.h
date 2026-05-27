#pragma once
#include <cstdint>
#include <string>
#include "../memory/memory.h"

/*
Loader: lê um arquivo .bin (já montado pelo assembler) e copia os bytes
direto na memória da VM. Não interpreta nada — só transfere bytes.
*/
class BinaryLoader
{
public:
  /**
   * @brief  Carrega um arquivo binário na memória da VM.
   *
   * @param  path       Caminho do arquivo .bin a ser carregado.
   * @param  memory     Referência à memória da VM onde os bytes serão gravados.
   * @param  startAddr  Endereço inicial de gravação (padrão 0x0000 = início do
   *                    segmento de código).
   * @return            Quantidade de bytes carregados; 0 se houve falha
   *                    (arquivo inexistente, vazio ou maior que a memória
   *                    disponível a partir de startAddr).
   */
  uint32_t load(const std::string& path, Memory& memory, uint16_t startAddr = 0x0000);
};
