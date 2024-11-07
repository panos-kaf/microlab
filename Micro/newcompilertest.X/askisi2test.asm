.include "m328PBdef.inc"

 .def temp = R16
 .def led = R17
 .def delay = R18

; delay = (1000F1+14) cycles (about DEL_mS in mSeconds)
//.equ FOSC_MHZ=16    ;MHz
//.equ DEL_mS=1        ;mS
//.equ F1=FOSC_MHZDEL_mS
//.equ delay_c = (1000*F1 + 14)
 
start:
 ldi led, 0x01
 clr temp 
 out DDRD, temp
 
out PORTD,led
 jmp start


