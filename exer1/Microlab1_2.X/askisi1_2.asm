.include "m328PBdef.inc"
    
.def A = R16
.def B = R17
.def C = R18
.def D = R19
.def F0 = R20
.def F1 = R21
.def temp = R22
    
.org 0x0000

start:
	
	ldi temp,high(RAMEND)	;stack init
    out SPH,temp
    ldi temp,low(RAMEND)
    out SPL,temp
	
    ldi A,0x51				;initializations
	ldi B,0x41
	ldi C,0x21
	ldi D,0x01
	ldi r24,0x06
	
loop:
	subi r24,1
	call compute_F0
	call compute_F1
	subi A,-1
	subi B,-2
	subi C,-3
	subi D,-4
	cpi r24,0
	brne loop
	rjmp end

compute_F0:
	push A
	push B
	com B		; B'
	and A,B		; A = A AND B'
	and B,D		; B = B' AND D
	or A,B		; A = A OR B
	com A		; A'
	mov F0,A
	pop B
	pop A
	ret
	
compute_F1:
	push A
	push B
	com C		; C'
	com D		; D'
	or A,C		; A = A OR C'
	or B,D		; B = B OR D'
	and A,B		; A = A AND B
	mov F1,A
	com C		; Restore C
	com D		; Restore D
	pop B
	pop A
	ret
	
end: