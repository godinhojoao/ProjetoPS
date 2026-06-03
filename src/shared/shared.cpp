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

std::string Shared::replaceAllMany(
  const std::string &inputText,
  const std::vector<std::string> &params,
  const std::vector<std::string> &args
) {
  std::string result;
  size_t position = 0;

  while (position < inputText.size())
  {
    bool matched = false;

    for (size_t i = 0; i < params.size(); i++)
    {
      const std::string &parameter = params[i];

      if (parameter.empty())
        continue;

      if (inputText.substr(position, parameter.size()) == parameter)
      {
        result.append(args[i]);
        position += parameter.size();
        matched = true;
        break;
      }
    }

    if (!matched)
    {
      result.push_back(inputText[position]);
      position++;
    }
  }

  return result;
}
