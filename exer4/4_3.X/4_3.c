/*
 * File:   4_3.c
 * Author: panos_katsalifis
 *
 * Created on October 31, 2024, 10:51 AM
 */

// -8 57  121 186 251 315 380

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int flag = 0;
char message1[13] = "GAS DETECTED";
char message2[6] = "CLEAR";

void write_2_nibbles(unsigned char input){
    unsigned char pind = PIND&(0x0f);
    unsigned char temp = input;
    input&=0xf0;
    input+=pind;
    PORTD = input;
    PORTD |= 0b00001000;
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    PORTD &= 0b11110111;
    
    input=temp;
    input&=0x0f;
    input <<= 4;
    temp&=0xf0;
    temp >>= 4;
    input+=temp;
    
    input&=0xf0;
    input+=pind;
    PORTD = input;
    PORTD |= 0b00001000;
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    PORTD &= 0b11110111;
}

void lcd_data(unsigned char data){
    PORTD |= 0b00000100;
    write_2_nibbles(data);
    _delay_us(250);
}

void lcd_command(unsigned char command){
    PORTD &= 0b11111011;
    write_2_nibbles(command);
    _delay_us(250);
}

void lcd_clear_display(){
    lcd_command(1);
    _delay_ms(5);
}

void lcd_init(){
    _delay_ms(200);
    
    for(int i=0;i<3;i++){
        PORTD = 0x30;
        PORTD |= 0b00001000;
        __asm__ __volatile__("nop");
        __asm__ __volatile__("nop");
        PORTD &= 0b11110111;
        _delay_us(250);
    }
    PORTD = 0x20;
    PORTD |= 0b00001000;
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    PORTD &= 0b11110111;
    _delay_us(250);
    
    lcd_command(0x28);
    lcd_command(0x0c);
    lcd_clear_display();
    lcd_command(0x06);
}

void display(int value){
    if (value<146)
        PORTB = 0x00;
    else if (value<292)
        PORTB = 0b000001;
    else if(value<438)
        PORTB = 0b000011;
    else if(value<584)
        PORTB = 0b000111;
    else if(value<730)
        PORTB = 0b001111;
    else if(value<877)
        PORTB = 0b011111;
    else PORTB = 0b111111;
    _delay_ms(100);
}

void gas_detected(){
    
    lcd_clear_display();
    _delay_ms(1);
    
    for(int i = 0; i < 12; i++) { 
        lcd_data(message1[i]);
        _delay_us(10);
    }
    flag = 1;
     
}

void clear(){
    
    lcd_clear_display();
    _delay_ms(1);
    for(int i = 0; i < 5; i++) {
        lcd_data(message2[i]);
        _delay_us(10);
    }
    flag = 0;
}

int main(void) {

    DDRD = 0xff;
    DDRB = 0xff;
    DDRC = 0x00;
    
    ADMUX = 0b01000010;
    ADCSRA = 0b10000111;
    float value;
    
    lcd_init();
    _delay_ms(100);
        
    lcd_clear_display();
    _delay_ms(100);
    
    while (1) {
        ADCSRA|=(1<<ADSC);
        while(ADCSRA & (1<<ADSC));
       
        display(ADC);
        value = 5*ADC/1024;
        value = 10000*(value-0.1)/129;
        
        if (flag && value<=70) clear();
        if(value>70){
            if(!flag) gas_detected();            
            PORTB = 0;
            _delay_ms(200);
        }
        else _delay_ms(100);
    }
}
