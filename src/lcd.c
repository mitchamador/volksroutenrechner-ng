#include "core.h"
#include "lcd.h"

#if defined(LCD_BUFFERED)

char lcd_buf[LCD_WIDTH * 2];
lcd_cursor_t lcd_cursor = {LCD_CURSOR_OFF, 0};
void LCD_Write_Buffer(char *src, uint8_t len);

void LCD_cursor_set_state(uint8_t mode, uint8_t pos) {
    lcd_cursor.mode = mode;
    lcd_cursor.pos = pos;
}

void LCD_Write_String(char* buf, unsigned char len, unsigned char max, align_t align) {
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

#else

#define LCD_Write_Buffer(src, len) LCD_Write_String(src, len, len, ALIGN_NONE)

#endif

#if defined(LCD_1602) || defined(LCD_1602_I2C)

#define LCD_FIRST_ROW          0x80
#define LCD_SECOND_ROW         0xC0
#define LCD_CLEAR              0x01
#define LCD_THIRD_ROW          0x94
#define LCD_FOURTH_ROW         0xD4
#define LCD_RETURN_HOME        0x02
#define LCD_ENTRY_MODE_SET     0x04
#define LCD_MOVE_CURSOR_LEFT   0x10
#define LCD_MOVE_CURSOR_RIGHT  0x14
#define LCD_TURN_ON            0x0C
#define LCD_TURN_OFF           0x08
#define LCD_SHIFT_LEFT         0x18
#define LCD_SHIFT_RIGHT        0x1E
#define LCD_SETCGRAMADDR       0x40
#define LCD_INCREMENT          0x02
#define LCD_DECREMENT          0x00
#define LCD_SHIFT              0x01
#define LCD_NOSHIFT            0x00
#define LCD_FUNCTION_SET       0x20
#define LCD_TYPE               2       // 0 -> 5x7 | 1 -> 5x10 | 2 -> 2 lines

#endif

void LCD_Clear(void) {
#if defined(LCD_BUFFERED)
    _memset(lcd_buf, ' ', LCD_WIDTH * 2);
#endif
#if defined(LCD_1602) || defined(LCD_1602_I2C)
    LCD_CMD(LCD_CLEAR);
    HW_delay_us(LCD_DELAY_CLEAR);
#endif
}

#if defined(LCD_1602) || defined(LCD_1602_I2C)

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
void _LCD_Write_4Bit(unsigned char data) {
    HW_lcd_set_data(data);
    HW_lcd_en_high();
    LCD_delay_en_strobe();
    HW_lcd_en_low();
}
#endif

#if defined(LCD_1602_I2C)
void _LCD_Write_4Bit_I2C(unsigned char data, unsigned char mode) {
    I2C_Master_Start(LCD_I2C_ADDRESS);
    I2C_Master_Write(((data & 0xF0) | EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();

    I2C_Master_Start(LCD_I2C_ADDRESS);
    I2C_Master_Write(((data & 0xF0) & ~EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();
}
#endif

#if defined(LCD_1602) && defined(LCD_1602_I2C)
#define LCD_Write_4Bit(data, mode) if (use_lcd_1602_i2c() == 0) { _LCD_Write_4Bit(data); } else { _LCD_Write_4Bit_I2C(data, mode); };
#elif defined(LCD_1602)
#define LCD_Write_4Bit(data, mode) _LCD_Write_4Bit(data);
#elif defined(LCD_1602_I2C)
#define LCD_Write_4Bit(data, mode) _LCD_Write_4Bit_I2C(data, mode);
#endif

void LCD_Init(void) {

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
       LCD_Write_4Bit(i > 3 ? 0x00 : 0x30, 0);
       if (i > 2) {
           HW_delay_us(4100);
       } else {
           HW_delay_us(100);
       }
    }

    LCD_CMD(LCD_RETURN_HOME);
    LCD_CMD(LCD_FUNCTION_SET | (LCD_TYPE << 2));

    LCD_CMD(LCD_TURN_OFF);
    LCD_CMD(0x17);

    LCD_Clear();
    LCD_CMD(LCD_TURN_ON);
    LCD_CMD(LCD_ENTRY_MODE_SET | LCD_INCREMENT | LCD_NOSHIFT);

    // LCD set custom characters
    char tbuf[8];
#ifdef LCD_CUSTOM_CHARS_PROGRAM_MEMORY
    char *src = (char *) &custom_symbols;
#endif

    for (i = 0; i < 64; i += 8) {
        LCD_CMD(LCD_SETCGRAMADDR | (i & ~0x07));
#ifndef LCD_CUSTOM_CHARS_PROGRAM_MEMORY
        HW_read_eeprom_block((unsigned char*) tbuf, EEPROM_CUSTOM_CHARS_ADDRESS + i, 8);
#else
        char *dst = (char *) &tbuf;
        uint8_t c = 8 + 1;
        while(--c) *dst++ = pgm_read_byte(src++);
#endif
        LCD_Write_Buffer(tbuf, 8);
    }
}

void LCD_CMD(char data) {
#if defined(LCD_1602)
    HW_lcd_rs_low();
#endif    
    LCD_Write_4Bit(data, 0);
    LCD_delay_4bits();
    LCD_Write_4Bit((unsigned char) (data << 4), 0);
    LCD_Check_Busy();
}

#if defined(LCD_BUFFERED)

void LCD_Write_Buffer(char *src, uint8_t len) {
    len++;
    while (--len != 0) {
        uint8_t ch = *src++;
        HW_lcd_rs_high();
        LCD_Write_4Bit(ch, RS);
        LCD_delay_4bits();
        LCD_Write_4Bit((unsigned char) (ch << 4), RS);
        LCD_Check_Busy();
    }
}

void LCD_flush_buffer() {
    LCD_CMD(LCD_CURSOR_OFF);

    LCD_CMD(LCD_FIRST_ROW);
    LCD_Write_Buffer(&lcd_buf[LCD_CURSOR_POS_00], LCD_WIDTH);
    LCD_CMD(LCD_SECOND_ROW);
    LCD_Write_Buffer(&lcd_buf[LCD_CURSOR_POS_10], LCD_WIDTH);

    if (lcd_cursor.mode != LCD_CURSOR_OFF) {
        // convert buffered cursor position to real position
        uint8_t pos;
        if (lcd_cursor.pos >= LCD_WIDTH) {
            pos = LCD_SECOND_ROW - LCD_WIDTH + lcd_cursor.pos;
        } else {
            pos = LCD_FIRST_ROW + lcd_cursor.pos;
        }
        LCD_CMD(pos);
        LCD_CMD(lcd_cursor.mode);
    }
}

#else

void LCD_Write_String(char* str, unsigned char len, unsigned char max, align_t align) {
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
        LCD_Write_4Bit(ch, RS);
        LCD_delay_4bits();
        LCD_Write_4Bit((unsigned char) (ch << 4), RS);
        LCD_Check_Busy();
    }
}

#endif

#endif