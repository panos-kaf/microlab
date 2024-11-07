/*
 * File:   4_2.c
 * Author: panos_katsalifis
 *
 * Created on October 29, 2024, 4:47 PM
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

unsigned char d1,d2,d3;

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

void digits(int value){
    int result = (int)value;
    d1 = result/100;
    d2 = result/10;
    d2 %= 10;
    d3 = result%10;
}

void output(){
    d1+=48;
    d2+=48;
    d3+=48;
    
    lcd_command(0b10000000);
    _delay_ms(10);
    lcd_data(d1);
    
    lcd_command(0b10000001);
    _delay_ms(10);
    lcd_data(0b00101110);
    
    lcd_command(0b10000010);
    _delay_ms(10);
    lcd_data(d2);
    
    lcd_command(0b10000011);
    _delay_ms(10);
    lcd_data(d3);
}

int main(void) {

    ADMUX = 0b010000001;
    ADCSRA = 0b10001111;
    DDRD = 0xff;
    DDRC = 0x00;
    double value;

    lcd_init();
    _delay_ms(100);

    while (1) {
        ADCSRA|=(1<<ADSC);
        while(ADCSRA & (1<<ADSC));
        
        value = (double)500*ADC/1024;
        digits((int)value);
        output();
        _delay_ms(1000);   
    }
}