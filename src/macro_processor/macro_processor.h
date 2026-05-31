#include <string>
#include <vector>
#include <cstdint>

class MacroInstruction
{
public:
  std::string expandMacro(std::vector<uint8_t> args);

private:
  std::string label;
  std::string code;
  std::vector<std::string> params;
};
