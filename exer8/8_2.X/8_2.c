/*
 * File:   8_1.c
 * Author: panos_katsalifis
 *
 * Created on November 25, 2024, 2:31 PM
 */

#define F_CPU 16000000UL

#include <string.h>
#include <stdlib.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#define PCA9555_0_ADDRESS 0x40 
#define TWI_READ 1 
#define TWI_WRITE 0 
#define SCL_CLOCK 100000L 

//A0=A1=A2=0 by hardware
// reading from twi device
// writing to twi device
// twi clock in Hz
//Fscl=Fcpu/(16+2*TWBR0_VALUE*PRESCALER_VALUE)
#define TWBR0_VALUE ((F_CPU/SCL_CLOCK)-16)/2
// PCA9555 REGISTERS
typedef enum {
REG_INPUT_0 = 0,
REG_INPUT_1 = 1,
REG_OUTPUT_0 = 2,
REG_OUTPUT_1 = 3,
REG_POLARITY_INV_0 = 4,
REG_POLARITY_INV_1 = 5,
REG_CONFIGURATION_0 = 6,
REG_CONFIGURATION_1 = 7
} PCA9555_REGISTERS;
//----------- Master Transmitter/Receiver -------------------
#define TW_START 0x08
#define TW_REP_START 0x10
//---------------- Master Transmitter ----------------------
#define TW_MT_SLA_ACK 0x18
#define TW_MT_SLA_NACK 0x20
#define TW_MT_DATA_ACK 0x28
//---------------- Master Receiver ----------------
#define TW_MR_SLA_ACK 0x40
#define TW_MR_SLA_NACK 0x48
#define TW_MR_DATA_NACK 0x58

#define TW_STATUS_MASK 0b11111000
#define TW_STATUS (TWSR0 & TW_STATUS_MASK)
//initialize TWI clock
void twi_init(void){
    TWSR0 = 0; // PRESCALER_VALUE=1
    TWBR0 = TWBR0_VALUE; // SCL_CLOCK 100KHz
}
// Read one byte from the twi device (request more data from device)
unsigned char twi_readAck(void){
    TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    while(!(TWCR0 & (1<<TWINT)));
    return TWDR0;
}
//Read one byte from the twi device, read is followed by a stop condition
unsigned char twi_readNak(void){
    TWCR0 = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR0 & (1<<TWINT)));
    return TWDR0;
}
// Issues a start condition and sends address and transfer direction.
// return 0 = device accessible, 1= failed to access device
unsigned char twi_start(unsigned char address){
    uint8_t twi_status;
    // send START condition
    TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    // wait until transmission completed
    while(!(TWCR0 & (1<<TWINT)));
    // check value of TWI Status Register.
    twi_status = TW_STATUS & 0xF8;
    if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) return 1;
    // send device address
    TWDR0 = address;
    TWCR0 = (1<<TWINT) | (1<<TWEN);
    // wail until transmission completed and ACK/NACK has been received
    while(!(TWCR0 & (1<<TWINT)));
    // check value of TWI Status Register.
    twi_status = TW_STATUS & 0xF8;
    if ((twi_status != TW_MT_SLA_ACK) && (twi_status != TW_MR_SLA_ACK))return 1;
    return 0;
}

// Send start condition, address, transfer direction.
// Use ack polling to wait until device is ready
void twi_start_wait(unsigned char address)
{
    uint8_t twi_status;
    while (1){
        // send START condition
        TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
        // wait until transmission completed
        while(!(TWCR0 & (1<<TWINT)));
        // check value of TWI Status Register.
        twi_status = TW_STATUS & 0xF8;
        if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) continue;
        // send device address
        TWDR0 = address;
        TWCR0 = (1<<TWINT) | (1<<TWEN);
        // wail until transmission completed
        while(!(TWCR0 & (1<<TWINT)));
        // check value of TWI Status Register.
        twi_status = TW_STATUS & 0xF8;
        if ( (twi_status == TW_MT_SLA_NACK )||(twi_status ==TW_MR_DATA_NACK)){
            /* device busy, send stop condition to terminate write operation */
            TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
            // wait until stop condition is executed and bus released
            while(TWCR0 & (1<<TWSTO));
            continue;
        }
        break;
    }
}

