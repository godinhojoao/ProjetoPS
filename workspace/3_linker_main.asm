; ================================================
; Teste do LINKER - Modulo 1 (principal)
; Usa EXTERN para chamar simbolos definidos no
; modulo 3_linker_mathlib.asm
; ================================================

MODULE principal
PUBLIC START
EXTERN SOMA_EXT, SUBTRAI_EXT

START:
    LD A, 10
    LD B, 5
    CALL SOMA_EXT
    LD C, A
    CALL SUBTRAI_EXT
    LD (RESULTADO), A
    JP FIM

RESULTADO: DS 1

FIM:
    HLT
    END
