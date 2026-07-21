; ================================================
; Teste do ASSEMBLER (sem modularidade / sem linker)
; Objetivo: cobrir os modos de enderecamento e
; instrucoes suportadas na primeira etapa.
; ================================================

ORG 0x0000

LIMITE: EQU 5

START:
    LD A, 0x01
    LD B, LIMITE
    LD C, 0x02
    LD D, 0x03
    LD E, 0x04
    ADD A, B
    SUB C
    AND D
    OR E
    XOR A
    CP B
    INC A
    DEC B
    LD H, A
    LD L, B
    LD (HL), A
    LD A, (HL)
    LD (DADO), A
    LD A, (DADO)
    PUSH BC
    POP DE
    LD (IX+2), A
    LD A, (IY+3)
    ADD A, (IX+1)

LOOP:
    DEC B
    JR LOOP
    JP FIM

DADO: DS 1
FIM:
    HLT
    END
