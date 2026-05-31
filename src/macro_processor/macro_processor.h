#include <string>
#include <vector>
#include <cstdint>

class MacroInstruction
{
private:
  std::string label;
  std::string code;
  std::vector<std::string> params;
};

class MacroProcessor
{
public:
  static MacroProcessor findMacro(std::string code);
  std::string expandMacro(MacroInstruction instruction, std::vector<uint8_t> args);
};

