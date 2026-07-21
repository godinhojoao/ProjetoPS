; ================================================
; Teste de BUILD (macro + assembler + linker juntos)
; Modulo 1 (principal): usa macro local E chama
; simbolo externo de 4_build_lib.asm
; ================================================

MODULE principal_build
PUBLIC MAIN
EXTERN SOMA_LIB, SUB_LIB

.macro CARREGA p1, p2
  LD A, p1
  LD B, p2
.endm

MAIN:
    CARREGA 0x08, 0x02
    CALL SOMA_LIB
    LD C, A
    CALL SUB_LIB
    HLT
    END
