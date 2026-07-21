; ================================================
; Teste do PROCESSADOR DE MACROS
; Objetivo: expandir macros com parametros e macros
; sem parametro, chamadas em sequencia.
; ================================================

.macro SOMA p1, p2
  LD A, p1
  LD B, p2
  ADD A, B
.endm

.macro DOBRA p1
  LD A, p1
  ADD A, A
.endm

.macro ZERA_TUDO
  LD A, 0x00
  LD B, 0x00
  LD C, 0x00
.endm

NOP
SOMA 0x05, 0x03
DOBRA 0x04
ZERA_TUDO
SOMA 0x0A, 0x0A
HLT
