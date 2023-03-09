#include "utils.h"

#if defined(__AVR__)
#include <util/crc16.h>
#else
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#endif

uint8_t bcd8_to_bin(uint8_t b) {
    uint8_t _t = b & 0x0F;
    b = (b & 0xF0) >> 1;
    _t += b;
    b = b >> 2;
    return _t + b;
    //return ((b >> 1) & 0x78) + ((b >> 3) & 0x1E) + (b & 0x0F);
    //return ((b >> 4) & 0x0F) * 10 + (b & 0x0F);
}

uint8_t bin8_to_bcd(uint8_t b) {
    // use lwmod and lwdiv instead of byte functions (gives some size optimizations as lwmod and lwdiv already used)
    unsigned short _b = b;
    return (uint8_t) ((_b / 10) << 4) + (_b % 10);
}

void bcd8_to_str(char* buf, uint8_t b) {
    buf[1] = (b & 0x0F) + '0';
    buf[0] = ((b >> 4) & 0x0F) + '0';
}

uint8_t bcd8_inc(uint8_t bcd, uint8_t min, uint8_t max) {
    uint8_t _tmp = bcd8_to_bin(bcd);
    if (_tmp++ >= max) {
        _tmp = min;
    }
    return bin8_to_bcd(_tmp);
}

uint8_t bcd8_dec(uint8_t bcd, uint8_t min, uint8_t max) {
    uint8_t _tmp = bcd8_to_bin(bcd);
    if (_tmp-- <= min) {
        _tmp = max;
    }
    return bin8_to_bcd(_tmp);
}

int8_t bcd_subtract(uint8_t a, uint8_t b) {
    return (int8_t) bcd8_to_bin(a) - (int8_t) bcd8_to_bin(b);
}

uint24_t strtoul2(char * buf) {
    uint24_t val = 0;
    while (*buf) {
        char c = *buf++;
        if (c != '.')
        {
            val = val * 10 + (c - '0');
        }
    }
    return val;
}

uint8_t ultoa2_10(char * buf, uint24_t val) {
    uint8_t _len = 0;

    uint24_t v;
    char c;

    v = val;
    do {
        v /= 10;
        buf++;
        _len++;
    } while (v != 0);
    *buf-- = 0;
    do {
        c = (uint8_t) (val % 10);
        val /= 10;
        c += '0';
        *buf-- = c;
    } while (val != 0);
    
    return _len;
}

uint8_t ultoa2(char * buf, uint24_t val, uint8_t b) {
    uint8_t _len = 0;

    uint24_t v;
    char c;

    v = val;
    do {
        v /= b;
        buf++;
        _len++;
    } while (v != 0);
    *buf-- = 0;
    do {
        c = (uint8_t) (val % b);
        val /= b;
        if (c >= 10)
            c += 'A' - '0' - 10;
        c += '0';
        *buf-- = c;
    } while (val != 0);
    
    return _len;
}

void add_leading_symbols(char* buf, char s, uint8_t len, uint8_t max_len) {
    while (max_len != 0) {
        char c;
        if (len > 0) {
            // right align symbols
            c = buf[--len];
        } else {
            // add leading symbols
            c = s;
        }
        buf[--max_len] = c;
    }
}

uint8_t strcpy2(char* buf, char* str, uint8_t pos) {
    uint8_t divider = pos == 0 ? 0x00 : pgm_read_byte(str);

    while (pos > 0) {
        if (pgm_read_byte(str++) == divider) {
            pos--;
        }
    }
    while (pgm_read_byte(str) != divider) buf[pos++] = pgm_read_byte(str++);
    return pos;
}

// long long ptr to hex string (for printing ds18b20 serial number)
void llptrtohex(uint8_t *sn, uint8_t *p) {
    uint8_t i = 16, t;

    while (--i != 0) {
        t = *sn;
        if ((i & 0x01) != 0) {
            t >>= 4;
        } else {
            sn++;
        }
        t &= 0x0F;
        if (t >= 10) {
            *p++ = 'A' - 10 + t;
        } else {
            *p++ = '0' + t;
        }
    }
}

#if !defined(__AVR)
void * _memset(void * p1, char c, char n)
{
    char * p;

    p = p1;
    while (n--)
        *p++ = c;
    return p1;
}
#endif

void buf_write_string(char* buf, uint8_t len, uint8_t max, align_t align) {
    uint8_t p_lower = max - len, p_upper = max;
    if (align == ALIGN_LEFT) {
        p_lower = 0;
    } else if (align == ALIGN_CENTER) {
        p_lower >>= 1;
    };
    p_upper = p_lower + len;

    while (max-- > 0) {
        if (max < p_lower || max >= p_upper) {
            buf[max] = ' ';
        } else {
            buf[max] = buf[--len];
        }
    }
}

uint8_t crc8_dallas(const uint8_t *addr, uint8_t len) {
    uint8_t crc = 0;
    while (len--) {
#if defined(__AVR__)
        crc = _crc_ibutton_update(crc, *addr++);
#else
        uint8_t inbyte = *addr++;
        for (uint8_t i = 8; i; i--) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
#endif
    }
    return crc;
}
