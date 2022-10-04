#ifndef I2C_EEPROM_H
#define	I2C_EEPROM_H

#include "utils.h"

#define WAIT_RESPONSE_MS 10

#if defined(JOURNAL_SUPPORT) && !defined(JOURNAL_EEPROM_INTERNAL)
void I2C_read_eeprom_block(unsigned char* p, uint16_t ee_addr, unsigned char length);
void I2C_write_eeprom_block(unsigned char* p, uint16_t ee_addr, unsigned char length);
#endif

#endif	/* I2C_EEPROM_H */

