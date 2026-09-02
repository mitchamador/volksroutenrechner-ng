#include "core.h"
#include "lcd.h"
#include "i2c.h"

#if defined(LCD_1602) || defined(LCD_1602_I2C)

#if defined(LCD_BUFFERED)
void LCD_write_buffer(char *src, uint8_t len);
#else
#define LCD_write_buffer(src, len) LCD_write_string(src, len, len, ALIGN_NONE)
#endif

#ifdef LCD_CUSTOM_CHARS_PROGRAM_MEMORY
// custom lcd characters
PROGMEM const char custom_symbols[64] = {
    0x05,0x06,0x05,0x00,0x00,0x01,0x02,0x00, // 0x00 - kmh[0]
    0x0A,0x15,0x15,0x00,0x15,0x07,0x01,0x00, // 0x01 - kmh[1]
    0x07,0x05,0x07,0x00,0x01,0x00,0x00,0x00, // 0x02 - omin[0]
    0x00,0x00,0x08,0x10,0x0A,0x15,0x15,0x00, // 0x03 - omin[1]
    0x0C,0x14,0x15,0x02,0x05,0x01,0x01,0x00, // 0x04 - L100[0]
    0x00,0x00,0x00,0x00,0x1F,0x15,0x1F,0x00, // 0x05 - L100[1]
    0x03,0x05,0x05,0x00,0x01,0x00,0x00,0x00, // 0x06 - l/h[0]
    0x00,0x00,0x08,0x10,0x0A,0x0E,0x02,0x00, // 0x07 - l/h[1]
};
#endif

#ifdef LCD_1602
void _LCD_write_4bit(unsigned char data) {
    HW_lcd_set_data(data);
    HW_lcd_en_high();
    LCD_delay_en_strobe();
    HW_lcd_en_low();
}
#endif

