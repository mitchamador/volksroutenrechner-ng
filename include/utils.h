#ifndef UTILS_H
#define	UTILS_H

#include "hw.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    FORWARD,
    BACKWARD,
} dir_t;

unsigned char bcd8_to_bin(unsigned char);
unsigned char bin8_to_bcd(unsigned char);
void bcd8_to_str(char*, unsigned char);
unsigned char bcd8_incdec(unsigned char, dir_t, unsigned char, unsigned char);
unsigned char bcd8_inc(unsigned char, unsigned char, unsigned char);
signed char bcd_subtract(unsigned char, unsigned char);

unsigned char strcpy2(char *, char *, unsigned char);

unsigned long strtoul2(char *);
unsigned char ultoa2(char *, unsigned long, unsigned char);
void add_leading_symbols(char*, char, unsigned char, unsigned char);
void str_center16(char *, unsigned char);

#endif	/* UTILS_H */

