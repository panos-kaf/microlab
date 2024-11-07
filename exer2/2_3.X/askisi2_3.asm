.include "m328PBdef.inc"

.org 0x00
	rjmp reset
.org 0x04
	rjmp isr1

.def temp = r18
.def led = r19
.def F1_low = r24
.def F1_high = r25

.equ FOSC_MHZ=16        ; MHz
.equ DEL_mS=1000           ; mS
.equ F1=FOSC_MHZ*DEL_mS
    
	
reset:
	ldi temp, LOW(RAMEND)
	out SPL, temp
	ldi temp, HIGH(RAMEND)
	out SPH, temp
	
	ldi F1_low, low(F1)          
    ldi F1_high, high(F1)         ; Set delay
	
	ldi temp,(0<<ISC10)|(1<<ISC11)
	sts EICRA,temp

	ldi temp,(1<<INT1)
	out EIMSK,temp
	sei
	
	ser temp 
	out DDRB,temp
	clr temp
	out DDRD,temp
	
	
main:
	clr led
	out PORTB, led
	rjmp main


delay_outer:	;this routine is used to produce a delay of 1000*F1 cycles
    push F1_low              ; 2 cycles
    push F1_high              ; 2 cycles

outer_loop:
    rcall delay_inner     ; 3+993=996 cycles
    sbiw F1_low,1            ; 2 cycles
    brne outer_loop       ; 1 or 2 cycles

    pop F1_high               ; 2 cycles
    pop F1_low               ; 2 cycles
    ret                   ; 4 cycles
	
delay_inner:			  ;this routine is used to produce a delay 993 cycles
    ldi r21, 247          ; 1 cycle
inner_loop:
    dec r21               ; 1 cycle
    nop                   ; 1 cycle
    brne inner_loop       ; 1 or 2 cycles
    nop                   ; 1 cycle
    ret                   ; 4 cycles
	
isr1:
	in temp,SREG
	push temp
	sei
restart:					
	ldi temp,(1<<INTF1)
	out EIFR,temp
	call delay_inner
	call delay_inner
	call delay_inner
	in temp,EIFR
	andi temp,(1<<INTF1)	
	cpi temp,(1<<INTF1)
	breq restart
	
	andi led,0x01			
	cpi led,0x01
	brne start
	ldi led,0xFF
	out PORTB,led
	call delay_outer
	
start:
	ldi led,0x01
	out PORTB,led
	call delay_outer
	call delay_outer
	call delay_outer
	call delay_outer
	call delay_outer
	
	pop temp
	out SREG,temp
	cli
	reti

	
	

	
	