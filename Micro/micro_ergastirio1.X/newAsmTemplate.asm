


.include "m328PBdef.inc"
	
; delay = (1000*F1+14) cycles (abougt DEL_mS in mSeconds)

.equ FOSC_MHZ=16        ; MHz
.equ DEL_mS=10         ; mS
.equ F1=FOSC_MHZ*DEL_mS
    
; initialize stack pointer
    ldi r24, LOW(RAMEND)
    out SPL, r24
    ldi r24, HIGH(RAMEND)
    out SPH, r24
    
; init portd as output
    ser r26
    out DDRD, r26
    ldi r24, low(F1)          ; 
    ldi r25, high(F1)         ; Set delay

loop1:
    ser r26
    out PORTD, r26 
    rcall delay_outer      ; mS
    clr r26
    out PORTD, r26
    rcall delay_outer      ; mS
    rjmp loop1

;this routine is used to produce a delay 993 cycles
delay_inner:
    ldi r23, 247          ; 1 cycle
loop3:
    dec r23               ; 1 cycle
    nop                   ; 1 cycle
    brne loop3            ; 1 or 2 cycles
    nop                   ; 1 cycle
    ret                   ; 4 cycles

;this routine is used to produce a delay of (1000*Fl+14) cycles
delay_outer:
    push r24              ; (2 cycles)
    push r25              ; (2 cycles) Save r24:r25

loop4:
    rcall delay_inner     ; (3+993)=996 cycles
    sbiw r24,1            ; 2 cycles
    brne loop4            ; 1 or 2 cycles

    pop r25               ; (2 cycles)
    pop r24               ; (2 cycles) Restore r24:r25
    ret                   ; 4 cycles