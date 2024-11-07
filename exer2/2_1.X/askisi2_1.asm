.include "m328PBdef.inc"    ; ATmega328P microcontroller definitions

.org 0x00 
	rjmp reset
.org 0x04
	rjmp isr1
	
	
.equ FOSC_MHZ = 16              ; Microcontroller operating frequency in MHz
.equ DEL_mS = 5000              ; Delay in mS (valid number from 1 to 4095)
.equ DEL_NU = FOSC_MHZ * DEL_mS ; delay_mS routine: (1000 * DEL_NU + 6) cycles
.def int_counter = r27
.def temp = r28
reset:
; Init Stack Pointer
ldi r24, LOW(RAMEND)
out SPL, r24
ldi r24, HIGH(RAMEND)
out SPH, r24
	
ser temp
out DDRC,temp


ldi temp,(0<<ISC10)|(1<<ISC11)
sts EICRA,temp
    
ldi temp,(1<<INT1)
out EIMSK,temp
sei

ldi int_counter,0

; Init PORTB as output
ser r26
out DDRB, r26
clr temp
out DDRD, temp
	
loop1:
    clr r26
loop2:
    out PORTB, r26
	out PORTC, int_counter
    ldi r24, low(DEL_NU)
    ldi r25, high(DEL_NU)   ; Set delay (number of cycles)
    rcall delay_mS
    inc r26
    cpi r26, 16             ; compare r26 with 16
    breq loop1
    rjmp loop2

; delay of 1000*F1+6 cycles (almost equal to 1000*F1 cycles)
delay_mS:
    ; total delay of next 4 instruction group = 1 + (249 * 4 - 1) = 996 cycles
    ldi r23, 249            ; (1 cycle)
loop_inn:
    dec r23                 ; 1 cycle
    nop                     ; 1 cycle
    brne loop_inn            ; 1 or 2 cycles
    sbiw r24, 1             ; 2 cycles
    brne delay_mS            ; 1 or 2 cycles
ret                         ; 4 cycles

isr1:
	push r24
	push r25
loop4:
	in temp,PIND
	andi temp,0x20
	cpi temp,0x20
	brne loop4
	
loop3:
	ldi temp,(1<<INTF1)
	out EIFR,temp
	ldi r24,0
	ldi r25,10
	call delay_mS
	in temp,EIFR
	andi temp,(1<<???F1)
	cpi temp,(1<<INTF1)
	breq loop3
	
skip:
	inc int_counter
	out PORTC,int_counter
	pop r25
	pop r24
	reti
	
	
