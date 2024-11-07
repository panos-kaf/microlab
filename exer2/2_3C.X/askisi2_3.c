/*
 * File:   newavr-main.c
 * Author: panos_katsalifis
 *
 * Created on October 17, 2024, 10:51 AM
 */

#define F_CPU 16000000UL
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include <avr/io.h>

ISR(INT1_vect){
    sei();
    if(PORTB){
        PORTB = 0xFF;
        _delay_ms(500);
    }
    PORTB = 0x01;
    _delay_ms(5000);
    PORTB = 0x00;
    cli();
}

int main(void) {
    EICRA = (1<<ISC11)|(0<<ISC10);
    EIMSK = (1<<INT1);
    sei();
    DDRB = 0xFF;
    PORTB = 0x00;
    while (1);
}
