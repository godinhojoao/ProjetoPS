; ================================================
; Teste do LINKER - Modulo 2 (biblioteca)
; Exporta SOMA_EXT e SUBTRAI_EXT via PUBLIC para
; serem usados pelo modulo 3_linker_main.asm
; ================================================

MODULE mathlib
PUBLIC SOMA_EXT, SUBTRAI_EXT

SOMA_EXT:
    ADD A, B
    RET

SUBTRAI_EXT:
    SUB B
    RET
    END
