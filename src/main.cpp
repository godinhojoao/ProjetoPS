#include <iostream>
#include "vm/vm.h"
#include <string>
#include <vector>

int main(int argc, char** argv)
{
    std::vector<macroinstruction> macroinstructions;

    for(item in  macroinstructions) {
        code = Macroinstruction::expand()
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
    std::string code = ".macroADD_VALS p1, p2 NOPLDB,p1LDA,p2ADDA,BHALT.endmNOPNOPADD_VALS0x05,0x05NOPHALT";





    std::cout << "Hello World on main";
    return 0;
}