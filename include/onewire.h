#ifndef ONEWIRE_H
#define	ONEWIRE_H

#include "core.h"

#if defined(__AVR__)
#include <util/crc16.h>
#endif

__bit onewire_start(void);
void onewire_write_bit(uint8_t);
void onewire_write_byte(uint8_t);
__bit onewire_read_bit(void);
uint8_t onewire_read_byte(void);
uint8_t onewire_crc8(const uint8_t*, uint8_t);
__bit onewire_search();
uint8_t onewire_search_devices(uint8_t *buf, int8_t max_devices);

#endif	/* ONEWIRE_H */

