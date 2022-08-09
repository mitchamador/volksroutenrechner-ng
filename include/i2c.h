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
