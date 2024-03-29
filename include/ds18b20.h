#ifndef DS18B20_H
#define	DS18B20_H

#include "onewire.h"

// magic number
#define DS18B20_TEMP_NONE 0x8181

flag_t ds18b20_start_conversion(void);
flag_t ds18b20_read_rom(unsigned char *);
flag_t ds18b20_read_temp_matchrom(unsigned char*, uint16_t *);

#endif	/* DS18B20_H */

