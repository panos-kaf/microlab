.include "m328PBdef.inc"    ; ATmega328P microcontroller definitions

.org 0x00 
	rjmp reset
.org 0x02
	rjmp isr0
	
	
.equ FOSC_MHZ = 16          ; Microcontroller operating frequency in MHz
.equ DEL_mS = 100; Delay in mS (valid number from 1 to 4095)
.equ DEL_NU = FOSC_MHZ * DEL_mS ; delay_mS routine: (1000 * DEL_NU + 6) cycles
.def counter = r27
.def temp = r28
	
reset:
; Init Stack Pointer
ldi temp, LOW(RAMEND)
out SPL, temp
ldi temp, HIGH(RAMEND)
out SPH, temp
	
		
ser temp
out DDRC,temp
	
ldi r24, low(DEL_NU)
ldi r25, high(DEL_NU)   ; Set delay (number of cycles)

ldi temp,(0<<ISC00)|(0<<ISC01)
sts EICRA,temp
    
ldi temp,(1<<INT0)
out EIMSK,temp
sei

ldi counter,0

; Init PORTB as output
clr temp
out DDRB, temp
clr temp
out DDRD, temp
	
loop1:
    clr r26
loop2:
    out PORTC, r26
    rcall delay_mS
    inc r26
    cpi r26, 32             ; compare r26 with 32
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

isr0:
	in temp,SREG
	push temp
	in temp,PINB
	com temp
	andi temp,15

	sbrc temp,0
	inc counter
	sbrc temp,1
	inc counter
	sbrc temp,2
	inc counter
	sbrc temp,3
	inc counter
	
	ldi temp,1
loop3:
	cpi counter,0
	breq output
	dec counter
	lsl temp
	rjmp loop3
		
output:
	dec temp
	out PORTC,temp
	clr counter
	pop temp
	reti
