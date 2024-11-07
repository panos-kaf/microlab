.include "m328PBdef.inc"
	
.def A = R16
.def temp = R17
.def output = R18
	
.org 0x0000

start:
    clr A
	clr output
    out DDRB, A
    ser A
    out PORTB, A
    out DDRC, A

check_f0:

    in A, PINB
    mov temp, A
	andi temp,0x03
	cpi temp,0x01
	breq F0_IS_ON
	mov temp,A
	andi temp,0x0A
	cpi temp,0x0A
	breq F0_IS_ON
	
F0_IS_OFF:
	andi output,0b00000010
	out PORTC,output
	// set F0 led to OFF
	
check_f1:
	
	mov temp,A
	andi temp,0x05
	cpi temp,0x05
	breq F1_IS_OFF
	
	mov temp,A
	andi temp,0x0A
	cpi temp,0x02
	breq F1_IS_OFF
	
F1_IS_ON:
	ori output,0b00000010
	out PORTC,output
	//Set F1 Led to ON
	rjmp check_f0
	
F0_IS_ON:
	ori output,0b00000001
	out PORTC,output
	//Set F0 Led to ON
	rjmp check_f1
	
F1_IS_OFF:
	andi output,0b00000001
	out PORTC,output
	//set F1 Led to OFF
	rjmp check_f0
	
	//F0 = LSB   , F1= 2nd LSB