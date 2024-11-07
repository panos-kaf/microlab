;LAB 1 ASKISI 2
.include "m328PBdef.inc"

 .def temp = R16
 .def led = R17
 .def delay = R18


 ldi led, 0x01
 ser temp 
 out DDRD, temp
 ldi delay, 0xFF

left:
    out PORTD, led

    call Delay_Routine
left_loop:
    out PORTD, led
    call Delay_Routine
    call Delay_Routine
    lsl led
    cpi led, 0x80
    breq right
    jmp left_loop

right:
    out PORTD, led
    call Delay_Routine
right_loop:
    
    out PORTD,led
    call Delay_Routine
    call Delay_Routine
    lsr led
    cpi led, 0x01
    breq left
    jmp right_loop

Delay_Routine:
    ldi r27,0x17
    dec delay
    call Inner_Delay
    cpi delay,0x00
    brne Delay_Routine
    ret
    
Inner_Delay:
    ldi r28,0x2A
    dec r27
    call Inner_Delay_2
    cpi r27,0x00
    brne Inner_Delay
    ret
    
Inner_Delay_2:
    dec r28
    cpi r28,0x00
    brne Inner_Delay_2
    ret