#ifndef HW_H
#define	HW_H

#include "config.h"

#if defined(_MPC_)
// pic
#include "hw-pic.h"
#elif defined(__AVR)
// avr
#include "hw-avr.h"
#else
#error "device not supported"
#endif

void HW_Init(void);
void HW_read_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length);
void HW_write_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length);

#include "i2c.h"

#endif	/* HW_H */

