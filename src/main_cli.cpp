#include <iostream>
#include "vm/vm.h"
#include <string>
#include <vector>
#include "macro_processor/macro_processor.h"
#include <fstream>

int main(int argc, char** argv){
  MacroProcessor macroProcessor;
  macroProcessor.findAndStoreMacros("./tests/asm_codes/c.asm");
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

// #include <fstream>
// #include <vector>
// #include <cstdint>

// int main() {
//     std::vector<uint8_t> program = {
//         0x00,
//         0x41,
//         0x3E, 0x05,
//         0x0C,
//         0x25,
//         0x80,
//         0x92,
//         0xA3,
//         0xB5,
//         0xA8,
//         0xBF,
//         0xC5,
//         0xC1,
//         0x76
//     };

//     std::ofstream file("test.bin", std::ios::binary);
//     file.write(reinterpret_cast<const char*>(program.data()), program.size());
// }
