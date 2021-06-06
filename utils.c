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

void add_leading_symbols(char* buf, char s, unsigned char v_len, unsigned char max_len) {
    unsigned char i;
    // add leading zeroes
    if (v_len < max_len) {
        unsigned char diff = max_len - v_len;
        for (i = v_len + 1; i > 0; i--) {
            buf[i + diff - 1] = buf[i - 1];
        }
        for (i = 0; i < diff; i++) {
            buf[i] = s;
        }
    }
}

unsigned char strcpy2(char* buf, const char* str, unsigned char pos) {
    while (pos > 0) {
        if (*str == '\0') {
            pos--;
        } else {
            str++;
        }
    }
    while (*str != '\0') buf[pos++] = *str++;
    return pos;
}