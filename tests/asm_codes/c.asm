.macro ADD1
  NOP

  .macro ADD2
    .macro ADD99
     .macro ADD991
        LD C, 0x01
      .endm

      ADD991
      LD C, 0x02

    .endm

    ADD99

    LD C, 0x03
  .endm

  .macro ADD3
    LD C, 0x00
  .endm

  ADD3
  ADD2
  HLT
.endm

NOP
ADD1
NOP
