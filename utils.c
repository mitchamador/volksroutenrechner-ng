#include <xc.h>
#include "utils.h"

unsigned char bcd8_to_bin(unsigned char b) {
    unsigned char _t = b & 0x0F;
    b = (b & 0xF0) >> 1;
    _t += b;
    b = b >> 2;
    return _t + b;
    //return ((b >> 1) & 0x78) + ((b >> 3) & 0x1E) + (b & 0x0F);
    //return ((b >> 4) & 0x0F) * 10 + (b & 0x0F);
}

unsigned char bin8_to_bcd(unsigned char b) {
    return (unsigned char) ((b / 10) << 4) + (b % 10);
}

void bcd8_to_str(char* buf, unsigned char b) {
    buf[1] = (b & 0x0F) + '0';
    buf[0] = ((b >> 4) & 0x0F) + '0';
}

unsigned char bcd8_inc(unsigned char bcd, unsigned char max) {
    unsigned char _tmp = bcd8_to_bin(bcd);
    if (_tmp++ >= max) {
        return 0;
    }
    return bin8_to_bcd(_tmp);
}

signed char bcd_subtract(unsigned char a, unsigned char b) {
    return (signed char) bcd8_to_bin(a) - (signed char) bcd8_to_bin(b);
}

#if defined(_16F1936)
char * utoa(char * buf, unsigned val, int b)
{
	unsigned	v;
	char		c;

	v = val;
	do {
		v /= b;
		buf++;
	} while(v != 0);
	*buf-- = 0;
	do {
		c = val % b;
		val /= b;
		if(c >= 10)
			c += 'A'-'0'-10;
		c += '0';
		*buf-- = c;
	} while(val != 0);
	return ++buf;
}

char * ultoa(char * buf, unsigned long val, int b) {
	unsigned	long	v;
	char		c;

	v = val;
	do {
		v /= b;
		buf++;
	} while(v != 0);
	*buf-- = 0;
	do {
		c = val % b;
		val /= b;
		if(c >= 10)
			c += 'A'-'0'-10;
		c += '0';
		*buf-- = c;
	} while(val != 0);
	return buf;
}
#endif

unsigned char ultoa2(char * buf, unsigned long val) {
    ultoa(buf, val, 10);
    unsigned char _len = 0;
    while(buf[++_len] != 0);
    return _len;
}

unsigned char utoa2(char * buf, unsigned short val, unsigned char b) {
    utoa(buf, val, b);
    unsigned char _len = 0;
    while(buf[++_len] != 0);
    return _len;
}

void add_leading_symbols(char* buf, char s, unsigned char len, unsigned char max_len) {
    unsigned char i;
    // add leading zeroes
    unsigned char diff = max_len - len;
    for (i = len; i != 0; i--) {
        buf[i + diff] = buf[i];
    }
    for (i = 0; i < diff; i++) {
        buf[i] = s;
    }
}

unsigned char strcpy2(char* buf, char* str, unsigned char pos) {
    while (pos > 0) {
        if (*str++ == '\0') {
            pos--;
        }
    }
    while (*str != '\0') buf[pos++] = *str++;
    return pos;
}

void str_center16(char * buf, unsigned char len) {
    unsigned char i;
    unsigned char pos = (16 - len) >> 1;
    for (i = len; i != 0; i--) {
        buf[i + pos] = buf[i];
    }
    for (i = 0; i < pos; i++) {
        buf[i] = ' ';
    }
    for (i = len + pos; i < 16; i++) {
        buf[i] = ' ';
    }
}