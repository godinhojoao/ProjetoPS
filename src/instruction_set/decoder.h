#ifndef DECODER_H
#define DECODER_H
//Augusto Menchaca
#include <cstdint>
#include "instruction.h"
#include "../memory/memory.h"

class Decoder {
public:
    static Instruction decode(const Memory& memory, uint16_t pc);
};

#endif
