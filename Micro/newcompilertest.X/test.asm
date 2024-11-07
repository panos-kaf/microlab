.include "m328PBdef.inc"
.def A = R16
.def B = R17
.def C = R18
.def D = R19
.org 0x0000



start:
    ldi A, 0x00		//ldi temp, 0x00
    out DDRB, A
    ldi A, 0xFF		// ldi temp,0xFF
    out PORTB, A
    out DDRC, A

check_f0:

    in A, PINB
    mov C, A		
    andi C, 0x03
    cpi C, 0x01
    breq one_f0
    mov B,A
    andi B,0x0A
    cpi B, 0x0A
    breq one_f0

    ldi C,0xFE		// For f0 = 1 logw antistrofis logikis

check_f1: 

    mov D, A
    andi D, 0x05
    cpi D, 0x05
    breq one_f1
    mov D, A
    andi D, 0x0A
    cpi D, 0x02
    breq one_f1

    ldi D,0x02		// for f1 = 1 logw antistrofis logikis
    or C,D
    out PORTC, C
    jmp check_f0

one_f0:

    ldi C,0x01
    jmp check_f1

one_f1: 

    ldi D,0x02
    or C,D
    out PORTC, C
    jmp check_f0