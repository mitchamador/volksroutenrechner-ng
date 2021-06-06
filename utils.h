/* 
 * File:   utils.h
 * Author: victor
 *
 * Created on 31 мая 2021 г., 10:51
 */

#ifndef UTILS_H
#define	UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

unsigned char bcd8_to_bin(unsigned char);
unsigned char bin8_to_bcd(unsigned char);
void bcd8_to_str(char*, unsigned char);
unsigned char bcd8_inc(unsigned char, unsigned char);
signed char bcd_subtract(unsigned char, unsigned char);

unsigned char strcpy2(char *, const char *, unsigned char);
unsigned char ultoa2(char *, unsigned long);
unsigned char utoa2(char *, unsigned short, unsigned char);
void add_leading_symbols(char*, char, unsigned char, unsigned char);

#endif	/* UTILS_H */

