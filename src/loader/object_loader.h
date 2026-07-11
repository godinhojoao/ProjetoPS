#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../memory/memory.h"

/*
Conteudo util de um arquivo objeto .o (formato texto de object_file_standard.md)
do ponto de vista do loader: o codigo montado com enderecos relativos ao inicio
do modulo, e a lista de offsets desses enderecos.

As secoes EXTDEF/EXTREF sao reconhecidas mas ignoradas aqui — quem resolve
simbolo externo e o ligador (src/linker/extref_resolver.*).
*/
struct ObjectFile
{
  std::string moduleName;             // campo MODULE do HEADER
  uint16_t declaredSize = 0;          // campo SIZE do HEADER
  std::vector<uint8_t> code;          // secao CODE
  std::vector<uint16_t> relocOffsets; // secao REALOC
};

/*
Loader de arquivo objeto realocavel (Cenario A): le o .o, copia o codigo para
a memoria da VM a partir de um endereco de carga arbitrario e soma esse
endereco aos operandos apontados pela tabela de realocacao.

Para carregar um .bin cru, sem cabecalho nem realocacao, use BinaryLoader.
*/
class ObjectLoader
{
public:
  /**
   * @brief  Le e valida um arquivo objeto .o, sem tocar na memoria da VM.
   *
   * @param  path  Caminho do .o gerado pelo montador/ligador.
   * @param  obj   Saida: header, codigo relativo e tabela de realocacao.
   * @return       true se o arquivo foi lido e a secao CODE nao esta vazia.
   */
  bool parse(const std::string& path, ObjectFile& obj);

  /**
   * @brief  Aplica o offset de carga aos enderecos apontados por REALOC.
   *
   * Cada offset da tabela aponta para um operando de 16 bits little-endian
   * dentro de obj.code (ex: o alvo de um JP/CALL). Somar loadAddr a esse
   * operando transforma o endereco relativo ao modulo em endereco absoluto.
   *
   * Roda no buffer, antes da copia para a memoria: Memory::write recusa
   * escrita abaixo de codeEnd, entao corrigir os bytes ja gravados dependeria
   * da ordem em que a VM chama setCodeEnd.
   *
   * @param  obj       Objeto ja parseado; obj.code e modificado no lugar.
   * @param  loadAddr  Endereco de carga do modulo.
   * @return           false se algum offset aponta fora do codigo ou se o
   *                   endereco realocado passa de 0xFFFF.
   */
  static bool relocate(ObjectFile& obj, uint16_t loadAddr);

  /**
   * @brief  Carrega um .o realocado na memoria da VM (parse + relocate + copia).
   *
   * @param  path      Caminho do .o.
   * @param  memory    Memoria da VM onde o codigo sera gravado.
   * @param  loadAddr  Endereco inicial de carga (padrao 0x0000).
   * @return           Quantidade de bytes gravados; 0 se houve falha.
   */
  uint32_t load(const std::string& path, Memory& memory, uint16_t loadAddr = 0x0000);

private:
  // uma funcao por secao do .o; todas reportam o erro em stderr e devolvem
  // false, o que aborta o parse
  static bool parseHeaderLine(const std::string& line, uint32_t lineNumber, ObjectFile& obj);
  static bool parseRelocLine(const std::string& line, uint32_t lineNumber, ObjectFile& obj);
  static bool parseCodeLine(const std::string& line, uint32_t lineNumber, ObjectFile& obj);

  // checagens que so fazem sentido com o arquivo inteiro lido
  static bool validate(const std::string& path, const ObjectFile& obj);
};
