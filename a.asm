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


1o acha .macro
2o apos espaco => label
3o apos espaco params separados por virgula (primeira quebra de linha comeca o codigo)
4o cada linha tem codigo até achar o .endm (ai acabou a macroinstruction)

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





NOP
NOP
LD B, 0x05
LD A, 0x05
ADD A, B
HALT
NOP
HALT