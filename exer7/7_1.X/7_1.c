/*
 * File:   7_1.c
 * Author: panos_katsalifis
 *
 * Created on November 12, 2024, 1:50 PM
 */

#define F_CPU 16000000UL

#include<avr/io.h>
#include<util/delay.h>

uint8_t one_wire_reset(){
    uint8_t temp;
    DDRD |= (1<<PD4);
    PORTD &= ~(1<<PD4);
    _delay_us(480);
    DDRD &= ~(1<<PD4);
    PORTD &= ~(1<<PD4);
    _delay_us(100);
    temp = PIND;
    _delay_us(380);
    if(temp&(1<<PD4)) return 0;
    return 1;
}

uint8_t one_wire_receive_bit(){
    uint8_t bit=0;
    DDRD |= (1<<PD4);
    PORTD &= ~(1<<PD4);
    _delay_us(2);
    DDRD &= ~(1<<PD4);
    PORTD &= ~(1<<PD4);
    _delay_us(10);
    if(PIND&(1<<PD4)) bit=1;
    _delay_us(49);
    return bit;
}

void one_wire_transmit_bit(uint8_t bit){
    DDRD |= (1<<PD4);
    PORTD &= ~(1<<PD4);
    _delay_us(2);

    PORTD|=(bit<<PD4);
    
    _delay_us(58);
    DDRD &= ~(1<<PD4);
    PORTD &= ~(1<<PD4);
    _delay_us(1);  
}

uint8_t one_wire_receive_byte(){
    uint8_t byte=0;
    for (uint8_t i =0;i<8;i++){
        byte += one_wire_receive_bit()<<i;
    }
    return byte;
}   

void one_wire_transmit_byte(uint8_t byte){
    for(uint8_t i=0;i<8;i++){
        uint8_t bit = (byte>>i)&0x01;
        one_wire_transmit_bit(bit);
    }
}

int16_t read_temp(){
    int16_t value;
    if(one_wire_reset()==0) return 0x8000;
    one_wire_transmit_byte(0xCC);
    one_wire_transmit_byte(0x44);
    
    while(one_wire_receive_bit()==0);
    
    if(one_wire_reset()==0) return 0x8000;
    one_wire_transmit_byte(0xCC);
    one_wire_transmit_byte(0xBE);
    value = one_wire_receive_byte();
    value += one_wire_receive_byte()<<8;
    return value;
}

int main(){}
