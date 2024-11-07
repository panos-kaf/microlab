.include "m328PBdef.inc"

.equ FOSC_MHZ=16        ; MHz
.equ DEL_mS=1           ; mS
.equ F1=FOSC_MHZ*DEL_mS
    
; initialize stack pointer
    ldi r16, LOW(RAMEND)
    out SPL, r16
    ldi r16, HIGH(RAMEND)
    out SPH, r16
    
; init portd as output
    ser r20
    out DDRD, r20
    ldi r16, low(F1)          
    ldi r17, high(F1)         ; Set delay

main:
    ser r20
    out PORTD, r20
    rcall delay_outer      
    clr r20
    out PORTD, r20
    rcall delay_outer      
    rjmp main


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
	nop                   ; 1 cycle
    dec r21               ; 1 cycle
    brne inner_loop       ; 1 or 2 cycles
    nop                   ; 1 cycle
    ret                   ; 4 cycles
	

	
	