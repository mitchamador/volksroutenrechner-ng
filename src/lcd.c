#include "lcd.h"

#ifndef EEPROM_CUSTOM_CHARS
#include "locale.h"
PROGMEM const unsigned char custom_chars[] = {
DATA_KMH_0,    // kmh[0]
DATA_KMH_1,    // kmh[1]
DATA_OMIN_0,   // omin[0]
DATA_OMIN_1,   // omin[1]
DATA_L100_0,   // L100[0]
DATA_L100_1,   // L100[1]
DATA_LH_0,     // l/h[0]
DATA_LH_1,     // l/h[1]
};
#endif

void LCD_Write_4Bit(unsigned char Nibble, unsigned char mode);
void LCD_Set_Cursor(unsigned char ROW, unsigned char COL);
void LCD_Write_Char(char);
void LCD_Write_String(char*);
void LCD_Write_String_Len(char* Str, unsigned char len);
void LCD_Write_String0_8(char*, align_t);
void LCD_Write_String0_16(char*, align_t);


void LCD_Init(void) {

#ifdef LCD_LEGACY
    LCD_PORT = (LCD_PORT & ~LCD_PORT_MASK) | (RS | EN);
#else
    I2C_Master_Start();
    I2C_Master_Write(LCD_I2C_ADDRESS); // Initialize LCD module with I2C address = 0x4E ((0x27<<1) for PCF8574) or 0x7E ((0x3F<<1) for PCF8574A)
    I2C_Master_Write((RS | EN) | LCD_BACKLIGHT);
    I2C_Master_Stop();
#endif

    delay_ms(50);
    LCD_Write_4Bit(0x30, 0);
    delay_us(4100);
    LCD_Write_4Bit(0x30, 0);
    delay_us(100);
    LCD_Write_4Bit(0x30, 0);
    delay_us(100);
    LCD_CMD(LCD_RETURN_HOME);
    LCD_CMD(LCD_FUNCTION_SET | (LCD_TYPE << 2));
    LCD_CMD(LCD_TURN_ON);
    LCD_Clear();
    LCD_CMD(LCD_ENTRY_MODE_SET | LCD_INCREMENT | LCD_NOSHIFT);

    // LCD set custom characters
    unsigned char i = 0;
#ifdef EEPROM_CUSTOM_CHARS
    char tbuf[8];
    for (i = 0; i < 64; i = i + 8) {
        LCD_CMD(LCD_SETCGRAMADDR | (i & ~0x07));
        HW_read_eeprom_block((unsigned char*) tbuf, EEPROM_CUSTOM_CHARS_ADDRESS + i, 8);
        char *ptr = &tbuf[0];
        unsigned char c = 8;
        while (c--) {
            LCD_Write_Char(*ptr++);
        }
    }
#else        
    for (i = 0; i < 64; i++) {
        // LCD_SETCGRAMADDR | (location << 3)
        if ((i & 0x07) == 0) {
            LCD_CMD(LCD_SETCGRAMADDR | (i & ~0x07));
        }
        LCD_Write_Char(pgm_read_byte(&custom_chars[i]));
    }
#endif
}


void LCD_Write_4Bit(unsigned char Nibble, unsigned char mode) {
#ifndef LCD_LEGACY
    I2C_Master_Start();
    I2C_Master_Write(LCD_I2C_ADDRESS);
    I2C_Master_Write((Nibble | EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();

    I2C_Master_Start();
    I2C_Master_Write(LCD_I2C_ADDRESS);
    I2C_Master_Write((Nibble & ~EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();
    delay_us(40);
#else
  LCD_PORT = (LCD_PORT & ~LCD_PORT_MASK) | ((Nibble | EN)  | mode);
  delay_us(5);
  LCD_PORT = (LCD_PORT & ~LCD_PORT_MASK) | ((Nibble & ~EN) | mode);
  delay_us(50);
#endif
}

void LCD_CMD(unsigned char CMD) 
{
  LCD_Write_4Bit(CMD & 0xF0, 0);
  LCD_Write_4Bit((CMD << 4) & 0xF0, 0);
  delay_us(40);
}

void LCD_Write_Char(char Data)
{
  LCD_Write_4Bit(Data & 0xF0, RS);
  LCD_Write_4Bit((Data << 4) & 0xF0, RS);
}

void LCD_Write_String(char* Str) {
    for (unsigned char i = 0; Str[i] != '\0'; i++) {
        LCD_Write_Char(Str[i]); 
    }
}

void LCD_Write_String_Len(char* Str, unsigned char len) {
    for (unsigned char i = 0; i < len; i++) {
        LCD_Write_Char(Str[i]); 
    }
}

void __LCD_Write_String(char* Str, unsigned char len, unsigned char max, align_t align) {
    unsigned char i;
    unsigned char p = 0;
    if (align == LCD_ALIGN_RIGHT || align == LCD_ALIGN_CENTER) {
        p = (max - len);
        if (align == LCD_ALIGN_CENTER) {
            p = p >> 1;
        }
        for (i = 0; i < p; i++) {
            LCD_Write_Char(' '); 
        }
    }

    for (i = 0; i < len; i++) {
        LCD_Write_Char(Str[i]); 
    }

    if (align == LCD_ALIGN_LEFT || align == LCD_ALIGN_CENTER) {
        p += len;
        while (++p <= max) {
            LCD_Write_Char(' '); 
        }
    }
}

void LCD_Write_String0_8(char* Str, align_t align) {
    unsigned char len = 0;

    char* p = Str;
    while (*p++ != '\0') len++;
    
    __LCD_Write_String(Str, len, 8, align);
}

void LCD_Write_String0_16(char* Str, align_t align) {
    unsigned char len = 0;

    char* p = Str;
    while (*p++ != '\0') len++;
    
    __LCD_Write_String(Str, len, 16, align);
}

void LCD_Set_Cursor(unsigned char ROW, unsigned char COL) 
{    
  switch(ROW) 
  {
    case 2:
      LCD_CMD(0xC0 + COL-1);
      break;
    case 3:
      LCD_CMD(0x94 + COL-1);
      break;
    case 4:
      LCD_CMD(0xD4 + COL-1);
      break;
    // Case 1  
    default:
      LCD_CMD(0x80 + COL-1);
  }
}

void LCD_Clear(void)
{
  LCD_CMD(LCD_CLEAR);
  delay_us(1520);
}
