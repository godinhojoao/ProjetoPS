#pragma once
#include <cstdint>
#include "../constants.h"

class Memory
{
public:
  uint8_t read(uint16_t addr) const;
  bool write(uint16_t addr, uint8_t value);
  bool stackWrite(uint16_t addr, uint8_t value);

  uint32_t getCodeEnd() const;
  void setCodeEnd(uint32_t end);

  uint32_t getDataEnd() const;
  void setDataEnd(uint32_t end);

  void resetSegments();

  //funções pra expor ponteiro de leitura pra memória e tamanho
  const uint8_t* raw() const { return data; }
  static constexpr uint32_t SIZE = VM_MEMORY_IN_BYTES; // permite acessar com vm::SIZE e custa 1x pra cada instancia da vm
private:
  uint8_t data[VM_MEMORY_IN_BYTES] = {};
  uint32_t codeEnd = 0;
  uint32_t dataEnd = 0;
};
