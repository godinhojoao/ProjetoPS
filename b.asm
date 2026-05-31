.macro ADD_VALS p1, p2
  NOP
  LD B, p1
  LD A, p2
  ADD A, B
  HALT
.endm

NOP
NOP
ADD_VALS 0x05, 0x05
NOP
HALT