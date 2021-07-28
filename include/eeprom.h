#ifndef EEPROM_H
#define	EEPROM_H

#include "hw.h"
#include "lcd.h"
#include "locale.h"

__EEPROM_DATA(0xEF,0xA6,0x04,0x00,0x0A,0x20,0x80,0x3E); /*config*/
__EEPROM_DATA(0x6E,0xB3,0xA0,0x01,0x00,0x05,0x00,0x00);
__EEPROM_DATA(0xB6,0x00,0xC9,0x26,0x6C,0x02,0xC1,0x07); /*trips*/
__EEPROM_DATA(0x32,0x28,0x00,0x00,0x1C,0x03,0x28,0x0A);
__EEPROM_DATA(0x21,0x06,0xD1,0x22,0x4F,0xBC,0x00,0x00);
__EEPROM_DATA(0x06,0x00,0xEF,0x0A,0x48,0x48,0x41,0x00);
__EEPROM_DATA(0x80,0x01,0x00,0x00,0x15,0x17,0x28,0x07);
__EEPROM_DATA(0x21,0xDB,0x02,0x00,0x00,0x00,0x00,0x00);
__EEPROM_DATA(0xE2,0xD9,0x04,0x00,0x00,0x00,0x7C,0x11); /*services*/
__EEPROM_DATA(0x0A,0x22,0x01,0x21,0x00,0x00,0xFF,0x1A);
__EEPROM_DATA(0x28,0xFF,0xFF,0xFF,0x00,0x00,0xA5,0x06);
__EEPROM_DATA(0x0A,0x21,0x03,0x20,0x00,0x00,0x10,0x12);
__EEPROM_DATA(0x0A,0x07,0x01,0x21,0x00,0x00,0x00,0x00);
// ds18b20 serial numbers (OUT, IN, ENGINE)
__EEPROM_DATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF); /*ds18b20 serial numbers*/
__EEPROM_DATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
__EEPROM_DATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);

#ifdef EEPROM_CUSTOM_CHARS
#define CUSTOM_CHAR_DATA(a) __EEPROM_DATA(a);
CUSTOM_CHAR_DATA(DATA_KMH_0)    // kmh[0]
CUSTOM_CHAR_DATA(DATA_KMH_1)    // kmh[1]
CUSTOM_CHAR_DATA(DATA_OMIN_0)   // omin[0]
CUSTOM_CHAR_DATA(DATA_OMIN_1)   // omin[1]
CUSTOM_CHAR_DATA(DATA_L100_0)   // L100[0]
CUSTOM_CHAR_DATA(DATA_L100_1)   // L100[1]
CUSTOM_CHAR_DATA(DATA_LH_0)     // l/h[0]
CUSTOM_CHAR_DATA(DATA_LH_1)     // l/h[1]
#endif

#endif	/* EEPROM_H */
