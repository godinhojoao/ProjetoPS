.macro ADD_VALS p1, p2
  NOP
  LD B, p1
  LD A, p2
  ADD A, B
  HALT
.endm

.macro ADD_VALS2 p1, p2
  NOP
  LD B, p1
  LD A, p2
  ADD A, B
  HALT
.endm

NOP
NOP
ADD A, B
ADD_VALS 0x01, 0x02
ADD_VALS2 0x03, 0x04
NOP
HALT