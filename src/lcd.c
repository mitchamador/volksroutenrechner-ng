#include "core.h"
#include "lcd.h"

#ifdef LCD_LEGACY
void LCD_Write_4Bit(unsigned char data);
#else
void LCD_Write_4Bit(unsigned char Nibble, unsigned char mode);
#endif

#ifndef LCD_LEGACY
    unsigned char lcd_i2c_skip = 0;
#endif

void LCD_Init(void) {

    unsigned char i = 0;

#ifdef LCD_LEGACY
    HW_lcd_rs_high();
    HW_lcd_en_high();
#else
    if (I2C_Master_Start(LCD_I2C_ADDRESS) == ACK) { // Initialize LCD module with I2C address = 0x4E ((0x27<<1) for PCF8574) or 0x7E ((0x3F<<1) for PCF8574A)
        I2C_Master_Write((RS | EN) | LCD_BACKLIGHT);
    } else {
        lcd_i2c_skip = 1;
    }
    I2C_Master_Stop();
#endif

    HW_delay_ms(50);

#ifdef NO_LCD_OLED_RESET
    // 0x30 (4100us delay), 0x30 (100us), 0x30 (100us)
    for (i = 3; i > 0; i--) {
#ifdef LCD_LEGACY
        LCD_Write_4Bit(0x30);
#else
        LCD_Write_4Bit(0x30, 0);
#endif
#else
    // 5x 0x00 with 4100us delay (oled reset) + as above
    for (uint8_t i = 8; i > 0; i--) {
#ifdef LCD_LEGACY
        LCD_Write_4Bit(i > 3 ? 0x00 : 0x30);
#else
        LCD_Write_4Bit(i > 3 ? 0x00 : 0x30, 0);
#endif
#endif
       if (i > 2) {
           HW_delay_us(4100);
       } else {
           HW_delay_us(100);
       }
    }


    LCD_CMD(LCD_RETURN_HOME);
    LCD_CMD(LCD_FUNCTION_SET | (LCD_TYPE << 2));

#ifndef NO_LCD_OLED_RESET
    LCD_CMD(LCD_TURN_OFF);
    LCD_CMD(0x17);
#endif

    LCD_Clear();
    LCD_CMD(LCD_TURN_ON);
    LCD_CMD(LCD_ENTRY_MODE_SET | LCD_INCREMENT | LCD_NOSHIFT);

    // LCD set custom characters
    char tbuf[8];

    for (i = 0; i < 64; i = i + 8) {
        LCD_CMD(LCD_SETCGRAMADDR | (i & ~0x07));
        HW_read_eeprom_block((unsigned char*) tbuf, EEPROM_CUSTOM_CHARS_ADDRESS + i, 8);
#if defined(LCD_BUFFERED)
        LCD_Write_Buffer(tbuf, 8);
#else
        LCD_Write_String(tbuf, 8, 8, ALIGN_NONE);
#endif
    }

}

#ifdef LCD_LEGACY
void LCD_Write_4Bit(unsigned char data) {
    HW_lcd_set_data(data);
    HW_lcd_en_high();
    LCD_delay_en_strobe();
    HW_lcd_en_low();
}

void LCD_CMD(char data) {
    HW_lcd_rs_low();
    LCD_Write_4Bit(data);
    LCD_delay_4bits();
    LCD_Write_4Bit((unsigned char) (data << 4));
    LCD_Check_Busy();
}

#else

void LCD_Write_4Bit(unsigned char Nibble, unsigned char mode) {
    if (lcd_i2c_skip != 0) return;

    I2C_Master_Start(LCD_I2C_ADDRESS);
    I2C_Master_Write((Nibble | EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();

    I2C_Master_Start(LCD_I2C_ADDRESS);
    I2C_Master_Write((Nibble & ~EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();
}

void LCD_CMD(char CMD) {
    LCD_Write_4Bit(CMD & 0xF0, 0);
    LCD_Write_4Bit((CMD << 4) & 0xF0, 0);
    LCD_Check_Busy();
}

#endif

#if defined(LCD_BUFFERED)
char lcd_buf[LCD_WIDTH * 2];

typedef struct {
    uint8_t mode;
    uint8_t pos;
} lcd_cursor_t;

lcd_cursor_t lcd_cursor = {LCD_CURSOR_OFF};
uint8_t lcd_cursor_position;
#endif

void LCD_Clear(void) {
#if defined(LCD_BUFFERED)
    _memset(lcd_buf, ' ', LCD_WIDTH * 2);
#endif    
    LCD_CMD(LCD_CLEAR);
    HW_delay_us(LCD_DELAY_CLEAR);
}

#if defined(LCD_BUFFERED)

void LCD_Write_Buffer(char *src, uint8_t len) {
    len++;
    while (--len != 0) {
        uint8_t ch = *src++;
#ifdef LCD_LEGACY
        HW_lcd_rs_high();
        LCD_Write_4Bit(ch);
        LCD_delay_4bits();
        LCD_Write_4Bit((unsigned char) (ch << 4));
        LCD_Check_Busy();
#else
        LCD_Write_4Bit(ch & 0xF0, RS);
        LCD_Write_4Bit((ch << 4) & 0xF0, RS);
        LCD_Check_Busy();
#endif
    }
}

void LCD_flush_buffer() {
    LCD_CMD(LCD_CURSOR_OFF);

    LCD_CMD(LCD_FIRST_ROW);
    LCD_Write_Buffer(&lcd_buf[LCD_CURSOR_POS_00], LCD_WIDTH);
    LCD_CMD(LCD_SECOND_ROW);
    LCD_Write_Buffer(&lcd_buf[LCD_CURSOR_POS_10], LCD_WIDTH);

    if (lcd_cursor.mode != LCD_CURSOR_OFF) {
        LCD_CMD(lcd_cursor.pos);
        LCD_CMD(lcd_cursor.mode);
    }
}

void LCD_cursor_set_state(uint8_t mode, uint8_t pos) {
    lcd_cursor.mode = mode;
    // convert buffered cursor position to real position
    if (pos >= LCD_WIDTH) {
        lcd_cursor.pos = LCD_SECOND_ROW - LCD_WIDTH + pos;
    } else {
        lcd_cursor.pos = LCD_FIRST_ROW + pos;
    }
}

void LCD_cursor_set_position(uint8_t _pos) {
    lcd_cursor_position = _pos;
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
    
    char *dst = (char *) &lcd_buf[lcd_cursor_position + max];
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
#ifdef LCD_LEGACY
        HW_lcd_rs_high();
        LCD_Write_4Bit(ch);
        LCD_delay_4bits();
        LCD_Write_4Bit((unsigned char) (ch << 4));
        LCD_Check_Busy();
#else
        LCD_Write_4Bit(ch & 0xF0, RS);
        LCD_Write_4Bit((ch << 4) & 0xF0, RS);
        LCD_Check_Busy();
#endif
    }
}

#endif