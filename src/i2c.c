#include "i2c.h"

#ifdef I2C_SOFTWARE

unsigned char bit_in(void);
void bit_out(unsigned char);

void I2C_Master_Init()
{
/*
    SCL_HIGH;
    SCL_OUTPUT;
    SDA_HIGH;
    SDA_INPUT;
*/
}

#if defined(_16F876A)

unsigned char _I2C_Master_Write(unsigned char address, unsigned char start)
{
    if (start != 0) {
        //SDA_INPUT;                  // ensure SDA & SCL are high
        HW_i2c_soft_scl_high();
        HW_delay_us(0.6);
        HW_i2c_soft_sda_output();     // SDA = output
        HW_i2c_soft_sda_low();        // pull SDA low
        HW_delay_us(3.5);
    }

    unsigned char mask = 0x80;        // mask

    do {
        bit_out(address & mask);      // output MSB bit
        mask = mask >> 1;
    } while (mask != 0);

    mask = bit_in();                  // input ACK bit
    HW_delay_us(1);
    HW_i2c_soft_scl_low();            // pull SCL low
    return mask;

}

#else

unsigned char I2C_Master_Start(unsigned char address)
{
    //SDA_INPUT;                  // ensure SDA & SCL are high
    HW_i2c_soft_scl_high();
    HW_delay_us(0.6);
    HW_i2c_soft_sda_output();     // SDA = output
    HW_i2c_soft_sda_low();        // pull SDA low
    HW_delay_us(3.5);

    return I2C_Master_Write(address);
}

unsigned char I2C_Master_Write(unsigned char data)
{
    unsigned char mask = 0x80;    // mask

    do {
        bit_out(data & mask);     // output MSB bit
        mask = mask >> 1;
    } while (mask != 0);

    mask = bit_in();              // input ACK bit
    HW_delay_us(1);
    HW_i2c_soft_scl_low();        // pull SCL low
    return mask;

}

#endif

void I2C_Master_Stop()
{
    HW_i2c_soft_sda_output();     // SDA = output
    HW_i2c_soft_sda_low();        // SDA low
    HW_delay_us(3.5);
    HW_i2c_soft_scl_high();       // pull SCL high
    HW_delay_us(0.6);
    HW_i2c_soft_sda_input();      // allow SDA to be pulled high
}

//....................................................................
// Outputs a bit to the I2C bus
//....................................................................
void bit_out(unsigned char data)
{
    HW_i2c_soft_scl_low();        // ensure SCL is low
    HW_i2c_soft_sda_output();     // configure SDA as an output
    if (data == 0) {
        HW_i2c_soft_sda_low();
    } else {
        HW_i2c_soft_sda_high();
    }
    HW_delay_us(3.5);
    HW_i2c_soft_scl_high();       // pull SCL high to clock bit
    HW_delay_us(2);
}


//....................................................................
// Inputs a bit from the I2C bus
//....................................................................
unsigned char bit_in()
{
    HW_i2c_soft_scl_low();        // ensure SCL is low
    HW_delay_us(3.5);
    HW_i2c_soft_sda_input();      // configure SDA as an input
    HW_i2c_soft_scl_high();       // bring SCL high to begin transfer
    HW_delay_us(2);
    return HW_i2c_soft_sda_get();             // input the received bit
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
    HW_delay_us(1);
    HW_i2c_soft_scl_low();        // pull SCL low
    return ret;
}

#endif
