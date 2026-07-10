#include "shared.h"
#include <sstream>
#include <algorithm>

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

std::string Shared::trim(const std::string &text) {
    if (text.empty()) return text;

    size_t preComment = text.find(';');
    bool existComment = (preComment != std::string::npos);
    size_t end = existComment ? preComment : text.size();

    // Extrai a parte sem comentários
    std::string noComment = text.substr(0, end);

    // Remove whitespaces (\r, \n, \t, espaços) das extremidades
    size_t startIdx = noComment.find_first_not_of(" \t\r\n");
    if (startIdx == std::string::npos) return "";

    size_t endIdx = noComment.find_last_not_of(" \t\r\n");
    return noComment.substr(startIdx, endIdx - startIdx + 1);
}

uint16_t Shared::parseAddress(const std::string &token) {
    return static_cast<uint16_t>(std::stoul(token, nullptr, 0));
}

bool Shared::isSectionKeyword(const std::string &line) {
    static const std::string SECTION_KEYWORDS[] = {
        "HEADER", "EXTDEF", "EXTREF", "REALOC", "CODE"
    };
    for (const auto& keyword : SECTION_KEYWORDS) {
        if (line == keyword) {
            return true;
        }
    }
    return false;
}

std::string Shared::toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}