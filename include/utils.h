#ifndef UTILS_H
#define	UTILS_H

#include "hw.h"
#include "ds1307.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    ALIGN_NONE = 0,
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER
} align_t;

unsigned char bcd8_to_bin(unsigned char);
unsigned char bin8_to_bcd(unsigned char);
void bcd8_to_str(char*, unsigned char);
unsigned char bcd8_inc(unsigned char, unsigned char, unsigned char);
unsigned char bcd8_dec(unsigned char, unsigned char, unsigned char);
signed char bcd_subtract(unsigned char, unsigned char);
unsigned char strcpy2(char *, char *, unsigned char);
unsigned long strtoul2(char *);
unsigned char ultoa2(char *, unsigned long, unsigned char);
void add_leading_symbols(char*, char, unsigned char, unsigned char);
void llptrtohex(unsigned char*, unsigned char*);
void set_day_of_week(ds_time* time);
#if defined(__AVR)
#define _memset(ptr, ch, len) memset(ptr, ch, len)
#else
void * _memset(void *, char, char);
#endif
void buf_write_string(char* buf, unsigned char len, unsigned char max, align_t align);
#endif	/* UTILS_H */

