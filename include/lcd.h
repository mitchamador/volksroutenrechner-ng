#ifndef _LCD_H
#define _LCD_H

#include "utils.h"

#if defined(LCD_SSD1322_1602)
#include "lcd_ssd1322.h"
#elif defined(LCD_1602) || defined(LCD_1602_I2C)
#include "lcd_hd44780.h"
#endif

// width
#define LCD_WIDTH              16

void LCD_init(void);
void LCD_clear(void);
void LCD_off(void);
#if defined(LCD_BUFFERED)
void LCD_flush_buffer(void);
#endif
#if defined(LCD_CONTRAST_CONFIG)
void LCD_set_contrast(uint8_t contrast);
#endif

void LCD_write_string(char*, unsigned char, unsigned char, align_t);

#define LCD_CURSOR_OFF         0x0C
#define LCD_UNDERLINE_ON       0x0E
#define LCD_BLINK_CURSOR_ON    0x0F

// print to buffer only
#define LCD_CURSOR_POS_NONE    0xFF

#if defined(LCD_BUFFERED)

extern char lcd_buf[LCD_WIDTH * 2];

typedef struct {
    uint8_t mode;
    uint8_t pos;
} lcd_cursor_t;

extern lcd_cursor_t lcd_cursor;

// define cursor position for half width
// 00           01
// 10           11

#define LCD_CURSOR_POS_00       0
#define LCD_CURSOR_POS_01       (LCD_WIDTH/2)
#define LCD_CURSOR_POS_10       (LCD_WIDTH)
#define LCD_CURSOR_POS_11       (LCD_WIDTH + LCD_WIDTH/2)

#define LCD_cursor_off() LCD_cursor_set_state(LCD_CURSOR_OFF, 0);
#define LCD_cursor_set_position(pos) LCD_cursor_set_state(LCD_CURSOR_OFF, pos);
#define LCD_cursor_blink(pos) { LCD_cursor_set_state(LCD_BLINK_CURSOR_ON, pos); }
#define LCD_cursor_underline(pos)  { LCD_cursor_set_state(LCD_UNDERLINE_ON, pos); }

void LCD_cursor_set_state(uint8_t mode, uint8_t pos);

#endif

#endif
