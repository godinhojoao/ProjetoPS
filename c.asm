.macro OUTER

  .macro MIDDLE

    .macro INNER
      NOP
      HALT
    .endm

    NOP
    INNER

  .endm

  NOP

  MIDDLE
.endm

START:
NOP
OUTER
END:
HALT
