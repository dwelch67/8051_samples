

    ;.equ    P0,0x80
    ;.equ    P0MDIN,0xF1
    ;.equ    P0MDOUT,0xA4
    ;.equ    P0SKIP,0xD4
    ;.equ    XBR0,0xE1
    ;.equ    XBR1,0xE2
    ;.equ    XBR1,0xE3

.org 0

top:
    mov a,#0xFF
    mov 0xF1,a
    mov a,#0x01
    mov 0xA4,a
    mov a,#0x40
    mov 0xE2,a

toggle:
    mov a,#0x00
    mov 0x80,a

    mov r6,#0
d06:
    nop
    nop
    nop
    djnz r6,d06


    mov a,#0x01
    mov 0x80,a


    ajmp toggle

