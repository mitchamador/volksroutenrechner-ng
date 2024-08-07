#ifndef EEPROM_H
#define	EEPROM_H

#if defined(__AVR)

#ifdef PROGMEM_EEPROM
// save default eeprom in progmem
PROGMEM const char eedata[] = {
#else
volatile const struct {
char data;
} eedata[] EEMEM = {
#endif      
#endif

__EEDATA(0x39,0x1B,0x05,0x00,0x20,0x22,0x80,0x3E) /*config*/
__EEDATA(0x6C,0xAC,0x80,0xE7,0x13,0x05,0x00,0x00)
__EEDATA(0x31,0x02,0xA8,0x05,0x11,0x49,0x3D,0x16) /*trips*/
__EEDATA(0x0A,0xC7,0x00,0x00,0x92,0x00,0x7C,0x01)
__EEDATA(0x25,0x22,0x94,0x05,0x97,0x2D,0x00,0x00)
__EEDATA(0x61,0x00,0x0C,0x01,0x1B,0x54,0x3B,0x03)
__EEDATA(0x40,0x15,0x00,0x00,0x22,0x17,0x03,0x07)
__EEDATA(0x24,0x07,0x4A,0x04,0x03,0x00,0x00,0x00)
__EEDATA(0xC1,0x31,0x00,0x00,0x00,0x00,0x9D,0x00) /*services*/
__EEDATA(0x0A,0x30,0x06,0x24,0x28,0x68,0x28,0x07)
__EEDATA(0x11,0x21,0xCE,0x35,0x0A,0x15,0x03,0x23)
__EEDATA(0xCE,0x35,0x0A,0x15,0x03,0x23,0x00,0x00)

// ds18b20 serial numbers (OUT, IN, ENGINE)
__EEDATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF) /*ds18b20 serial numbers*/
__EEDATA(0x28,0x27,0xF6,0xDC,0x06,0x00,0x00,0xD5)
__EEDATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF)

// custom lcd characters
__EEDATA(0x05,0x06,0x05,0x00,0x00,0x01,0x02,0x00) // 0x00 - kmh[0]
__EEDATA(0x0A,0x15,0x15,0x00,0x15,0x07,0x01,0x00) // 0x01 - kmh[1]
__EEDATA(0x07,0x05,0x07,0x00,0x01,0x00,0x00,0x00) // 0x02 - omin[0]
__EEDATA(0x00,0x00,0x08,0x10,0x0A,0x15,0x15,0x00) // 0x03 - omin[1]
__EEDATA(0x0C,0x14,0x15,0x02,0x05,0x01,0x01,0x00) // 0x04 - L100[0]
__EEDATA(0x00,0x00,0x00,0x00,0x1F,0x15,0x1F,0x00) // 0x05 - L100[1]
__EEDATA(0x03,0x05,0x05,0x00,0x01,0x00,0x00,0x00) // 0x06 - l/h[0]
__EEDATA(0x00,0x00,0x08,0x10,0x0A,0x0E,0x02,0x00) // 0x07 - l/h[1]

// continuous data
__EEDATA(0x69,0x77,0x01,0x00,0x09,0x09,0x23,0xED)
__EEDATA(0x0B,0x00,0x09,0x00,0x0C,0x00,0x0C,0x00)

#if defined(__AVR)
#if defined(PROGMEM_EEPROM)
// special eeprom mark
__EEDATA(0x00,0x00,0x00,0x00,0xDE,0xAD,0xC0,0xDE)
#endif
};
#endif

// internal eeprom journal data
#if defined(JOURNAL_EEPROM_INTERNAL)

#include "journal.h"
// dummy bytes data
#ifdef PROGMEM_EEPROM
volatile const char dummy_data[J_EEPROM_START] EEMEM;
#else
volatile const char dummy_data[J_EEPROM_START - sizeof(eedata)] EEMEM;
#endif

#include "eeprom-journal.h"

#endif

#endif	/* EEPROM_H */

