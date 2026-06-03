.macro ADD_VALS p1, p2
  NOP
  LD B, p1
  LD A, p2
  ADD A, B;comentario
  HALT
.endm

.macro ADD_VALS2 p1, p2 ;comentario
  NOP
  LD B, p1
  LD A, p2  ;comentario
  ADD A, B
  HALT
.endm
;comentario
NOP
 NOP
ADD_VALS 0x05, 0x04;comentario
ADD_VALS2 0x03, 0x02
NOP  ;comentario
HALT