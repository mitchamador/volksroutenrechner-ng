#ifndef UTILS_H
#define	UTILS_H

#include "core.h"
#include <stdint.h>

#if defined(__AVR)
#include <string.h>
#endif

typedef enum {
    ALIGN_NONE = 0,
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER
} align_t;

uint8_t bcd8_to_bin(uint8_t);
uint8_t bin8_to_bcd(uint8_t);
void bcd8_to_str(char*, uint8_t);
uint8_t bcd8_inc(uint8_t, uint8_t, uint8_t);
uint8_t bcd8_dec(uint8_t, uint8_t, uint8_t);
int8_t bcd_subtract(uint8_t, uint8_t);
uint8_t strcpy2(char *, char *, uint8_t);
uint24_t strtoul2(char *);
uint8_t ultoa2_10(char *, uint24_t);
uint8_t ultoa2(char *, uint24_t, uint8_t);
void add_leading_symbols(char*, char, uint8_t, uint8_t);
void llptrtohex(uint8_t*, uint8_t*);
#if defined(__AVR)
#define _memset(ptr, ch, len) memset(ptr, ch, len)
#else
void * _memset(void *, char, char);
#endif
void buf_write_string(char* buf, uint8_t len, uint8_t max, align_t align);
uint8_t crc8_dallas(const uint8_t *addr, uint8_t len);
#endif	/* UTILS_H */

