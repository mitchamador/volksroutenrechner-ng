#ifndef ONEWIRE_H
#define	ONEWIRE_H

#include "main.h"

__bit onewire_start(void);
void onewire_write_bit(uint8_t);
void onewire_write_byte(uint8_t);
__bit onewire_read_bit(void);
uint8_t onewire_read_byte(void);

#endif	/* ONEWIRE_H */

