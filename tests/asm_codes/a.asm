estado atual:""


bug se tiver macro dentro de codigo (macros no z80 ficam somente no inicio do codigo asm)


NOP
NOP
ADD_VALS 0x05, 0x05
.macro ADD_VALS p1, p2 ////// ISSO NAO PODE
  NOP
  LD B, p1
  LD A, p2
  ADD A, B
 HLT
.endm
NOP
HLT