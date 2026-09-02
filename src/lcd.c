#include "core.h"
#include "lcd.h"

#if defined(LCD_BUFFERED)

char lcd_buf[LCD_WIDTH * 2];
lcd_cursor_t lcd_cursor = {LCD_CURSOR_OFF, 0};

void LCD_cursor_set_state(uint8_t mode, uint8_t pos) {
    lcd_cursor.mode = mode;
    lcd_cursor.pos = pos;
}

void LCD_write_string(char* buf, unsigned char len, unsigned char max, align_t align) {
    if (len > max) len = max;

    unsigned char p_lower = max - len;
    if (align == ALIGN_LEFT) {
        p_lower = 0;
    } else if (align == ALIGN_CENTER) {
        p_lower >>= 1;
    };
    unsigned char p_upper = p_lower + len;
    
    char *dst = (char *) &lcd_buf[lcd_cursor.pos + max];
    char *src = (char *) &buf[len];

    while (max-- > 0) {
        if (max < p_lower || max >= p_upper) {
            *--dst = ' ';
        } else {
            *--dst = *--src;
        }
    }
}

#endif

