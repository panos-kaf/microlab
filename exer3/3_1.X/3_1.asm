.include "m328PBdef.inc"
	
.def temp = r16
.def DC_VALUE = r17
.def pd3 = r18
.def pd4 = r19
.equ FOSC_MHZ = 16          ; Microcontroller operating frequency in MHz
.equ DEL_mS = 50; Delay in mS (valid number from 1 to 4095)
.equ DEL_NU = FOSC_MHZ * DEL_mS ; delay_mS routine: (1000 * DEL_NU + 6) cycl	
	
.org 0x0
	rjmp reset

array: .DB 5,26,46,67,87,108,128,148,169,189,210,230,251

reset:
	ldi temp,high(RAMEND)
	out SPH,temp
	ldi temp,low(RAMEND)
	out SPL,temp
	
	LDI ZL, LOW(2*array)   ; Load the low byte of the address of my_array into ZL
    LDI ZH, HIGH(2*array)  ; Load the high byte of the address of my_array into ZH
	
	ldi r24, low(DEL_NU)
	ldi r25, high(DEL_NU)   ; Set delay (number of cycles)
	
	ser temp 
	out DDRB,temp
	clr temp
	out DDRD,temp
	
	ldi temp, (1<<WGM10)|(1<<COM1A1)
	sts TCCR1A,temp
	clr temp
	ldi temp, (1<<WGM12)|(1<<CS12)		; Prescaler = 256
	sts TCCR1B,temp
	
	inc ZL
	inc ZL
	inc ZL
	inc ZL
	inc ZL
	inc ZL
	lpm DC_VALUE,Z
loop:
	sts OCR1AL,DC_VALUE
	in pd3,PIND
	rcall delay_outer
	com pd3
	mov pd4,pd3
	andi pd3,0x08
	andi pd4,0x10
	
	cpi pd3,0x08
	breq increase
	cpi pd4,0x10
	breq decrease
	rjmp loop
	
increase:
	cpi DC_VALUE,251
	breq loop
	inc ZL
	lpm DC_VALUE,Z
	rjmp loop
	
decrease:
	cpi DC_VALUE,5
	breq loop
	dec ZL
	lpm DC_VALUE,Z
	rjmp loop
	
	
;this routine is used to produce a delay of 1000*F1 cycles
delay_outer: 
    push r24              
    push r25              

outer_loop:
    rcall delay_inner     ; 3+993=996 cycles
    sbiw r24,1            
    brne outer_loop      

    pop r24               
    pop r25               
	ret                   
	
;this routine is used to produce a delay 993 cycles
delay_inner:			  
    ldi r21, 247          
inner_loop:
	nop                   
    dec r21               
    brne inner_loop       
    nop                   
    ret                   
