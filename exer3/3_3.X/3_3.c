/*
 * File:   3_3.c
 * Author: panos_katsalifis
 *
 * Created on October 23, 2024, 1:14 PM
 */
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define pb2 !(PINB&(0x04))
#define pb3 !(PINB&(0x08))

#define pd6 !(PIND&(0x40))
#define pd7 !(PIND&(0x80))

const char value[13]={5,26,46,67,87,108,128,148,169,189,210,230,251};

volatile unsigned char level = 6;

void mode1(){
    
    while(1){
       
        if (pd7) break;
        
        if (pb2){
            while(pb2) _delay_ms(10);
            if(level!=12) OCR1AL = value[++level];
        }
        
        if (pb3){
            while(pb3) _delay_ms(10);
            if(level!=0) OCR1AL = value[--level];
        }
        _delay_ms(100);
    }
}

void mode2(){
    while(1){
        if (pd6) break;
        ADCSRA|=(1<<ADSC);
        while(ADCSRA & (1<<ADSC));
        OCR1AL = ADC/4;
        _delay_ms(10);
    }
}

int main(void) {

    TCCR1A |= (1<<WGM10)|(1<<COM1A1);
    TCCR1B |= (1<<WGM12)|(1<<CS10);     //prescaler = 256
    
    OCR1AL = value[level];
    
    DDRD=0x00; 
    DDRC=0x00;
    DDRB=0x03;
    char button6,button7;
    
    ADMUX = 0b01000000;
    
    ADCSRA = 0b10000111;
    
    while(1){
        if(pd6)
            mode1();
        if(pd7)
            mode2();
        _delay_ms(100);
    }
}