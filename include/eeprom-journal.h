#ifndef EEPROM_JOURNAL_H
#define	EEPROM_JOURNAL_H

#if defined(JOURNAL_EEPROM_INTERNAL)

#include "journal.h"

volatile const char ee_journal_data[J_EEPROM_LENGTH] EEMEM = {
  0x4a,0x4f,0x55,0x52,0x54,0x52,0x49,0x50,0x13,0x14,0x0b,0x0c,0x05,0x06,0x00,0x05,
  0x42,0x15,0x03,0x07,0x24,0x31,0x08,0x23,0x06,0x24,0x04,0x07,0x01,0x07,0x24,0x00,
  0xaa,0x08,0x07,0x13,0x06,0x24,0x04,0x02,0x1c,0x01,0x77,0x00,0x3e,0x00,0x6d,0x00,
  0x00,0x00,0xaa,0x07,0x07,0x14,0x06,0x24,0xf2,0x00,0xfb,0x00,0x84,0x00,0x20,0x00,
  0x3a,0x00,0x00,0x00,0xaa,0x42,0x08,0x15,0x06,0x24,0x82,0x05,0x07,0x02,0x5a,0x00,
  0x7f,0x00,0xa3,0x00,0x00,0x00,0xaa,0x20,0x09,0x16,0x06,0x24,0x2a,0x01,0x68,0x01,
  0x68,0x00,0x1f,0x00,0x30,0x00,0x00,0x00,0xaa,0x03,0x07,0x17,0x06,0x24,0x3d,0x01,
  0x1a,0x01,0x77,0x00,0x26,0x00,0x42,0x00,0x00,0x00,0xaa,0x06,0x07,0x18,0x06,0x24,
  0xd6,0x01,0x2d,0x01,0x72,0x00,0x36,0x00,0x5d,0x00,0x00,0x00,0xaa,0x07,0x07,0x19,
  0x06,0x24,0xee,0x01,0x34,0x01,0x72,0x00,0x38,0x00,0x60,0x00,0x00,0x00,0xaa,0x03,
  0x07,0x20,0x06,0x24,0xa8,0x00,0xd7,0x00,0x95,0x00,0x19,0x00,0x2c,0x00,0x00,0x00,
  0xaa,0x03,0x07,0x21,0x06,0x24,0x97,0x00,0xef,0x00,0x8b,0x00,0x15,0x00,0x25,0x00,
  0x00,0x00,0xaa,0x45,0x08,0x22,0x06,0x24,0xcd,0x00,0x34,0x01,0x77,0x00,0x18,0x00,
  0x27,0x00,0x00,0x00,0xaa,0x54,0x07,0x23,0x06,0x24,0x0d,0x01,0x55,0x01,0x6d,0x00,
  0x1d,0x00,0x2e,0x00,0x00,0x00,0xaa,0x05,0x07,0x24,0x06,0x24,0x43,0x01,0x35,0x01,
  0x75,0x00,0x26,0x00,0x3e,0x00,0x00,0x00,0xaa,0x05,0x07,0x25,0x06,0x24,0x43,0x01,
  0x22,0x01,0x78,0x00,0x27,0x00,0x42,0x00,0x00,0x00,0xaa,0x07,0x07,0x26,0x06,0x24,
  0x44,0x01,0x29,0x01,0x75,0x00,0x26,0x00,0x41,0x00,0x00,0x00,0xaa,0x04,0x07,0x27,
  0x06,0x24,0xa3,0x02,0x7a,0x01,0x65,0x00,0x44,0x00,0x6b,0x00,0x00,0x00,0xaa,0x01,
  0x07,0x28,0x06,0x24,0xf3,0x00,0x1f,0x01,0x77,0x00,0x1d,0x00,0x32,0x00,0x00,0x00,
  0xaa,0x14,0x08,0x29,0x06,0x24,0x18,0x05,0x0d,0x02,0x5b,0x00,0x77,0x00,0x95,0x00,
  0x00,0x00,0xaa,0x51,0x06,0x30,0x06,0x24,0x53,0x03,0x75,0x01,0x66,0x00,0x57,0x00,
  0x88,0x00,0x00,0x00,0xaa,0x04,0x07,0x01,0x07,0x24,0xf4,0x00,0x24,0x01,0x78,0x00,
  0x1d,0x00,0x32,0x00,0x00,0x00,0xaa,0x06,0x07,0x02,0x07,0x24,0xf5,0x00,0x12,0x01,
  0x7d,0x00,0x1f,0x00,0x35,0x00,0x00,0x00,0xaa,0x00,0x20,0x21,0x11,0x23,0x03,0x17,
  0xea,0x00,0x8e,0x00,0x46,0x03,0xe7,0x05,0x00,0x00,0xaa,0x37,0x13,0x09,0x12,0x23,
  0x61,0x0e,0xec,0x00,0x8e,0x00,0x0b,0x02,0xa7,0x03,0x00,0x00,0xaa,0x26,0x10,0x23,
  0x12,0x23,0x7f,0x0e,0x44,0x01,0x74,0x00,0xae,0x01,0xb0,0x02,0x00,0x00,0xaa,0x02,
  0x15,0x03,0x01,0x24,0xcf,0x0c,0x0a,0x01,0x82,0x00,0xaa,0x01,0xe1,0x02,0x00,0x00,
  0xaa,0x28,0x11,0x20,0x01,0x24,0xa2,0x0e,0x31,0x01,0x76,0x00,0xbb,0x01,0xe1,0x02,
  0x00,0x00,0xaa,0x05,0x15,0x28,0x01,0x24,0x31,0x13,0x1e,0x01,0x7c,0x00,0x61,0x02,
  0x05,0x04,0x00,0x00,0xaa,0x06,0x11,0x10,0x02,0x24,0xe1,0x17,0x51,0x01,0x72,0x00,
  0xbb,0x02,0x41,0x04,0x00,0x00,0xaa,0x46,0x14,0x02,0x03,0x24,0x28,0x0e,0x30,0x01,
  0x76,0x00,0xab,0x01,0xcb,0x02,0x00,0x00,0xaa,0x03,0x09,0x21,0x03,0x24,0xae,0x1c,
  0x23,0x01,0x7c,0x00,0x8b,0x03,0xec,0x05,0x00,0x00,0xaa,0x48,0x15,0x13,0x04,0x24,
  0x55,0x1a,0x15,0x01,0x7e,0x00,0x54,0x03,0xb2,0x05,0x00,0x00,0xaa,0x57,0x11,0x09,
  0x05,0x24,0x92,0x12,0x31,0x01,0x75,0x00,0x2c,0x02,0xa6,0x03,0x00,0x00,0xaa,0x37,
  0x12,0x25,0x05,0x24,0x01,0x27,0x40,0x01,0x72,0x00,0x71,0x04,0x51,0x07,0x00,0x00,
  0xaa,0x41,0x20,0x01,0x01,0x24,0x2a,0x25,0x1c,0x01,0x7c,0x00,0x99,0x04,0xdc,0x07,
  0x00,0x00,0xaa,0x02,0x07,0x01,0x02,0x24,0x90,0x23,0x3f,0x01,0x76,0x00,0x30,0x04,
  0xb0,0x06,0x00,0x00,0xaa,0x08,0x07,0x01,0x03,0x24,0x23,0x1f,0x33,0x01,0x77,0x00,
  0xb3,0x03,0x13,0x06,0x00,0x00,0xaa,0x06,0x07,0x01,0x04,0x24,0x25,0x1f,0x10,0x01,
  0x80,0x00,0xfe,0x03,0xdc,0x06,0x00,0x00,0xaa,0x49,0x09,0x01,0x05,0x24,0x9f,0x23,
  0x2e,0x01,0x76,0x00,0x33,0x04,0x14,0x07,0x00,0x00,0xaa,0x00,0x09,0x01,0x06,0x24,
  0xa5,0x2e,0x54,0x01,0x6e,0x00,0x20,0x05,0x3b,0x08,0x00,0x00,0xaa,0x59,0x11,0x19,
  0x07,0x22,0x00,0x3c,0xf6,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

#endif

#endif	/* EEPROM_JOURNAL_H */
