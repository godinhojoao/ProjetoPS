#include <vector>
#include <string>

class Shared
{
public:
  static std::vector<std::string> split(std::string text, char spliter);
};