#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "vm/vm.h"

class MacroInstruction
{
public:
  void setLabel(const std::string& label) {
    this->label = label;
  }
  void appendCode(const std::string& code) {
    this->code += code;
  }
  void pushParam(const std::string& param) {
    params.push_back(param);
  }

  const std::string& getLabel() const {
    return label;
  }
  const std::string& getCode() const {
    return code;
  }
  const std::vector<std::string>& getParams() const {
    return params;
  }

private:
  std::string label;
  std::string code;
  std::vector<std::string> params;
};

class MacroProcessor
{
public:
  void findAndStoreMacros(std::string file);
  // std::string expandMacro(MacroInstruction instruction, std::vector<uint8_t> args);

private:
  //std::vector<MacroInstruction> macroInstructions;
  std::unordered_map<std::string, MacroInstruction> macroInstructions ;
};

