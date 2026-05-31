// 1o estado atual:""
// bug se tiver macro dentro de codigo (macros no z80 ficam somente no inicio do codigo asm)
// NOP
// NOP
// ADD_VALS 0x05, 0x05
// .macro ADD_VALS p1, p2 ////// ISSO NAO PODE
//   NOP
//   LD B, p1
//   LD A, p2
//   ADD A, B
//   HALT
// .endm
// NOP
// HALT

// 2o outro bug -> permitimos macros com mesmo nome (nao deveria poder -- sobrescreve ou da erro?)
// 3o expand nem comecamos -> cuidar tamanho de parametros = argumentos

#include <shared/shared.h>
#include "macro_processor/macro_processor.h"

enum MACRO_READING_STATE
{
  LABEL = 0,
  PARAM = 1,
  CODE = 2
};

void MacroProcessor::findAndStoreMacros(std::string file)
{
  std::ifstream inFile;
  inFile.open(file);
  if (!inFile)
    throw std::runtime_error("Cannot open file");

  // 1. reading line by line and processing without holding entire file content
  std::string line;
  bool isReadingMacro = false;
  MacroInstruction instruction;
  MACRO_READING_STATE currReadingState = LABEL;

  while (std::getline(inFile, line))
  {
    std::cout << "line->" << line << "\n";

    if (line.find(".macro") == 0 && !isReadingMacro)
    {
      isReadingMacro = true;
      std::vector<std::string> splittedResult = Shared::split(line, ' ');

      for (int i = 0; i < splittedResult.size(); ++i)
      {
        std::string currChunk = splittedResult[i];
        if (currReadingState == LABEL && i == 1)
        {
          instruction.setLabel(currChunk);
          currReadingState = PARAM;
          continue;
        }

        if (currReadingState == PARAM)
        {
          bool isLastParam = i == splittedResult.size() - 1;
          instruction.pushParam(isLastParam ? currChunk : currChunk.substr(0, currChunk.size() - 1));
          currReadingState = isLastParam ? CODE : PARAM;
          continue;
        }
      }

      continue;
    }

    if (line.find(".endm") == 0 && isReadingMacro)
    {
      isReadingMacro = false;
      this->macroInstructions.push_back(instruction);
      instruction = MacroInstruction{};
      currReadingState = LABEL;
      continue;
    }

    if (currReadingState == CODE)
    {
      instruction.appendCode(line);
    }
  }

  // debug
  for (int i = 0; i < this->macroInstructions.size(); ++i)
  {
    std::cout << "label->" << this->macroInstructions[i].getLabel() << "\n";
    for (int j = 0; j < this->macroInstructions[i].getParams().size(); ++j)
    {
      std::cout << "params->" << this->macroInstructions[i].getParams()[j] << "\n";
    }
    std::cout << "code->" << this->macroInstructions[i].getCode() << "\n";
  }
  inFile.close();
}

// std::string MacroProcessor::expandMacro(std::vector<uint8_t> args)
// {
//   std::string expandedMacro;

//   // this->code

//   return expandedMacro;
// }
