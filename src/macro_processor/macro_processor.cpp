
// [ ] - processador de macros (tradutor)
//   - macro: (subrotina aberta), label com codigo associado
//   - ex chamando macro: SOMA 5,2
//   exemplo de macros z80: https://github.com/jhlagado/firth/blob/master/macros-stack.z80
//     - achar .macro (.endm é o fim) no codigo, pegar codigo e parametros, ir salvando num objeto array de macros ou map (hashtable)
//     - e ai dps fica facil pegar pelo nome
//   1. localiza macro
//   2. substitui troca instrucao pelo esqueleto (codigo associado)
//   3. parametros, substitui valores genericos

#include "macro_processor.h"

std::string MacroInstruction::expandMacro(std::vector<uint8_t> args)
{
  std::string expandedMacro;

  // this->code


  return expandedMacro;
}