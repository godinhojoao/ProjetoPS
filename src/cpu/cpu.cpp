/*
CPU:
guarda estado da maquina (registradores, flags)
executa operacoes basicas (ADD, SUB, MOV, etc)
e onde as instruções realmente mexem nos dados
*/
#include "cpu.h"
#include <iostream>
#include <vm.h>

void CPU::startRegisters()
{
  CPU::SP = VM_MEMORY_IN_BYTES - 1; // last index in memory
}