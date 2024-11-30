/*
 * File:   7_2.c
 * Author: panos_katsalifis
 *
 * Created on November 12, 2024, 1:50 PM
 */

#define F_CPU 16000000UL

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

void writeWord(char* str){
    int i = 0;
    while(str[i]!=0)
        pca_lcd_data(str[i++]);
}

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

uint8_t sign,d1,d2,d3,d4,d5,d6;

void digits(uint16_t value){
    //value = ~value+1;
    sign = value&(0xf800);
    value=(value&0x07f0)>>4;
    uint8_t decimal = value&(0x0f);
    
    int result = (int)value;
    
    double dval = decimal*62.5;
    
    d4 = (int)dval/100;
    d5 = (int)dval/10;
    d5 %= 10;
    d6 = (int)dval%10;
    
    d1 = result/100;
    d2 = result/10;
    d2 %= 10;
    d3 = result%10;
}

void output(){
    if(sign==0)
        pca_lcd_data('+');
    else pca_lcd_data('-');
    
    d1+=48;
    d2+=48;
    d3+=48;  
    d4+=48;
    d5+=48;
    d6+=48;
    
    if(d1!=48)
        pca_lcd_data(d1);
    if(d1!=48 || d2!=48)
        pca_lcd_data(d2);
    pca_lcd_data(d3);
    pca_lcd_data('.');
    pca_lcd_data(d4);
    if(d5!=48 || d6!=48)
        pca_lcd_data(d5);
    if(d6!=48)
        pca_lcd_data(d6);
    
    pca_lcd_data(0xDF);
    pca_lcd_data('C');
}

int main(void) {
    twi_init();
    pca_lcd_init();
    PCA9555_0_write(REG_CONFIGURATION_0,0x00);    
    int temp;
    
    while (1) {
        temp = read_temp();
        if(temp==0x8000){
            pca_lcd_clear_display();
            writeWord("No Device");
        }
        else {
            pca_lcd_clear_display();
            digits(temp);
            output();
        }
        _delay_ms(450);
        pca_lcd_clear_display();
        _delay_ms(50);
    }
}
