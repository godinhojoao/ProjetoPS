#pragma once
#include <vector>
#include <string>
#include <cstdint>

// secoes do arquivo objeto .o (object_file_standard.md). None = a linha nao e
// uma palavra-chave de secao.
enum class ObjSection
{
  None,
  Header,
  ExtDef,
  ExtRef,
  Realoc,
  Code
};

class Shared
{
public:
  static std::vector<std::string> split(std::string text, char spliter);
  static std::string replaceAllMany(
    const std::string &inputText,
    const std::vector<std::string> &params,
    const std::vector<std::string> &args
  );
  static std::string trim(const std::string &text);
  static uint16_t parseAddress(const std::string &token);

  /**
   * @brief  Traduz uma linha do .o para a secao que ela abre.
   * @return ObjSection::None se a linha nao for palavra-chave de secao.
   */
  static ObjSection toObjSection(const std::string &line);
  static bool isSectionKeyword(const std::string &line);

  /**
   * @brief  Converte um token numerico sem lancar excecao.
   *
   * Diferente de parseAddress, um token malformado vira `false` em vez de
   * std::invalid_argument — ler um .o corrompido nao deve derrubar a VM.
   *
   * @param  token  Texto a converter (ex: "0x000B", "11", "3E").
   * @param  base   Base numerica; 0 detecta pelo prefixo (0x = hex).
   * @param  out    Saida: valor convertido, so valido quando retorna true.
   * @return        true se o token inteiro foi consumido como numero.
   */
  static bool tryParseNumber(const std::string &token, int base, unsigned long &out);

  static std::string toUpper(std::string s);
};