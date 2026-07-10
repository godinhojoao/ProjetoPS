#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include "../vm/vm.h"

class MacroInstruction
{
public:
  void setLabel(const std::string& label) {
    this->label = label;
  }
  void appendCode(const std::string& code) {
    this->code += code + "\n";
  }
  void setCode(const std::string& c) {
    this->code = c;
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
    void reset();
    const std::string getExpandedCode() {
        return this->expandedCode;
    };

private:
  std::unordered_map<std::string, MacroInstruction> macroInstructions;
  std::unordered_map<std::string, bool> validInstructions = {
    {"LD", true},
    {"ADD", true},
    {"SUB", true},
    {"INC", true},
    {"DEC", true},
    {"CP", true},

    {"AND", true},
    {"OR", true},
    {"XOR", true},

    {"JP", true},
    {"JR", true},
    {"CALL", true},
    {"RET", true},

    {"PUSH", true},
    {"POP", true},

    {"NOP", true},
    {"HALT", true}
  };
  std::string expandedCode;
  std::string expandMacro(MacroInstruction instruction, std::vector<std::string> args);

  // controle de fluxo
  std::stack<MacroInstruction> macroInstructionsStack;

  bool isReadingMacro = false;
  int openMacros = 0;
  MacroInstruction& current(){
    return macroInstructionsStack.top();
  }
  void openM(){
    isReadingMacro = true;
    openMacros++;
    macroInstructionsStack.push(MacroInstruction{});
    std::cout << "openM this->openMacros:  " << this->openMacros << "\n";
  }
  void closeM(){
    this->macroInstructions.emplace(current().getLabel(), current());
    macroInstructionsStack.pop();
    openMacros--;
    isReadingMacro = openMacros > 0;
    std::cout << "closeM this->openMacros:  " << this->openMacros << "\n";
  }
};

