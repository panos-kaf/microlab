/*
 * File:   5_2.c
 * Author: panos_katsalifis
 *
 * Created on November 5, 2024, 1:13 PM
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

char name1[] = "Spiros Agathos";
char name2[] = "Panos Katsalifis";

void writeWord(char* word){
    int i = 0;
    while(word[i]!=0)
        pca_lcd_data(word[i++]);
}


int main(void){
    
    twi_init();
    
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output
    
    pca_lcd_init();
    
    while(1){
        writeWord(name1);
        _delay_ms(1000);
        pca_lcd_clear_display();
        _delay_ms(100);
        writeWord(name2);
        _delay_ms(1000);
        pca_lcd_clear_display();
        _delay_ms(100);
    }
}