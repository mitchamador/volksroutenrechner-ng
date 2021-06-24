#include "i2c.h"

#if defined(__PIC_MIDRANGE)

#ifdef I2C_BITBANG

void I2C_Master_Init()
{
}

void I2C_Master_Start()
{
    unsigned int i;

    SDA_TRIS = 1;                   // ensure SDA & SCL are high
    SCL = 1;
    SDA_TRIS = 0;                   // SDA = output
    SDA = 0;                        // pull SDA low
    for (i=0;i<2;i++) NOP();
    //SCL = 0;                        // pull SCL low
}

void I2C_Master_Stop()
{
    unsigned int i;

    SCL = 0;                        // ensure SCL is low
    SDA_TRIS = 0;                   // SDA = output
    SDA = 0;                        // SDA low
    for (i=0;i<3;i++) NOP();
    SCL = 1;                        // pull SCL high
    SDA_TRIS = 1;                   // allow SDA to be pulled high
    for (i=0;i<3;i++) NOP();
    //SCL=0;                          // ensure SCL is low
}

//....................................................................
// Outputs a bit to the I2C bus
//....................................................................
void bit_out(unsigned char data)
{
    unsigned int i;

    SCL = 0;                        // ensure SCL is low
    SDA_TRIS=0;                     // configure SDA as an output
    SDA= (data>>7);                 // output the MSB
    for (i=0;i<2;i++) NOP();
    SCL = 1;                        // pull SCL high to clock bit
    for (i=0;i<3;i++) NOP();
    SCL = 0;                        // pull SCL low for next bit
}


//....................................................................
// Inputs a bit from the I2C bus
//....................................................................
void bit_in(unsigned char *data)
{
    unsigned int i;

    SCL = 0;                        // ensure SCL is low
    SDA_TRIS = 1;                   // configure SDA as an input
    SCL = 1;                        // bring SCL high to begin transfer
    for (i=0;i<3;i++) NOP();
    *data |= SDA;                   // input the received bit
    SCL = 0;                        // bring SCL low again.
}

unsigned char I2C_Master_Write(unsigned char data)
{
    unsigned char i;                // loop counter
    unsigned char ack;              // ACK bit

    ack = 0;
    for (i = 0; i < 8; i++)         // loop through each bit
        {
        bit_out(data);              // output bit
        data = (unsigned char) (data << 1);           // shift left for next bit
        }

    bit_in(&ack);                   // input ACK bit
    return ack;
}

unsigned char I2C_Read_Byte(unsigned char ack)
{
    unsigned char i;                // loop counter
    unsigned char ret=0;            // return value

    for (i = 0; i < 8; i++)         // loop through each bit
        {
        ret = (unsigned char) (ret << 1);             // shift left for next bit
        bit_in(&ret);               // input bit
        }

    bit_out(ack);                   // output ACK/NAK bit
    return ret;
}

#else

#define I2C_Master_Wait while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F)) \
    ;

//---------------[ I2C Routines ]-------------------
//--------------------------------------------------
void I2C_Master_Init()
{
  SSPCON  = 0x28;
  SSPCON2 = 0x00;
  SSPSTAT = 0x00;
  SSPADD = ((_XTAL_FREQ/4)/I2C_BaudRate) - 1;
}

void I2C_Master_Start()
{
    I2C_Master_Wait;
    SEN = 1;
}

void I2C_Master_RepeatedStart()
{
    I2C_Master_Wait;
    RSEN = 1;
}

void I2C_Master_Stop()
{
    I2C_Master_Wait;
    PEN = 1;
}

unsigned char I2C_Master_Write(unsigned char data)
{
    I2C_Master_Wait;
    SSPBUF = data;
    while(!SSPIF);  // Wait Until Completion
	SSPIF = 0;
    return ACKSTAT;
}

unsigned char I2C_Read_Byte(unsigned char ack)
{
    
    //---[ Receive & Return A Byte ]---
	I2C_Master_Wait;
    RCEN = 1;		  // Enable & Start Reception
	while(!SSPIF);	  // Wait Until Completion
	SSPIF = 0;		  // Clear The Interrupt Flag Bit
    I2C_Master_Wait;

    if (ack == NACK) {
        ACKDT = 1;      // 1 -> NACK
    } else {
        ACKDT = 0;      // 0 -> ACK
    }
	I2C_Master_Wait;
    ACKEN = 1;			// Send ACK/NACK

    return SSPBUF;	  // Return The Received Byte
}
#endif

#endif
