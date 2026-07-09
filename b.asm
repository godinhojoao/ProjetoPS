.macro ADD_VALS ; comentario
  NOP
  LD B, 0x05
  LD A, 0x05
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
ADD_VALS;comentario
ADD_VALS2 0x03, 0x02
NOP  ;comentario
END:
 HALT