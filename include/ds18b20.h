/* 
 * File:   ds18b20.h
 * Author: victor
 *
 * Created on 1 июня 2021 г., 16:24
 */

#ifndef DS18B20_H
#define	DS18B20_H

#include "hw.h"
#include <stdint.h>

__bit ds18b20_start(void);
void ds18b20_write_bit(uint8_t);
void ds18b20_write_byte(uint8_t);
__bit ds18b20_read_bit(void);
uint8_t ds18b20_read_byte(void);
__bit ds18b20_read(uint16_t *);
__bit ds18b20_start_conversion(void);
__bit ds18b20_read_temp_skiprom(uint16_t*);
__bit ds18b20_read_rom(unsigned char *);
__bit ds18b20_read_temp_matchrom(unsigned char*, uint16_t *);
void ds18b20_serial_to_string(unsigned char*, unsigned char*);

#endif	/* DS18B20_H */

