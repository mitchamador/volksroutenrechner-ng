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

__EEDATA(0x0E,0xF3,0x04,0x00,0xBD,0x1B,0x80,0x3E) /*config*/
__EEDATA(0x6C,0xA9,0x80,0xE7,0x13,0x05,0x00,0x00)
__EEDATA(0x84,0x01,0x26,0x22,0x16,0x18,0xE8,0x10) /*trips*/
__EEDATA(0xBC,0x98,0x00,0x00,0x21,0x02,0xFD,0x2C)
__EEDATA(0x82,0x26,0x8F,0x17,0x14,0xD8,0x00,0x00)
__EEDATA(0x1A,0x00,0x8B,0x28,0x0C,0x57,0x4E,0x01)
__EEDATA(0x40,0x0C,0x00,0x00,0x32,0x17,0x17,0x07)
__EEDATA(0x23,0x07,0x15,0x03,0x00,0x00,0x00,0x00)
__EEDATA(0xD7,0xCB,0x02,0x00,0x00,0x00,0x62,0x06) /*services*/
__EEDATA(0x0A,0x18,0x05,0x23,0xEF,0x3F,0x28,0x07)
__EEDATA(0x11,0x21,0x95,0x0D,0x0A,0x15,0x03,0x23)
__EEDATA(0x95,0x0D,0x0A,0x15,0x03,0x23,0x00,0x00)

// ds18b20 serial numbers (OUT, IN, ENGINE)
__EEDATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF) /*ds18b20 serial numbers*/
__EEDATA(0x28,0x27,0xF6,0xDC,0x06,0x00,0x00,0xD5)
__EEDATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF)

// custom lcd characters
__EEDATA(0x05,0x06,0x05,0x00,0x00,0x00,0x01,0x02) // 0x00 - kmh[0]
__EEDATA(0x0A,0x15,0x15,0x00,0x10,0x05,0x07,0x01) // 0x01 - kmh[1]
__EEDATA(0x07,0x05,0x07,0x00,0x00,0x01,0x02,0x00) // 0x02 - omin[0]
__EEDATA(0x00,0x00,0x08,0x10,0x00,0x0A,0x15,0x15) // 0x03 - omin[1]
__EEDATA(0x0C,0x14,0x14,0x01,0x02,0x05,0x01,0x01) // 0x04 - L100[0]
__EEDATA(0x00,0x00,0x00,0x00,0x00,0x1F,0x15,0x1F) // 0x05 - L100[1]
__EEDATA(0x03,0x05,0x05,0x00,0x00,0x01,0x02,0x00) // 0x06 - l/h[0]
__EEDATA(0x00,0x00,0x08,0x10,0x00,0x14,0x1C,0x04) // 0x07 - l/h[1]

// continuous data
__EEDATA(0x30,0x28,0x01,0x00,0x09,0x09,0x7C,0x9E)
__EEDATA(0x0B,0x00,0x09,0x00,0x0C,0x00,0x0C,0x00)

#if defined(__AVR)
#if defined(PROGMEM_EEPROM)
// special eeprom mark
__EEDATA(0x00,0x00,0x00,0x00,0xDE,0xAD,0xC0,0xDE)
#endif
};
#endif

// internal eeprom journal data
#if defined(__AVR) && defined(JOURNAL_EEPROM_INTERNAL)

#include "journal.h"

// dummy bytes data
volatile const char dummy_data[J_EEPROM_START - sizeof(eedata)] EEMEM;

