#include "shared.h"
#include <sstream>

std::vector<std::string> Shared::split(std::string text, char spliter)
{
  std::stringstream ss(text);
  std::string token;
  std::vector<std::string> result;

  // Extract substrings sequentially using ',' as the delimiter
  while (std::getline(ss, token, spliter))
  {
    result.push_back(token);
  }

  return result;
}
