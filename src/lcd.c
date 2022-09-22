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
    RS_HIGH;
    EN_HIGH;
#else
    if (I2C_Master_Start(LCD_I2C_ADDRESS) == ACK) { // Initialize LCD module with I2C address = 0x4E ((0x27<<1) for PCF8574) or 0x7E ((0x3F<<1) for PCF8574A)
        I2C_Master_Write((RS | EN) | LCD_BACKLIGHT);
    } else {
        lcd_i2c_skip = 1;
    }
    I2C_Master_Stop();
#endif

    delay_ms(50);

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
           delay_us(4100);
       } else {
           delay_us(100);
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
        LCD_Write_String(tbuf, 8, 8, ALIGN_NONE);
    }

}

#ifdef LCD_LEGACY
void LCD_Write_4Bit(unsigned char data) {
    LCD_DATA(data);
    EN_HIGH;
    LCD_delay_en_strobe();
    EN_LOW;
}

void LCD_CMD(char data) {
    RS_LOW;
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

void LCD_Write_String(char* str, unsigned char len, unsigned char max, align_t align) {
    //if (align == ALIGN_NONE) return;
    
    unsigned char i;
    unsigned char ch;

    unsigned char p_lower = max - len, p_upper = max;
    if (align == ALIGN_LEFT) {
        p_lower = 0;
    } else if (align == ALIGN_CENTER) {
        p_lower >>= 1;
    };
    p_upper = p_lower + len;
    for (i = 0; i < max; i++) {
        if (i < p_lower || i >= p_upper) {
            ch = ' ';
        } else {
            ch = *str++;
        }
#ifdef LCD_LEGACY
        RS_HIGH;
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

void LCD_Clear(void) {
  LCD_CMD(LCD_CLEAR);
  delay_us(LCD_DELAY_CLEAR);
}
