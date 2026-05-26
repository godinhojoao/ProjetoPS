#include <iostream>
#include "vm/vm.h"

int main(int argc, char** argv)
{
    VM vm;

    // se passaram um .bin como argumento, carrega antes de executar
    if (argc > 1)
    {
        if (!vm.load(argv[1]))
        {
            std::cerr << "main: falha ao carregar '" << argv[1] << "'\n";
            return 1;
        }
    }

    vm.run();
    std::cout << "Hello World on main";
    return 0;
}