// Send one byte to twi device, Return 0 if write successful or 1 if write failed
unsigned char twi_write( unsigned char data ){
    // send data to the previously addressed device
    TWDR0 = data;
    TWCR0 = (1<<TWINT) | (1<<TWEN);
    // wait until transmission completed
    while(!(TWCR0 & (1<<TWINT)));
    if( (TW_STATUS & 0xF8) != TW_MT_DATA_ACK) return 1;
    return 0;
}
// Send repeated start condition, address, transfer direction
//Return: 0 device accessible
// 1 failed to access device
unsigned char twi_rep_start(unsigned char address)
{
return twi_start( address );
}
// Terminates the data transfer and releases the twi bus
void twi_stop(void)
{
    // send stop condition
    TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    // wait until stop condition is executed and bus released
    while(TWCR0 & (1<<TWSTO));
}
void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value)
{
twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
twi_write(reg);
twi_write(value);
twi_stop();
}
uint8_t PCA9555_0_read(PCA9555_REGISTERS reg){
    uint8_t ret_val;
    twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
    twi_write(reg);
    twi_rep_start(PCA9555_0_ADDRESS + TWI_READ);
    ret_val = twi_readNak();
    twi_stop();
    return ret_val;
}

void pca_write_2_nibbles(uint8_t input){
    
    unsigned char exp = PCA9555_0_read(REG_INPUT_0)&0x0f;
    unsigned char temp = input;
    temp&=0xf0;
    temp+=exp;
    PCA9555_0_write(REG_OUTPUT_0,temp);
    PCA9555_0_write(REG_OUTPUT_0,PCA9555_0_read(REG_INPUT_0)|(0b00001000));
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    PCA9555_0_write(REG_OUTPUT_0,PCA9555_0_read(REG_INPUT_0)&(0b11110111));
    
    temp=(input<<4)|(input>>4);
    
    exp = PCA9555_0_read(REG_INPUT_0)&0x0f;
    temp&=0xf0;
    temp+=exp;
    
    PCA9555_0_write(REG_OUTPUT_0,temp);
    PCA9555_0_write(REG_OUTPUT_0,PCA9555_0_read(REG_INPUT_0)|(0b00001000));
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    PCA9555_0_write(REG_OUTPUT_0,PCA9555_0_read(REG_INPUT_0)&(0b11110111));
     
}

void pca_lcd_data(unsigned char data){
    PCA9555_0_write(REG_OUTPUT_0,PCA9555_0_read(REG_INPUT_0)|0b00000100);
    pca_write_2_nibbles(data);
    _delay_us(250);
}

void pca_lcd_command(unsigned char command){
    PCA9555_0_write(REG_OUTPUT_0,PCA9555_0_read(REG_INPUT_0)&0b11111011);
    pca_write_2_nibbles(command);
    _delay_us(250);
}

void pca_lcd_clear_display(){
    pca_lcd_command(1);
    _delay_ms(5);
}

void pca_lcd_init(){
    _delay_ms(200);
    for(int i=0;i<3;i++){
        PCA9555_0_write(REG_OUTPUT_0,0x30);
        PCA9555_0_write(REG_OUTPUT_0,PCA9555_0_read(REG_INPUT_0)|0b00001000);
        __asm__ __volatile__("nop");
        __asm__ __volatile__("nop");
        PCA9555_0_write(REG_OUTPUT_0,PCA9555_0_read(REG_INPUT_0)&0b11110111);
        _delay_us(250);
    }
    PCA9555_0_write(REG_OUTPUT_0,0x20);   
    PCA9555_0_write(REG_OUTPUT_0,PCA9555_0_read(REG_INPUT_0)|0b00001000);
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    
    PCA9555_0_write(REG_OUTPUT_0,PCA9555_0_read(REG_INPUT_0)&0b11110111);
    _delay_us(250);
    
    pca_lcd_command(0x28);
    pca_lcd_command(0x0c);
    pca_lcd_clear_display();
    pca_lcd_command(0x06);
}

// ??????????

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
    //uint8_t bit = PIND&(1<<PD4);
    DDRD |= (1<<PD4);
    PORTD &= ~(1<<PD4);
    _delay_us(2);
    /*if (bit&0x01) PORTD |= (1<<PD4);          // ????
    if (bit&0x01) PORTD &= ~(1<<PD4);*/
    PORTD|=(bit<<PD4); //?
    
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

uint8_t sign;
char d[6];

