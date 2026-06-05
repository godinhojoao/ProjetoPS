// TODO: faltou só separar melhor o codigo, matar uns ifs, etc
// se tiver macro com mesmo nome uma definida após a outra, a primeira é sobrescrita
#include <shared/shared.h>
#include "macro_processor/macro_processor.h"

enum MACRO_READING_STATE
{
  LABEL = 0,
  PARAM = 1,
  CODE = 2
};

std::string MacroProcessor::expandMacro(MacroInstruction instruction, std::vector<std::string> args) {
  std::vector<std::string> instructionParams = instruction.getParams();
  bool isValidArgsSize = instructionParams.size() == args.size();
  if (!isValidArgsSize) {
    throw std::runtime_error("chamada de macro com quantidade de argumentos invalida");
  }

  for (std::string &arg : args) {
    size_t comma = arg.find(',');
    bool existComma = (comma != std::string::npos);
    if (existComma) arg.erase(comma);
  }

  std::string macroCode = instruction.getCode();
  return instructionParams.size() == 0 ? macroCode : Shared::replaceAllMany(macroCode, instructionParams, args);
}

void MacroProcessor::findAndStoreMacros(std::string file)
{
  std::ifstream inFile;
  inFile.open(file);
  if (!inFile)
    throw std::runtime_error("Cannot open file");

  // 1. reading line by line and processing without holding entire file content
  std::string line;
  bool isReadingMacro = false;
  bool hasFinishedMacroRead = false;
  MacroInstruction instruction;
  MACRO_READING_STATE currMacroReadingState = LABEL;

  while (std::getline(inFile, line))
  {
    // validations
    line = Shared::trim(line); // remove comentarios e espacos
    //std::cout << "line->" << line <<"\n";
    if (line.size() == 0) continue;

    if (!isReadingMacro && line.find(".endm") == 0) {
      throw std::runtime_error("Nao pode fechar macro que nao foi aberta");
    }

    if (hasFinishedMacroRead && line.find(".macro") == 0) {
      throw std::runtime_error("Nao pode ler macro");
    }

    // reading macro
    if (line.find(".macro") == 0 && !isReadingMacro) {
      isReadingMacro = true;
      std::vector<std::string> splittedResult = Shared::split(line, ' ');

      for (int i = 0; i < splittedResult.size(); ++i)
      {
        std::string currChunk = splittedResult[i];
        if (currMacroReadingState == LABEL && i == 1)
        {
          instruction.setLabel(currChunk);
          bool hasParams = splittedResult.size() > 2;
          currMacroReadingState = hasParams ? PARAM : CODE;
          continue;
        }

        if (currMacroReadingState == PARAM)
        {
          bool isLastParam = i == splittedResult.size() - 1;
          instruction.pushParam(isLastParam ? currChunk : currChunk.substr(0, currChunk.size() - 1));
          currMacroReadingState = isLastParam ? CODE : PARAM;
          continue;
        }
      }

      continue;
    }

    if (line.find(".endm") == 0 && isReadingMacro) {
      isReadingMacro = false;
      this->macroInstructions.emplace(instruction.getLabel(), instruction);
      instruction = MacroInstruction{};
      currMacroReadingState = LABEL;
      continue;
    }

    if (currMacroReadingState == CODE) {
      instruction.appendCode(line);
    }

    hasFinishedMacroRead = !isReadingMacro && line.find(".macro") != 0;

    // reading code
    if (hasFinishedMacroRead) {
      std::vector<std::string> splittedCodeLine = Shared::split(line, ' ');
      std::string firstChunk = splittedCodeLine[0];

      bool isValidInstruction = this->validInstructions.count(firstChunk);
      if (isValidInstruction) {
        this->expandedCode.append(line + "\n");
      }

      auto it = this->macroInstructions.find(firstChunk);
      MacroInstruction currMacroInstruction;
      if (it != this->macroInstructions.end()) {
        currMacroInstruction = it->second;
        std::vector<std::string> macroArguments(splittedCodeLine.begin() + 1, splittedCodeLine.end());

        std::string expandedCode = this->expandMacro(currMacroInstruction, macroArguments);
        this->expandedCode.append(expandedCode);
        continue;
      }

      if (!isValidInstruction) {
        throw std::runtime_error("nao é macro valida, nem instrucao valida");
      }
    }
  }

  if(isReadingMacro) {
    throw std::runtime_error("abriu macro e nao fechou");
  }
  inFile.close();
}
