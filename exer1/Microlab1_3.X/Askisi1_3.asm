.include "m328PBdef.inc"
	
.equ FOSC_MHZ=16		; MHz
.equ DEL_mS=1000        ; mS
.equ F1=FOSC_MHZ*DEL_mS
	
.def temp=r16
.def Bit=r17
	
main:
	ldi r24, LOW(RAMEND)
    out SPL, r24
    ldi r24, HIGH(RAMEND)
    out SPH, r24

	ser r26					
    out DDRD, r26			;Set D as Output
    ldi r24, low(F1)          
    ldi r25, high(F1)       ;Set delay
	ldi Bit,0x01
	
left_start:
	call delay_outer		;Extra delay
left:
	set
	out PORTD,Bit
	lsl Bit
	call delay_outer
	cpi Bit,0x80
	brne left
	
right_start:
	call delay_outer		;Extra delay
right:
	clt
	lsr Bit
	out PORTD,Bit
	call delay_outer
	cpi Bit,0x01
	brne right_start
	jmp left_start
	
delay_outer:	;this routine is used to produce a delay of 1000*F1 cycles
    push r16              ; 2 cycles
    push r17              ; 2 cycles

outer_loop:
    rcall delay_inner     ; 3+993=996 cycles
    sbiw r16,1            ; 2 cycles
    brne outer_loop       ; 1 or 2 cycles
    pop r17               ; 2 cycles
    pop r16               ; 2 cycles
    ret                   ; 4 cycles
	
delay_inner:			  ;this routine is used to produce a delay 993 cycles
    ldi r21, 247          ; 1 cycle
inner_loop:
    dec r21               ; 1 cycle
    nop                   ; 1 cycle
    brne inner_loop       ; 1 or 2 cycles
    nop                   ; 1 cycle
    ret                   ; 4 cycles
