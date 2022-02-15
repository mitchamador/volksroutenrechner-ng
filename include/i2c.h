#ifndef _I2C_H
#define _I2C_H

#include "hw.h"

#ifdef I2C_SOFTWARE
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
#ifdef I2C_SOFTWARE
#define I2C_Master_RepeatedStart() delay_us(3.5); I2C_Master_Start();
#else
void I2C_Master_RepeatedStart(void);
#endif
void I2C_Master_Stop(void);
unsigned char I2C_Master_Write(unsigned char data);
unsigned char I2C_Read_Byte(unsigned char);

#endif
