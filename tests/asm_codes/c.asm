.macro ADD1
  NOP
  .macro ADD2
    LD C, 0x05
  .endm

  ADD2
  HLT
.endm

ADD1

