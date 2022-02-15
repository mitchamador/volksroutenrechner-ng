#include "i2c.h"

#ifdef I2C_SOFTWARE

void I2C_Master_Init()
{
/*
    SCL_HIGH;
    SCL_OUTPUT;
    SDA_HIGH;
    SDA_INPUT;
*/
}

void I2C_Master_Start()
{
    //SDA_INPUT;                  // ensure SDA & SCL are high
    SCL_HIGH;
    SDA_OUTPUT;                   // SDA = output
    SDA_LOW;                      // pull SDA low
    delay_us(3.5);
}

void I2C_Master_Stop()
{
    SDA_OUTPUT;                   // SDA = output
    SDA_LOW;                      // SDA low
    delay_us(3.5);
    SCL_HIGH;                     // pull SCL high
    SDA_INPUT;                    // allow SDA to be pulled high
}

//....................................................................
// Outputs a bit to the I2C bus
//....................................................................
void bit_out(unsigned char data)
{
    SCL_LOW;                      // ensure SCL is low
    SDA_OUTPUT;                   // configure SDA as an output
    if (data == 0) {
        SDA_LOW;
    } else {
        SDA_HIGH;
    }
    delay_us(3.5);
    SCL_HIGH;                     // pull SCL high to clock bit
    delay_us(2);
}


//....................................................................
// Inputs a bit from the I2C bus
//....................................................................
unsigned char bit_in()
{
    SCL_LOW;                      // ensure SCL is low
    delay_us(3.5);
    SDA_INPUT;                    // configure SDA as an input
    SCL_HIGH;                     // bring SCL high to begin transfer
    delay_us(2);
    return SDA_GET();             // input the received bit
}

unsigned char I2C_Master_Write(unsigned char data)
{
    unsigned char mask = 0x80;    // mask

    do {
        bit_out(data & mask);     // output MSB bit
        mask = mask >> 1;
    } while (mask != 0);

    mask = bit_in();              // input ACK bit
    delay_us(1);
    SCL_LOW;                      // pull SCL low
    return mask;
}

unsigned char I2C_Read_Byte(unsigned char ack)
{
    unsigned char i = 8;          // loop counter
    unsigned char ret = 0;        // return value

    do {
        ret = (unsigned char) (ret << 1); // shift left for next bit
        ret |= bit_in();                  // input bit
    } while (--i != 0);

    bit_out(ack);                 // output ACK/NAK bit
    delay_us(1);
    SCL_LOW;                      // pull SCL low
    return ret;
}

#endif
