.include  "m328PBdef.inc"

.org 0x00
	rjmp reset
.org 0x2A
	rjmp isr
	
.equ PD0=0
.equ PD1=1
.equ PD2=2
.equ PD3=3
.equ PD4=4
.equ PD5=5
.equ PD6=6
.equ PD7=7

.def temp = r16
.UNDEF ADC_L
.UNDEF ADC_H
.def ADC_L = r26
.def ADC_H = r27

reset:
	ldi temp,high(RAMEND)
	out SPH,temp
	ldi temp,low(RAMEND)
	out SPL,temp
		
	ldi temp,0b01000001		; POT2
	sts ADMUX,temp
	ldi temp,0b10001111		; Enable interrupts
	sts ADCSRA,temp
	
	sei
	
	ser r24
	out DDRD, r24 
	clr r24
	out DDRC, r24
	
	clr r24
	rcall lcd_init
	
	ldi r24, low(100)
	ldi r25, high(100) 
	rcall wait_msec
	

main:
	lds temp, ADCSRA 
	ori temp, (1<<ADSC) 
	sts ADCSRA, temp
	
	rcall compute
	rcall bin_to_dec
	rcall output
	
	ldi r24, low(100)
	ldi r25, 0
	rcall wait_msec
	
	rjmp main
	
	; multiply * 25
	; 4 right shifts
	; multiply * 5
	; 4 right shifts
	
		
isr:
	lds ADC_L,ADCL
	lds ADC_H,ADCH
	reti
	
; Assume 9-bit binary number is in r16:r17 (r16 = low byte, r17 = high byte)
; Output: hundreds in r18, tens in r19, units in r20

compute:
	mov r28,ADC_L
	mov r29,ADC_H
	
	lsl r28
	rol r29
	lsl r28
	rol r29
	lsl r28
	rol r29
	
	mov r30,r28
	mov r31,r29
	
	lsl r28
	rol r29
	
	add r30,r28
	adc r31,r29
	add r30,ADC_L
	adc r31,ADC_H		;r30-31 = ADC*25
	
	lsr r31
	ror r30
	lsr r31
	ror r30
	lsr r31
	ror r30
	lsr r31
	ror r30				;4 right shifts
	
	mov r28,r30
	mov r29,r31
	
	lsl r30
	rol r31
	lsl r30
	rol r31
	
	add r30,r28
	adc r31,r29			;r30-31 = r30-31 * 5
	
	lsr r31
	ror r30
	lsr r31
	ror r30
	lsr r31
	ror r30
	lsr r31
	ror r30				;4 right shifts
	
	mov ADC_L,r30
	mov ADC_H,r31
	
	ret
	
output:
	ldi temp,48
	add r18,temp
	add r19,temp
	add r20,temp
	
	
	ldi r24,0b10000000
	rcall lcd_command
	
	ldi r24,10
	clr r25
	rcall wait_msec
	
	mov r24,r18
	rcall lcd_data
	
	ldi r24,10
	clr r25
	rcall wait_msec
	
	ldi r24,0b10000001
	rcall lcd_command
	
	ldi r24,10
	clr r25
	rcall wait_msec
	
	ldi r24,0b00101110
	rcall lcd_data
	
	ldi r24,10
	clr r25
	rcall wait_msec
	
	ldi r24,0b10000010
	rcall lcd_command
	
	ldi r24,10
	clr r25
	rcall wait_msec
	
	mov r24,r19
	rcall lcd_data
	
	ldi r24,0b10000011
	rcall lcd_command
	
	ldi r24,10
	clr r25
	rcall wait_msec
	
	mov r24,r20
	rcall lcd_data

	ret
	
bin_to_dec:
    clr r18              ; Clear hundreds place
    clr r19              ; Clear tens place
    clr r20              ; Clear units place

    ldi r21, 100         ; Load 100 into r21 for hundreds place calculation
	
; ---- Hundreds Calculation ----
hundreds_loop:
    ldi r22, 0           ; Temporary register to store carry from r17
    cp ADC_H, r22
	brne skip
	cp ADC_L, r21          ; Compare high byte with 100
    brlo tens_loop       ; If less than 100, go to tens calculation
skip:
	sub ADC_L, r21         ; Subtract 100 from high byte
    sbc ADC_H, r22         ; Subtract 0 from low byte (consider carry)
    inc r18              ; Increment hundreds place
    rjmp hundreds_loop   ; Repeat until r17:r16 < 100

; ---- Tens Calculation ----
tens_loop:
    ldi r21, 10          ; Load 10 into r21 for tens place calculation
