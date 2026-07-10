#pragma once
#include <vector>
#include <string>
#include <cstdint>

class Shared
{
public:
  static std::vector<std::string> split(std::string text, char spliter);
  static std::string replaceAllMany(
    const std::string &inputText,
    const std::vector<std::string> &params,
    const std::vector<std::string> &args
  );
  static std::string trim(const std::string &text);
  static uint16_t parseAddress(const std::string &token);
  static bool isSectionKeyword(const std::string &line);
  static std::string toUpper(std::string s);
};