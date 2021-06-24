#include "i2c.h"
#include <stdint.h>

#if defined(__AVR_ATMEGA)
#include <util/twi.h>
//---------------[ I2C Routines ]-------------------
//--------------------------------------------------

void I2C_Master_Init()
{
  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
  
  TWSR = 0;                         /* no prescaler */
  TWBR = ((F_CPU/I2C_BaudRate)-16)/2;  /* must be > 10 for stable operation */
}

void I2C_Master_Start()
{
//    uint8_t   twst;

	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

//	// check value of TWI Status Register. Mask prescaler bits.
//	twst = TW_STATUS & 0xF8;
//	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;
//
//	return 0;
}

void I2C_Master_RepeatedStart()
{
    return I2C_Master_Start();
}

void I2C_Master_Stop()
{
    /* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));
}

unsigned char I2C_Read_Byte(unsigned char ack)
{
    if (ack == ACK) {
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    } else {
        TWCR = (1<<TWINT) | (1<<TWEN);
    }
	while(!(TWCR & (1<<TWINT)));
    return TWDR;
}

unsigned char I2C_Master_Write(unsigned char data)
{
    uint8_t   twst;
    
	// send data to the previously addressed device
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits
	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;
}

#endif