tens_subtract:
    cp ADC_L, r21          ; Compare high byte with 10
    brlo units_place     ; If less than 10, go to units calculation
    sub ADC_L, r21         ; Subtract 10 from high byte
    sbc ADC_H, r22         ; Subtract 0 from low byte (consider carry)
    inc r19              ; Increment tens place
    rjmp tens_subtract   ; Repeat until r17:r16 < 10

; ---- Units Calculation ----
units_place:
    mov r20, ADC_L         ; The remaining value in r16 is the units place

    ret                  ; Return with r18 = hundreds, r19 = tens, r20 = units	

	
write_2_nibbles:
	push r24 ; save r24(LCD_Data)
	in r25 ,PIND ; read PIND
	andi r25 ,0x0f ;
	andi r24 ,0xf0
	add r24 ,r25 
	out PORTD ,r24 ;
	sbi PORTD ,PD3 
	nop
	nop
	cbi PORTD ,PD3
	pop r24 ; Recover r24(LCD_Data)
	swap r24 ;
	andi r24 ,0xf0
	add r24 ,r25 ; r24[3:0] Holds previus PORTD[3:0]
			; r24[7:4] <-- LCD_Data_Low_Byte
	out PORTD ,r24
	sbi PORTD ,PD3 
	nop
	nop
	cbi PORTD ,PD3
	ret
	
lcd_data:
	sbi PORTD ,PD2 ; LCD_RS=1(PD2=1), Data
	rcall write_2_nibbles ; send data
	ldi r24 ,250 ;
	ldi r25 ,0 ; Wait 250uSec
	rcall wait_usec
	ret
	
lcd_command:
	cbi PORTD ,PD2 ; LCD_RS=0(PD2=0), Instruction
	rcall write_2_nibbles ; send Instruction
	ldi r24 ,250 ;
	ldi r25 ,0 ; Wait 250uSec
	rcall wait_usec
	ret
	
lcd_clear_display:
	ldi r24 ,0x01 
	rcall lcd_command ; clear display command
	ldi r24 ,low(5) ;
	ldi r25 ,high(5) 
	rcall wait_msec ;
	ret

lcd_init:
    ldi r24, low(200)             ; 
    ldi r25, high(200)            ; Wait 200 mSec
    rcall wait_msec               ; 

    ldi r24, 0x30                 ; command to switch to 8 bit mode
    out PORTD, r24                ; 
    sbi PORTD, PD3                ; Enable Pulse
    nop
    nop
    cbi PORTD, PD3
    ldi r24, 250                  ; 
    ldi r25, 0                    ; Wait 250 uSec
    rcall wait_usec               ; 

    ldi r24, 0x30                 ; command to switch to 8 bit mode
    out PORTD, r24                ; 
    sbi PORTD, PD3                ; Enable Pulse
    nop
    nop
    cbi PORTD, PD3
    ldi r24, 250                  ; 
    ldi r25, 0                    ; Wait 250 uSec
    rcall wait_usec               ; 

    ldi r24, 0x30                 ; command to switch to 8 bit mode
    out PORTD, r24                ; 
    sbi PORTD, PD3                ; Enable Pulse
    nop
    nop
    cbi PORTD, PD3
    ldi r24, 250                  ; 
    ldi r25, 0                    ; Wait 250 uSec
    rcall wait_usec               ; 

    ldi r24, 0x20                 ; command to switch to 4 bit mode
    out PORTD, r24                ; 
    sbi PORTD, PD3                ; Enable Pulse
    nop
    nop
    cbi PORTD, PD3
    ldi r24, 250                  ; 
    ldi r25, 0                    ; Wait 250 uSec
    rcall wait_usec               ; 

    ldi r24, 0x28                 ; 5x8 dots, 2 lines
    rcall lcd_command             ; 

    ldi r24, 0x0c                 ; display on, cursor off
    rcall lcd_command             ; 
	rcall lcd_clear_display
	ldi r24 ,0x06 
	rcall lcd_command
	ret
	
wait_msec:
	push r24 ; 2 cycles
	push r25 ; 2 cycles
	ldi r24 , low(999) ; 1 cycle
	ldi r25 , high(999) ; 1 cycle
	rcall wait_usec ; 998.375 usec
	pop r25 ; 2 cycles
	pop r24 ; 2 cycles
	nop ; 1 cycle
	nop ; 1 cycle
	sbiw r24 , 1 
	brne wait_msec ; 2 cycles
	; 1 or 2 cycles
	ret ; 4 cycles
wait_usec:
	sbiw r24 ,1 ; 2 cycles (2/16 usec)
	call delay_8cycles ; 4+8=12 cycles
	brne wait_usec ; 1 or 2 cycles
	ret
	
delay_8cycles:
	nop
	nop
	nop
	nop
	ret