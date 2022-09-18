#ifndef _I2C_H
#define _I2C_H

#include "hw.h"

#define ACK 0
#define NACK 1

// define i2c bus frequency
#ifdef SLOW_I2C /* for slow devices like DS1307, anyway legacy hw uses software i2c */
#define I2C_BaudRate 100000
#else
#define I2C_BaudRate 400000
#endif

//---[ I2C Routines ]---

#define I2C_Master_RepeatedStart(address) I2C_Master_Start(address)

void I2C_Master_Init(void);

#if defined(I2C_SOFTWARE) && defined(_16F876A)
unsigned char _I2C_Master_Write(unsigned char, unsigned char);
#define I2C_Master_Start(address) _I2C_Master_Write(address, 1)
#define I2C_Master_Write(address) _I2C_Master_Write(address, 0)
#else
unsigned char I2C_Master_Start(unsigned char);
unsigned char I2C_Master_Write(unsigned char);
#endif

void I2C_Master_Stop(void);
unsigned char I2C_Read_Byte(unsigned char);

#endif
