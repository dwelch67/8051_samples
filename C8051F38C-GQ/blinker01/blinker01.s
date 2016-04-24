

    ;.equ    P0,0x80
    ;.equ    P0MDIN,0xF1
    ;.equ    P0MDOUT,0xA4
    ;.equ    P0SKIP,0xD4
    ;.equ    XBR0,0xE1
    ;.equ    XBR1,0xE2
    ;.equ    XBR1,0xE3
    ;.equ    PCA0MD,0xD9

.org 0
    sjmp reset
    nop
    nop
    nop
    nop
    nop
    nop
    nop
hang: sjmp hang

reset:
    mov a,#0x00
    mov 0xD9,a
    acall port_init
toggle:
    acall led_off
    acall delay
    acall led_on
    acall delay
    sjmp toggle

delay:
    mov r5,#0
    mov r6,#0
d6:
    djnz r6,d6
    djnz r5,d6
    ret

port_init:
    mov a,#0xFF
    mov 0xF1,a
    mov a,#0x03
    mov 0xA4,a
    mov a,#0x40
    mov 0xE2,a
    ret

led_on:
    mov a,#0x03
    mov 0x80,a
    ret

led_off:
    mov a,#0x00
    mov 0x80,a
    ret
    
    

