#ifndef _I2C_H
#define _I2C_H

#include "hw.h"

#ifdef I2C_BITBANG
#define ACK 0x00
#define NACK 0x80
#else
#define ACK 0
#define NACK 1
#endif

// define i2c bus frequency
#define I2C_BaudRate 100000

//---[ I2C Routines ]---

void I2C_Master_Init(void);
void I2C_Master_Start(void);
#ifdef I2C_BITBANG
#define I2C_Master_RepeatedStart() I2C_Master_Start();
#else
void I2C_Master_RepeatedStart(void);
#endif
void I2C_Master_Stop(void);
unsigned char I2C_Master_Write(unsigned char data);
unsigned char I2C_Read_Byte(unsigned char);

#define I2C_Read_Byte_ACK() I2C_Read_Byte(ACK);
#define I2C_Read_Byte_NACK() I2C_Read_Byte(NACK);

#endif
