#ifndef _LCD_H
#define _LCD_H

#include "i2c.h"
#include "utils.h"

#if defined(LCD_SSD1322_1602)
#define LCD_BUFFERED
#endif

#define LCD_CURSOR_OFF         0x0C
#define LCD_UNDERLINE_ON       0x0E
#define LCD_BLINK_CURSOR_ON    0x0F

// width
#define LCD_WIDTH               16

// print to buffer only
#define LCD_CURSOR_POS_NONE     0xFF

#if !defined(LCD_SSD1322_1602)

// max delays for ~190kHz
#define LCD_DELAY_CMD 53
#define LCD_DELAY_CLEAR 2160

#define LCD_Check_Busy() HW_delay_us(LCD_DELAY_CMD)

// delay en strobe delay
#define LCD_delay_en_strobe()
// delay between 4 bits
#define LCD_delay_4bits()

void LCD_CMD(char);

#if defined(LCD_LEGACY)
#define RS            0
#else
#define LCD_I2C_ADDRESS 0x4E
#define RS            (1 << 0)
#define RW            (1 << 1)
#define EN            (1 << 2)
#define LCD_BACKLIGHT (1 << 3)
#endif

#endif

void LCD_Init(void);
void LCD_Clear(void);
void LCD_Write_String(char*, unsigned char, unsigned char, align_t);

#if !defined(LCD_BUFFERED)

// define cursor position for half width
// 00           01
// 10           11

#define LCD_CURSOR_POS_00       0x80
#define LCD_CURSOR_POS_01       0x88
#define LCD_CURSOR_POS_10       0xC0
#define LCD_CURSOR_POS_11       0xC8

#define LCD_cursor_off() LCD_CMD(LCD_CURSOR_OFF);
#define LCD_cursor_set_position(pos) LCD_CMD(pos);
#define LCD_cursor_blink(pos) { LCD_CMD(pos); LCD_CMD(LCD_BLINK_CURSOR_ON); }
#define LCD_cursor_underline(pos)  { LCD_CMD(pos); LCD_CMD(LCD_UNDERLINE_ON); }

#define LCD_flush_buffer()

#else

extern char lcd_buf[LCD_WIDTH * 2];

typedef struct {
    uint8_t mode;
    uint8_t pos;
} lcd_cursor_t;

extern lcd_cursor_t lcd_cursor;

void LCD_Write_Buffer(char *src, uint8_t len);

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

void LCD_flush_buffer(void);
void LCD_cursor_set_state(uint8_t mode, uint8_t pos);

#endif

#endif
