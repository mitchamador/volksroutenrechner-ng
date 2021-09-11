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
    // use lwmod and lwdiv instead of byte functions (gives some size optimizations as lwmod and lwdiv already used)
    unsigned short _b = b;
    return (unsigned char) ((_b / 10) << 4) + (_b % 10);
}

void bcd8_to_str(char* buf, unsigned char b) {
    buf[1] = (b & 0x0F) + '0';
    buf[0] = ((b >> 4) & 0x0F) + '0';
}

unsigned char bcd8_incdec(unsigned char bcd, dir_t dir, unsigned char min, unsigned char max) {
    unsigned char _tmp = bcd8_to_bin(bcd);
    if (dir == BACKWARD) {
        if (_tmp++ >= max) {
            _tmp = min;
        }
    } else {
        if (_tmp-- <= min) {
            _tmp = max;
        }
    }
    return bin8_to_bcd(_tmp);
}

unsigned char bcd8_inc(unsigned char bcd, unsigned char min, unsigned char max) {
    unsigned char _tmp = bcd8_to_bin(bcd);
    if (_tmp++ >= max) {
        return min;
    }
    return bin8_to_bcd(_tmp);
}

signed char bcd_subtract(unsigned char a, unsigned char b) {
    return (signed char) bcd8_to_bin(a) - (signed char) bcd8_to_bin(b);
}

char * _ultoa(char * buf, unsigned long val, unsigned int b) {
	unsigned	long	v;
	char		c;

	v = val;
	do {
		v /= b;
		buf++;
	} while(v != 0);
	*buf-- = 0;
	do {
		c = (unsigned char) (val % b);
		val /= b;
		if(c >= 10)
			c += 'A'-'0'-10;
		c += '0';
		*buf-- = c;
	} while(val != 0);
	return buf;
}

unsigned long strtoul2(char * buf) {
    //return strtoul(buf, NULL, 10);
    unsigned long val = 0;
    while (*buf) {
        val = val * 10 + (*buf++ - '0');
    }
    return val;
}

unsigned char ultoa2(char * buf, unsigned long val, unsigned char b) {
    _ultoa(buf, val, b);
    unsigned char _len = 0;
    while(buf[++_len] != 0);
    return _len;
}

void add_leading_symbols(char* buf, char s, unsigned char len, unsigned char max_len) {
    // right align symbols
    while (len > 0) {
        buf[--max_len] = buf[--len];
    }
    // add leading symbols
    while (max_len > 0) {
        buf[--max_len] = s;
    }
}

unsigned char strcpy2(char* buf, char* str, unsigned char pos) {
    unsigned char divider = pos == 0 ? 0x00 : pgm_read_byte(str);

    while (pos > 0) {
        if (pgm_read_byte(str++) == divider) {
            pos--;
        }
    }
    while (pgm_read_byte(str) != divider) buf[pos++] = pgm_read_byte(str++);
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