#if defined(LCD_1602_I2C)
void _LCD_write_4bit_I2C(unsigned char data, unsigned char mode) {
    I2C_Master_Start(LCD_I2C_ADDRESS);
    I2C_Master_Write(((data & 0xF0) | EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();

    I2C_Master_Start(LCD_I2C_ADDRESS);
    I2C_Master_Write(((data & 0xF0) & ~EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();
}
#endif

#if defined(LCD_1602) && defined(LCD_1602_I2C)
#define LCD_write_4bit(data, mode) if (use_lcd_1602_i2c() == 0) { _LCD_write_4bit(data); } else { _LCD_write_4bit_I2C(data, mode); };
#elif defined(LCD_1602)
#define LCD_write_4bit(data, mode) _LCD_write_4bit(data);
#elif defined(LCD_1602_I2C)
#define LCD_write_4bit(data, mode) _LCD_write_4bit_I2C(data, mode);
#endif

void LCD_init(void) {

    uint8_t i = 0;

#if defined(LCD_1602) && defined(LCD_1602_I2C)
    if (use_lcd_1602_i2c() == 0)
#endif
#if defined(LCD_1602)
    {
      HW_lcd_rs_high();
      HW_lcd_en_high();
    }
#endif
#if defined(LCD_1602) && defined(LCD_1602_I2C)
    else
#endif
#if defined(LCD_1602_I2C)
    {
      if (I2C_Master_Start(LCD_I2C_ADDRESS) == ACK) { // Initialize LCD module with I2C address = 0x4E ((0x27<<1) for PCF8574) or 0x7E ((0x3F<<1) for PCF8574A)
        I2C_Master_Write((RS | EN) | LCD_BACKLIGHT);
      }
      I2C_Master_Stop();
    }
#endif

    HW_delay_ms(50);

    // 5x 0x00 with 4100us delay (oled reset)
    // 0x30 (4100us delay), 0x30 (100us), 0x30 (100us) (default reset)
    for (uint8_t i = 8; i > 0; i--) {
       LCD_write_4bit(i > 3 ? 0x00 : 0x30, 0);
       if (i > 2) {
           HW_delay_us(4100);
       } else {
           HW_delay_us(100);
       }
    }

    LCD_command(LCD_RETURN_HOME);
    LCD_command(LCD_FUNCTION_SET | (LCD_TYPE << 2));

    LCD_command(LCD_TURN_OFF);
    LCD_command(0x17);

    LCD_clear();
    LCD_command(LCD_TURN_ON);
    LCD_command(LCD_ENTRY_MODE_SET | LCD_INCREMENT | LCD_NOSHIFT);

    // LCD set custom characters
    char tbuf[8];
#ifdef LCD_CUSTOM_CHARS_PROGRAM_MEMORY
    char *src = (char *) &custom_symbols;
#endif

    for (i = 0; i < 64; i += 8) {
        LCD_command(LCD_SETCGRAMADDR | (i & ~0x07));
#ifndef LCD_CUSTOM_CHARS_PROGRAM_MEMORY
        HW_read_eeprom_block((unsigned char*) tbuf, EEPROM_CUSTOM_CHARS_ADDRESS + i, 8);
#else
        char *dst = (char *) &tbuf;
        uint8_t c = 8 + 1;
        while(--c) *dst++ = pgm_read_byte(src++);
#endif
        LCD_write_buffer(tbuf, 8);
    }
}

void LCD_clear(void) {
#if defined(LCD_BUFFERED)
    _memset(lcd_buf, ' ', LCD_WIDTH * 2);
#endif
#if defined(LCD_1602) || defined(LCD_1602_I2C)
    LCD_command(LCD_CLEAR);
    HW_delay_us(LCD_DELAY_CLEAR);
#endif
}

void LCD_off() {
    LCD_clear();
}

void LCD_command(char data) {
#if defined(LCD_1602)
    HW_lcd_rs_low();
#endif    
    LCD_write_4bit(data, 0);
    LCD_delay_4bits();
    LCD_write_4bit((unsigned char) (data << 4), 0);
    LCD_check_busy();
}

#if defined(LCD_BUFFERED)

void LCD_write_buffer(char *src, uint8_t len) {
    len++;
    while (--len != 0) {
        uint8_t ch = *src++;
        HW_lcd_rs_high();
        LCD_write_4bit(ch, RS);
        LCD_delay_4bits();
        LCD_write_4bit((unsigned char) (ch << 4), RS);
        LCD_check_busy();
    }
}

void LCD_flush_buffer() {
    LCD_command(LCD_CURSOR_OFF);

    LCD_command(LCD_FIRST_ROW);
    LCD_write_buffer(&lcd_buf[LCD_CURSOR_POS_00], LCD_WIDTH);
    LCD_command(LCD_SECOND_ROW);
    LCD_write_buffer(&lcd_buf[LCD_CURSOR_POS_10], LCD_WIDTH);

    if (lcd_cursor.mode != LCD_CURSOR_OFF) {
        // convert buffered cursor position to real position
        uint8_t pos;
        if (lcd_cursor.pos >= LCD_WIDTH) {
            pos = LCD_SECOND_ROW - LCD_WIDTH + lcd_cursor.pos;
        } else {
            pos = LCD_FIRST_ROW + lcd_cursor.pos;
        }
        LCD_command(pos);
        LCD_command(lcd_cursor.mode);
    }
}

#else

void LCD_write_string(char* str, unsigned char len, unsigned char max, align_t align) {
    if (len > max) len = max;

    unsigned char p_lower = max - len;
    if (align == ALIGN_LEFT) {
        p_lower = 0;
    } else if (align == ALIGN_CENTER) {
        p_lower >>= 1;
    };
    unsigned char p_upper = p_lower + len;

    for (unsigned char i = 0; i < max; i++) {
        unsigned char ch;
        if (i < p_lower || i >= p_upper) {
            ch = ' ';
        } else {
            ch = *str++;
        }
#if defined(LCD_1602)
        HW_lcd_rs_high();
#endif
        LCD_write_4bit(ch, RS);
        LCD_delay_4bits();
        LCD_write_4bit((unsigned char) (ch << 4), RS);
        LCD_check_busy();
    }
}

#endif

#endif