void temp_digits(uint16_t value){
    //value = ~value+1;
    sign = value&(0xf800);
    value=(value&0x07f0)>>4;
    uint8_t decimal = value&(0x0f);
    
    int result = (int)value;
    
    double dval = decimal*62.5;
    
    d[4] = (int)dval/100;
    d[5] = (int)dval/10;
    d[5] %= 10;
    d[6] = (int)dval%10;
    
    d[1] = result/100;
    d[2] = result/10;
    d[2] %= 10;
    d[3] = result%10;
}

void output_temp(){
    if(sign==0)
        pca_lcd_data('+');
    else pca_lcd_data('-');
    
    d[1]+=48;
    d[2]+=48;
    d[3]+=48;  
    d[4]+=48;
    d[5]+=48;
    d[6]+=48;
    
    if(d[1]!=48)
        pca_lcd_data(d[1]);
    if(d[1]!=48 || d[2]!=48)
        pca_lcd_data(d[2]);
    pca_lcd_data(d[3]);
    pca_lcd_data('.');
    pca_lcd_data(d[4]);
    if(d[5]!=48 || d[6]!=48)
        pca_lcd_data(d[5]);
    if(d[6]!=48)
        pca_lcd_data(d[6]);
    
    pca_lcd_data(0xDF);
    pca_lcd_data('C');
}

//


void usart_init(unsigned int ubrr){
    UCSR0A=0;
    UCSR0B=(1<<RXEN0)|(1<<TXEN0);
    UBRR0H=(unsigned char)(ubrr>>8);
    UBRR0L=(unsigned char)ubrr;
    UCSR0C=(3 << UCSZ00);
    return;
}

void usart_transmit(uint8_t data){
    while(!(UCSR0A&(1<<UDRE0)));
    UDR0=data;
}

uint8_t usart_receive(){
while(!(UCSR0A&(1<<RXC0)));
return UDR0;
}

void transmit_word(char* word){
    int i = 0;
    while(word[i]!=0){
        usart_transmit(word[i++]);
    }
}

void transmit_esp(char* word){
    transmit_word("ESP:");
    transmit_word(word);
    transmit_word('\n');
}

void receive_word(char* word){
    int i = 0;
    while(1){
        char letter = usart_receive();
        if (letter=='\n') break;
        word[i++]=letter;
    }
}

void print_lcd(char* str){
    int i = 0;
    while(str[i]!=0)
        pca_lcd_data(str[i++]);
}



#include <stdio.h>

/*
void create_payload(char *buffer, size_t buffer_size, uint8_t* temp_digits, float pressure, int team, const char *status) {
    snprintf(buffer, buffer_size,
             "payload: [{\"name\": \"temperature\",\"value\": \"%.1f\"},"
             "{\"name\": \"pressure\",\"value\": \"%.1f\"},"
             "{\"name\": \"team\",\"value\": \"%d\"},"
             "{\"name\": \"status\",\"value\": \"%s\"}]",
             temp_digits, pressure, team, status);
}
*/
//transmit_word(payload);

// Pressure

char pressure[3];

void pressure_digits(int value){
    int result = (int)value;
    pressure[0] = result/100;
    pressure[1] = result/10;
    pressure[1] %= 10;
    pressure[2] = result%10;
}

void pressure_output(){
    pressure[0]+=48;
    pressure[1]+=48;
    pressure[2]+=48;
    pca_lcd_data(pressure[0]);
    pca_lcd_data(pressure[1]);
    pca_lcd_data(0b00101110);
    pca_lcd_data(pressure[2]);
}

//


// Keypad

uint16_t pressed_keys=0;

uint8_t scan_row(uint8_t row){
    
    uint8_t temp = ~(0x01 << row);
    PCA9555_0_write(REG_OUTPUT_1,temp);
    return (PCA9555_0_read(REG_INPUT_1)&0xf0)>>4;
}

uint16_t scan_keypad(){
    uint16_t res=0;
    for(int i=0;i<4;i++)
        res+=scan_row(i)<<4*i;   
    return res;
}

uint16_t scan_keypad_rising_edge() {
    pressed_keys = 0;
    uint16_t pressed_keys_tempo = scan_keypad(); // Scan the keypad
    _delay_ms(20);                               // Debounce delay
    pressed_keys_tempo = scan_keypad();          // Scan again after delay

    // Rising edge detection: keys that are pressed now but weren't in the last scan
    uint16_t rising_edge_keys = (pressed_keys_tempo) & ~(pressed_keys);

    // Update last_pressed_keys for the next call
    pressed_keys = pressed_keys_tempo;

    return rising_edge_keys; // Return only the keys that were newly pressed
}

