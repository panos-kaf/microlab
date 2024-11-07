/*
 * File:   3_2.c
 * Author: panos_katsalifis
 *
 * Created on October 23, 2024, 1:14 PM
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define pb2 !(PINB&(0x04))
#define pb3 !(PINB&(0x08))

const char value[13]={5,26,46,67,87,108,128,148,169,189,210,230,251};

void output(int average){
         
        if(average<=200)
            PORTD=0x01;
        else if(average<=400)
            PORTD=0x02;
        else if(average<=600)
            PORTD=0x04;
        else if(average<=800)
            PORTD=0x08;
        else PORTD=0x10;        
}
int main(void) {

    TCCR1A |= (1<<WGM10)|(1<<COM1A1);
    TCCR1B |= (1<<WGM12)|(1<<CS12);     //prescaler = 256
    
    unsigned char level = 6;
    unsigned int ADCvalues[16] = {0};
    int index = 0,average = 0;
    OCR1AL = value[level];
    
    DDRD=0xFF; 
    DDRC=0x00;
    DDRB=0x03;
    
    ADMUX = 0b01000001;
    
    ADCSRA = 0b10000111;
    
    while(1) {
        ADCSRA|=(1<<ADSC);
        while(ADCSRA & (1<<ADSC));
        ADCvalues[index++]=ADC;
   
        if (pb2){
            while(pb2) _delay_ms(10);
            if(level!=12) OCR1AL = value[++level];
        }
        
        if (pb3){
            while(pb3) _delay_ms(10);
            if(level!=0) OCR1AL = value[--level];
        }
      
        if(index==15){
            for (int i=0;i<16;i++)
                average+=ADCvalues[i];
            output(average>>4);
            index=0;
            average=0;
        }
        _delay_ms(10);
       
    }
}
