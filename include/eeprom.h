#ifndef EEPROM_H
#define	EEPROM_H

#include "hw.h"
#include "lcd.h"
#include "locale.h"

#if defined(__AVR)
volatile const struct {
char data;
} eedata[] EEMEM = {
#endif

__EEDATA(0x4F, 0xAC, 0x04, 0x00, 0x74, 0x0B, 0x80, 0x3E) /*config*/
__EEDATA(0x6E, 0xB3, 0xA8, 0x01, 0x00, 0x05, 0x00, 0x00)
__EEDATA(0x09, 0x00, 0x18, 0x28, 0x5D, 0x36, 0x68, 0x00) /*trips*/
__EEDATA(0x12, 0x02, 0x00, 0x00, 0x49, 0x01, 0x61, 0x3A)
__EEDATA(0x0D, 0x4F, 0x9D, 0x0F, 0x23, 0x51, 0x00, 0x00)
__EEDATA(0x1A, 0x00, 0x79, 0x31, 0x3D, 0x09, 0x72, 0x01)
__EEDATA(0xDE, 0x06, 0x00, 0x00, 0x59, 0x12, 0x11, 0x09)
__EEDATA(0x21, 0xD6, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00)
__EEDATA(0x34, 0x20, 0x06, 0x00, 0xA5, 0x37, 0x47, 0x01) /*services*/
__EEDATA(0x00, 0x00, 0xDC, 0x16, 0x0A, 0x22, 0x01, 0x21)
__EEDATA(0x00, 0x00, 0x5F, 0x20, 0x28, 0xFF, 0xFF, 0xFF)
__EEDATA(0x00, 0x00, 0x05, 0x0C, 0x0A, 0x21, 0x03, 0x20)
__EEDATA(0x00, 0x00, 0x70, 0x17, 0x0A, 0x07, 0x01, 0x21)
__EEDATA(0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

// ds18b20 serial numbers (OUT, IN, ENGINE)
__EEDATA(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF) /*ds18b20 serial numbers*/
__EEDATA(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
__EEDATA(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
        
#ifdef EEPROM_CUSTOM_CHARS
#define CUSTOM_CHAR_DATA(a) __EEDATA(a);
CUSTOM_CHAR_DATA(DATA_KMH_0)    // kmh[0]
CUSTOM_CHAR_DATA(DATA_KMH_1)    // kmh[1]
CUSTOM_CHAR_DATA(DATA_OMIN_0)   // omin[0]
CUSTOM_CHAR_DATA(DATA_OMIN_1)   // omin[1]
CUSTOM_CHAR_DATA(DATA_L100_0)   // L100[0]
CUSTOM_CHAR_DATA(DATA_L100_1)   // L100[1]
CUSTOM_CHAR_DATA(DATA_LH_0)     // l/h[0]
CUSTOM_CHAR_DATA(DATA_LH_1)     // l/h[1]
#endif

#if defined(__AVR)
};
#endif

#endif	/* EEPROM_H */

