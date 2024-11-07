#define F_CPU 16000000UL
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include <avr/io.h>

volatile uint16_t count=0;
volatile uint8_t flag=0,flag2=0;

ISR(INT1_vect){
    _delay_ms(100);
    
    if(PORTB==0x01){ 
        flag=1;
    }
    count=0;
    PORTB=0x01;
}
    
int main(void) {
    
    EICRA=(1<<ISC11) | (1<<ISC10);
    EIMSK=(1<<INT1);
    sei();
    DDRB=0xFF;
    DDRD=0x00;
    PORTB=0x00;
    
    while (1) {
        for(count; count<3000;count++){
            _delay_ms(1);       
            if(flag){
                PORTB=0xFF;
                _delay_ms(1000);
                PORTB=0x01;
                count=1000;
                flag=0;
                flag2=1;
            }
            flag2=0;
        }
        
    PORTB=0x00;
    
    }
}