// array size is 768
volatile const char ee_journal_data[J_EEPROM_LENGTH] EEMEM = {
  0x4a,0x4f,0x55,0x52,0x54,0x52,0x49,0x50,0x13,0x14,0x0b,0x0c,0x05,0x06,0x00,0x05,
  0x07,0x07,0x17,0x07,0x23,0x20,0x20,0x04,0x07,0x23,0x33,0x09,0x01,0x07,0x23,0x00,
  0xa5,0x07,0x07,0x26,0x06,0x23,0x10,0x00,0xa1,0x11,0x47,0x4c,0xeb,0x00,0xa1,0x08,
  0x00,0x00,0xa5,0x05,0x07,0x27,0x06,0x23,0x10,0x00,0xb2,0x12,0x52,0x43,0xe8,0x00,
  0x14,0x09,0x00,0x00,0xa5,0x05,0x07,0x28,0x06,0x23,0x10,0x00,0xd9,0x12,0x7e,0x50,
  0xec,0x00,0x45,0x09,0x00,0x00,0xa5,0x03,0x07,0x29,0x06,0x23,0x37,0x00,0xee,0x1c,
  0x66,0x5d,0x78,0x02,0x2b,0x17,0x00,0x00,0xa5,0x05,0x07,0x30,0x06,0x23,0x10,0x00,
  0x37,0x11,0x46,0x28,0xf8,0x00,0x44,0x0a,0x00,0x00,0xa5,0x33,0x09,0x01,0x07,0x23,
  0x2e,0x00,0x43,0x10,0x73,0x14,0x2e,0x02,0xf1,0x15,0x00,0x00,0xa5,0x27,0x20,0x02,
  0x07,0x23,0x1c,0x00,0xc4,0x18,0x51,0x4c,0x01,0x01,0xe6,0x07,0x00,0x00,0xa5,0x40,
  0x16,0x03,0x07,0x23,0x25,0x00,0x31,0x19,0x67,0x32,0x66,0x01,0x27,0x0c,0x00,0x00,
  0xa5,0x05,0x07,0x04,0x07,0x23,0x35,0x00,0x62,0x29,0x05,0x01,0x5b,0x02,0xe0,0x17,
  0x00,0x00,0xa5,0x04,0x07,0x05,0x07,0x23,0x13,0x00,0xb9,0x14,0x10,0x1a,0xe9,0x00,
  0x07,0x08,0x00,0x00,0xa5,0x02,0x07,0x06,0x07,0x23,0x26,0x00,0x7e,0x15,0x7b,0x48,
  0xcf,0x01,0xd6,0x11,0x00,0x00,0xa5,0x04,0x07,0x07,0x07,0x23,0x13,0x00,0x4d,0x39,
  0x63,0x1e,0x12,0x01,0x5c,0x0b,0x00,0x00,0xa5,0x35,0x19,0x08,0x07,0x23,0x4c,0x00,
  0xb3,0x0a,0x7e,0x26,0x89,0x02,0xfd,0x10,0x00,0x00,0xa5,0x42,0x19,0x09,0x07,0x23,
  0x21,0x00,0x4a,0x2a,0x1d,0x2d,0x5f,0x01,0xa1,0x0c,0x00,0x00,0xa5,0x03,0x07,0x10,
  0x07,0x23,0x1a,0x00,0x06,0x34,0x20,0x38,0x39,0x01,0x15,0x0b,0x00,0x00,0xa5,0x07,
  0x07,0x11,0x07,0x23,0x12,0x00,0x17,0x32,0x75,0x5a,0xe5,0x00,0x03,0x08,0x00,0x00,
  0xa5,0x12,0x07,0x12,0x07,0x23,0x13,0x00,0xbf,0x10,0x0a,0x0a,0xf4,0x00,0xfe,0x08,
  0x00,0x00,0xa5,0x07,0x07,0x13,0x07,0x23,0x13,0x00,0x92,0x10,0x47,0x0e,0xef,0x00,
  0x87,0x08,0x00,0x00,0xa5,0x06,0x07,0x14,0x07,0x23,0x13,0x00,0xd2,0x03,0x6b,0x53,
  0xed,0x00,0x3e,0x09,0x00,0x00,0xa5,0x58,0x08,0x15,0x07,0x23,0x3e,0x00,0x97,0x2a,
  0x40,0x4a,0xaa,0x02,0x44,0x19,0x00,0x00,0xa5,0x46,0x22,0x20,0x12,0x22,0x2d,0x01,
  0xf5,0x38,0x1b,0x26,0x80,0x10,0x74,0xb2,0x00,0x00,0xa5,0x23,0x13,0x31,0x12,0x22,
  0x73,0x01,0x5e,0x1d,0x07,0x15,0xf4,0x12,0x3b,0xca,0x00,0x00,0xa5,0x17,0x09,0x21,
  0x01,0x23,0xad,0x02,0xe4,0x26,0x7c,0x16,0xad,0x21,0x44,0x54,0x01,0x00,0xa5,0x34,
  0x10,0x11,0x02,0x23,0xa5,0x01,0xe5,0x12,0x16,0x1f,0x51,0x15,0x2d,0xd8,0x00,0x00,
  0xa5,0x00,0x10,0x25,0x02,0x23,0x78,0x01,0xdc,0x15,0x2c,0x19,0xe7,0x12,0xef,0xc1,
  0x00,0x00,0xa5,0x50,0x13,0x11,0x03,0x23,0xbe,0x02,0x86,0x26,0x72,0x3a,0xc6,0x21,
  0xb2,0x55,0x01,0x00,0xa5,0x55,0x16,0x01,0x04,0x23,0xb7,0x01,0x0b,0x03,0x02,0x67,
  0x67,0x15,0xd0,0xcd,0x00,0x00,0xa5,0x15,0x11,0x15,0x04,0x23,0x87,0x01,0x91,0x30,
  0x7d,0x40,0x8f,0x13,0x31,0xc4,0x00,0x00,0xa5,0x32,0x20,0x07,0x05,0x23,0x3b,0x02,
  0xbb,0x24,0x2b,0x00,0x92,0x1b,0x12,0x1a,0x01,0x00,0xa5,0x06,0x20,0x20,0x05,0x23,
  0x6d,0x01,0xcb,0x2a,0x04,0x31,0x10,0x11,0x0d,0xa8,0x00,0x00,0xa5,0x57,0x10,0x12,
  0x06,0x23,0x98,0x01,0x84,0x36,0x20,0x1d,0x04,0x12,0x27,0xb1,0x00,0x00,0xa5,0x21,
  0x07,0x21,0x06,0x23,0x86,0x01,0x95,0x28,0x1b,0x3f,0x35,0x12,0xaa,0xb3,0x00,0x00,
  0xa5,0x36,0x12,0x01,0x01,0x23,0xb3,0x02,0x69,0x38,0x7e,0x31,0x6e,0x22,0x51,0x67,
  0x01,0x00,0xa5,0x00,0x07,0x01,0x02,0x23,0x69,0x03,0x95,0x17,0x20,0x1a,0xcc,0x2b,
  0x48,0xb9,0x01,0x00,0xa5,0x06,0x07,0x01,0x03,0x23,0x99,0x03,0x99,0x37,0x53,0x3e,
  0x79,0x2d,0xb4,0xd3,0x01,0x00,0xa5,0x17,0x09,0x01,0x04,0x23,0xa4,0x02,0x9e,0x37,
  0x38,0x44,0xeb,0x20,0x67,0x3e,0x01,0x00,0xa5,0x48,0x14,0x02,0x05,0x23,0x48,0x04,
  0x1b,0x2a,0x75,0x4f,0xb4,0x34,0x80,0x13,0x02,0x00,0xa5,0x26,0x10,0x11,0x06,0x23,
  0xba,0x02,0xe8,0x22,0x0d,0x2f,0x28,0x20,0xdb,0x3e,0x01,0x00,0xa5,0x59,0x11,0x19,
  0x07,0x22,0x00,0x3c,0xf6,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

#endif

#endif	/* EEPROM_H */

