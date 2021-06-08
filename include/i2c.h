#ifndef _I2C_H
#define _I2C_H

#include "hw.h"

// define i2c bus frequency
#define I2C_BaudRate 100000

//---[ I2C Routines ]---

void I2C_Master_Init(void);
void I2C_Master_Start(void);
void I2C_Master_RepeatedStart(void);
void I2C_Master_Stop(void);
unsigned char I2C_Master_Write(unsigned char data);
unsigned char I2C_Read_Byte_ACK(void);
unsigned char I2C_Read_Byte_NACK(void);

#endif
