/*
 * File:   8_1.c
 * Author: panos_katsalifis
 *
 * Created on November 25, 2024, 2:31 PM
 */

#define F_CPU 16000000UL

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define PCA9555_0_ADDRESS 0x40 
#define TWI_READ 1 
#define TWI_WRITE 0 
#define SCL_CLOCK 100000L 


// Global Variables
uint16_t pressed_keys=0;
uint8_t temp_sign;
char temperature[7],pressure[4];
//

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


// LCD

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

void print_lcd(char* str){
    int i = 0;
    while(str[i]!=0)
        pca_lcd_data(str[i++]);
}


// Keypad


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


// One wire - temperature

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

void temp_digits(uint16_t value){
    //value = ~value+1;
    temp_sign = value&(0xf800);
    value=(value&0x07f0)>>4;
    uint8_t decimal = value&(0x0f);
    
    int result = (int)value;
    
    double dval = decimal*62.5;
    
    temperature[4] = (int)dval/100;
    temperature[5] = (int)dval/10;
    temperature[5] %= 10;
    temperature[6] = (int)dval%10;
    
    temperature[1] = result/100;
    temperature[2] = result/10;
    temperature[2] %= 10;
    temperature[3] = result%10;
}

void output_temp(){
    if(temp_sign==0)
        pca_lcd_data('+');
    else pca_lcd_data('-');
    
    temperature[1]+=48;
    temperature[2]+=48;
    temperature[3]+=48;  
    temperature[4]+=48;
    temperature[5]+=48;
    temperature[6]+=48;
    
    if(temperature[1]!=48)
        pca_lcd_data(temperature[1]);
    if(temperature[1]!=48 || temperature[2]!=48)
        pca_lcd_data(temperature[2]);
    pca_lcd_data(temperature[3]);
    pca_lcd_data('.');
    pca_lcd_data(temperature[4]);
    if(temperature[5]!=48 || temperature[6]!=48)
        pca_lcd_data(temperature[5]);
    if(temperature[6]!=48)
        pca_lcd_data(temperature[6]);
    
    pca_lcd_data(0xDF);
    pca_lcd_data('C');
}


//  UART

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

void receive_word(char* buffer,size_t size) {
    int i = 0;
    //int size = sizeof(buffer);

    while (1) {
        char letter = usart_receive();
        if (i < size - 1) {
            buffer[i++] = letter;  // Add character to buffer
        }
        if (letter == '\n') {
            break;
        }
    }
    buffer[i] = '\0';  // Null-terminate the string
}

// Pressure

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
    if(pressure[0]!=48)
        pca_lcd_data(pressure[0]);
    pca_lcd_data(pressure[1]);
    pca_lcd_data(0b00101110);
    pca_lcd_data(pressure[2]);
}


//

void create_payload(char *buffer, size_t buffer_size, int team, const char *status) {
    snprintf(buffer, buffer_size,
             " ESP:payload:[{\"name\": \"temperature\",\"value\": \"%c%c.%c\"},"
             "{\"name\": \"pressure\",\"value\": \"%c%c.%c\"},"
             "{\"name\": \"team\",\"value\": \"%d\"},"
             "{\"name\": \"status\",\"value\": \"%s\"}]\n",
             temperature[2],temperature[3],temperature[4], 
            pressure[0],pressure[1],pressure[2], team, status);
}

void check(char* answer,int num){
    char message[256];
    if (strcmp(answer,"\"Success\"\n")==0)
        snprintf(message,sizeof(message),"%d.Success",num);
    else snprintf(message,sizeof(message),"%d.Fail",num);
    print_lcd(message);
}

void check_transmit(char* answer,int num){
    char message[256];
    snprintf(message,sizeof(message),"%d.%s",num,answer);
    print_lcd(message);
}

int main(void) {
    
    TCCR1A |= (1<<WGM10)|(1<<COM1A1);
    TCCR1B |= (1<<WGM12)|(1<<CS12);     //prescaler = 256
        
    DDRC=0x00;
    
    ADMUX = 0b01000000;
    ADCSRA = 0b10000111;    
    
    char answer[256],payload[256];
    char *status="OK",*prev_status="0";
    uint8_t key;
    int16_t temp;
    int pres, counter=0;
    twi_init();
    
    PCA9555_0_write(REG_CONFIGURATION_1,0b11110000);  
    PCA9555_0_write(REG_CONFIGURATION_0,0x00);    
    
    pca_lcd_init();
    
    PCA9555_0_write(REG_OUTPUT_1,0x00);
    
    usart_init(103);

    print_lcd("restarting...");    
    transmit_word("ESP:restart\n");
    while (1) {
        receive_word(answer,256);
        if (strstr(answer, "Waiting for command") != NULL) {
            break;  // ESP is ready
        }
    }
    pca_lcd_clear_display();
    print_lcd(answer);
    pca_lcd_command(0b11000000);
    print_lcd("Connecting...");
    transmit_word("ESP:connect\n");
    receive_word(answer,256);
    pca_lcd_clear_display();
    check(answer,1);
    
    pca_lcd_command(0b11000000);
    
    transmit_word("ESP:url:\"http://192.168.1.250:5000/data\"\n");
    receive_word(answer,256);
    check(answer,2);

    _delay_ms(1000);
           
    while(1){
        
        counter++;
        
        ADCSRA|=(1<<ADSC);
        while(ADCSRA & (1<<ADSC));
        pres = ADC/5.11;
        pressure_digits(pres);
        pres/=10;

        temp = read_temp()+16*12;
        temp_digits(temp);
        temp&=0x07FF;
        temp>>=4;
       
        if (pres>=12 || pres<4){
            status="CHECK PRESSURE";
            if (temp<34 || temp>=37)
                status="CHECK BOTH";
        }
        else if ((temp<34 || temp>=37))
            status="CHECK TEMP"; 
       
        pressed_keys = scan_keypad_rising_edge();
        key = keypad_to_ascii();
        if(key=='#' && strcmp(status,"NURSE CALL")==0){
            status="OK";
        }
        
        pressed_keys = scan_keypad_rising_edge();
        key = keypad_to_ascii();
        if(key=='4')
            status="NURSE CALL";

        //pca_lcd_clear_display();
        pca_lcd_command(0b10000000);
        output_temp();
        print_lcd(" | ");
        pressure_output();        
        pca_lcd_command(0b11000000);
        print_lcd(status);
         
        if(strcmp(status,prev_status) || counter == 100){
          pca_lcd_clear_display();      
          create_payload(payload,sizeof(payload),24,status);
          transmit_word(payload);
          receive_word(answer,256);
          check(answer,3);
          pca_lcd_command(0b11000000);
          transmit_word("ESP:transmit\n");
          receive_word(answer,256);
          check_transmit(answer,4); 
          prev_status = status;
          _delay_ms(1000);
          counter = 0;
          pca_lcd_clear_display();
        }       
    }
}