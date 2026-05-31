#include <iostream>
#include "vm/vm.h"
#include <string>
#include <vector>
#include "macro_processor/macro_processor.h"
#include <fstream>
#include <sstream>

std::vector<std::string> split(std::string text, char spliter){
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

void demoFileStreams()
{
  // !!!!! it handles unicode for us automatically without making switch cases or if's statements to handle utf-8 or etc
  // std::ifstream (input) -> read files
  // std::ofstream (output) -> write files

  std::ifstream inFile;
  inFile.open("b.asm");
  if (!inFile)
    throw std::runtime_error("Cannot open input.txt");
  std::cout << "inFile.is_open: " << (inFile.is_open() ? "yes" : "not") << "\n";

  // 1. reading line by line and processing without holding entire file content
  std::string line;
  bool isReadMacro = false;
  while (std::getline(inFile, line))
  {
    std::cout << "line: " << line << "\n";
    if (line.find(".macro") == 0 && !isReadMacro) {
      isReadMacro = true;
      std::vector<std::string> nome = split(line, ' ');

      for ( int i = 0; i < nome.size(); ++i ) {
        std::cout << "######## "<< nome[i] << "\n";
      }
    }

  }
  inFile.close();
}

int main(int argc, char **argv)
{
  demoFileStreams();
  std::vector<MacroInstruction> macroInstructions;

  std::string code = ".macroADD_VALS p1, p2 NOPLDB,p1LDA,p2ADDA,BHALT.endmNOPNOPADD_VALS0x05,0x05NOPHALT";

  std::string text = "apple, banana, cherry";
  std::stringstream ss(text);
  std::string token;
  std::vector<std::string> result;

  // Extract substrings sequentially using ',' as the delimiter
  while (std::getline(ss, token, ','))
  {
    result.push_back(token);
  }

  // Print results
  for (const auto &str : result)
  {
    std::cout << str << '\n';
  }

  // VM vm;

  // se passaram um .bin como argumento, carrega antes de executar
  // if (argc > 1)
  // {
  //     if (!vm.load(argv[1]))
  //     {
  //         std::cerr << "main: falha ao carregar '" << argv[1] << "'\n";
  //         return 1;
  //     }
  // }

  // vm.run();

  // antes tiramos espacos em branco do codigo

  std::cout << "Hello World on main";
  return 0;
}