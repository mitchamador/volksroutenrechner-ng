#ifndef _I2C_H
#define _I2C_H

#include "hw.h"

//---[ I2C Routines ]---

void I2C_Master_Init(void);
void I2C_Master_Start(void);
void I2C_Master_RepeatedStart(void);
void I2C_Master_Stop(void);
void I2C_ACK(void);
void I2C_NACK(void);
unsigned char I2C_Master_Write(unsigned char data);
unsigned char I2C_Read_Byte(void);

#endif