/*
        1101
        0111
        1110
        0111
 
row1= pressed_keys&0f<<4;
row1+= 0b1110;
// 1110 0111
if row1 == 1110 0111 then ascii = 'D';

 */

uint8_t keypad_to_ascii(){
    uint8_t row0,row1,row2,row3;
    
    row0 = (pressed_keys&0x000f)<<4;
    row0 += 0b1110;
    
    row1 = (pressed_keys&0x00f0);
    row1 += 0b1101;
    
    row2 = (pressed_keys&0x0f00)>>4;
    row2 += 0b1011;
    
    row3 = (pressed_keys&0xf000)>>8;
    row3 += 0b0111;
        
    switch (row0){
        case 0b11101110: 
            return '*';
        case 0b11011110:
            return '0';
        case 0b10111110:
            return '#';
        case 0b01111110:
            return 'D';
    }
        switch (row1){
        case 0b11101101: 
            return '7';
        case 0b11011101:
            return '8';
        case 0b10111101:
            return '9';
        case 0b01111101:
            return 'C';
    }
    
        switch (row2){
        case 0b11101011: 
            return '4';
        case 0b11011011:
            return '5';
        case 0b10111011:
            return '6';
        case 0b01111011:
            return 'B';
    }
    
        switch (row3){
        case 0b11100111: 
            return '1';
        case 0b11010111:
            return '2';
        case 0b10110111:
            return '3';
        case 0b01110111:
            return 'A';       
    }
    return 0;
}

//

void write_word(char* str){
    int i = 0;
    while(str[i]!=0)
        pca_lcd_data(str[i++]);
}

int main(void) {
    
    TCCR1A |= (1<<WGM10)|(1<<COM1A1);
    TCCR1B |= (1<<WGM12)|(1<<CS12);     //prescaler = 256
        
    DDRD=0xFF; 
    DDRC=0x00;
    DDRB=0x03;
 
    ADMUX = 0b01000000;
    ADCSRA = 0b10000111;    
    
    char answer[20];
    char* status="OK";
    uint8_t key;
    
    twi_init();\
    pca_lcd_init();
    PCA9555_0_write(REG_CONFIGURATION_1,0b11110000);  
    PCA9555_0_write(REG_OUTPUT_1,0x00);
    usart_init(103);
    transmit_esp("restart");

    while(1){
        
        ADCSRA|=(1<<ADSC);
        while(ADCSRA & (1<<ADSC));
        int pres = ADC/5.11;
        int16_t temp = read_temp()+16*12;
        temp_digits(temp);
        pressure_digits(pres);
        
        pres/=10;
        temp&=0x07FF;
        temp/=16;
        
        
        if (pres>=12 || pres<4){
            status="CHECK PRESSURE";
            if (temp<34 || temp>=37)
                status="TEMP & PRESSURE";
        }
        else if ((temp<34 || temp>=37))
            status="CHECK TEMP";         
       
        pressed_keys = scan_keypad_rising_edge();
        key = keypad_to_ascii();
        if(key=='#' && strcmp("NURSE CALL",status)==0)
            status="OK";
        
        pressed_keys = scan_keypad_rising_edge();
        key = keypad_to_ascii();
        if(key=='4')
            status="NURSE CALL";
        
        
        write_word(status);
        pca_lcd_command(0b11000000);
        output_temp();
        write_word(" | ");
        pressure_output();
        _delay_ms(500);
        pca_lcd_clear_display();
        
        char* payload[9];
        char t[5],p[5];
        
        t[0] = d[2];
        t[1] = d[3];
        t[2] = '.';
        t[3] = d[4];
        p[0] = pressure[0];
        p[1] = pressure[1];
        p[2] = '.';
        p[3] = pressure[2];
           
        payload[0] = "ESP:payload:[{\"name\": \"temperature\",\"value\": \"";
        payload[1] = t;
        payload[2] = "\"},{\"name\": \"pressure\",\"value\": \"";
        payload[3] = p;
        payload[4] = "\"},{\"name\": \"team\",\"value\": \"24\"}"
                     ",{\"name\": \"status\",\"value\": \"";
        payload[5] = status;
        payload[6] = "\"}]\n";
        
        for(int i=0;i<7;i++)
            transmit_word(payload[i]);
    
    }
}