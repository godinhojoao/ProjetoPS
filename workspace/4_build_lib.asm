; ================================================
; Teste de BUILD (macro + assembler + linker juntos)
; Modulo 2 (biblioteca)
; ================================================

MODULE biblioteca_build
PUBLIC SOMA_LIB, SUB_LIB

SOMA_LIB:
    ADD A, B
    RET

SUB_LIB:
    SUB B
    RET
    END
