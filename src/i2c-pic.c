#include "i2c.h"

#if defined(__PIC_MIDRANGE)
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

unsigned char I2C_Read_Byte_ACK(void)
{
    
    //---[ Receive & Return A Byte ]---
	I2C_Master_Wait;
    RCEN = 1;		  // Enable & Start Reception
	while(!SSPIF);	  // Wait Until Completion
	SSPIF = 0;		  // Clear The Interrupt Flag Bit
    I2C_Master_Wait;
    
	ACKDT = 0;			// 0 -> ACK
	I2C_Master_Wait;
    ACKEN = 1;			// Send ACK

    return SSPBUF;	  // Return The Received Byte
}

unsigned char I2C_Read_Byte_NACK(void)
{
    //---[ Receive & Return A Byte ]---
	I2C_Master_Wait;
    RCEN = 1;		  // Enable & Start Reception
	while(!SSPIF);	  // Wait Until Completion
	SSPIF = 0;		  // Clear The Interrupt Flag Bit
    I2C_Master_Wait;

	ACKDT = 1;			// 1 -> NACK
    I2C_Master_Wait;
	ACKEN = 1;			// Send NACK

    return SSPBUF;	  // Return The Received Byte
}

#endif
