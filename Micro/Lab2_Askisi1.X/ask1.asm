.include "m328PBdef.inc"
	
.def temp=r20
	
.org 0x0
    rjmp reset
.org 0x004
    rjmp interrupt1

reset:
    ldi temp,high(RAMEND)
    out SPH,temp
    ldi temp,low(RAMEND)
    out SPL,temp
    
    ser temp
    out DDRC,temp
    
    clr temp
    out DDRD,temp
    
    ldi temp,(1<<ISC10)|(1<<ISC11)
    sts EICRA,temp
	
    ldi temp,(1<<INT1)
    out EIMSK,temp
    sei

    clr r21    
	;out PORTC, r21
loop:
	
	out PORTC,r21
	rjmp loop
	
interrupt1:
	
	in r27, PIND
	sbrs r27, 0x07
	rjmp skip
	inc r21	
	
	; DELAY 100
	ldi r24, low(16*100) ; Init r25, r24 for delay 500 mS
	ldi r25, high(16*100) ; CPU frequency = 16 MHz
delay1:
	ldi r23, 249 ; (1 cycle)
delay2:
	dec r23 ; 1 cycle
	nop ; 1 cycle
	brne delay2 ; 1 or 2 cycles
	sbiw r24, 1 ; 2 cycles
	brne delay1
    
	cpi r21,0x10 ;z=0
	brne skip
	clr r21
    
skip:
	ldi r24, (1 << INTF1)
    out EIFR, r24
    reti


