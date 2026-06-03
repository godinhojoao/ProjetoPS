#include <iostream>
#include "vm/vm.h"
#include <string>
#include <vector>
#include "macro_processor/macro_processor.h"
#include <fstream>


int main(int argc, char **argv)
{
  MacroProcessor macroProcessor;
  macroProcessor.findAndStoreMacros("b.asm");
  std::string expandedCode = macroProcessor.getExpandedCode();
  std::cout << "expandedCode: \n" << expandedCode << "\n";

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

  std::cout << "Hello World on main";
  return 0;
}