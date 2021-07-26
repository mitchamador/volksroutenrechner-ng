
#include "lcd.h"
#include "i2c.h"
#include <stdbool.h>

#ifndef LCD_LEGACY
unsigned char i2c_add;
#endif
//---------------[ LCD Routines ]----------------
//------------------------------------------------------

#ifndef LCD_LEGACY
void LCD_Init(unsigned char I2C_Add) 
{
  i2c_add = I2C_Add;
  

  I2C_Master_Start();
  I2C_Master_Write(i2c_add);
  I2C_Master_Write((RS | EN) | LCD_BACKLIGHT);
  I2C_Master_Stop();
#else
void LCD_Init() {  
  LCD_PORT = (LCD_PORT & ~LCD_PORT_MASK) | (RS | EN);
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
}

void LCD_Write_4Bit(unsigned char Nibble, unsigned char mode) {
#ifndef LCD_LEGACY
    I2C_Master_Start();
    I2C_Master_Write(i2c_add);
    I2C_Master_Write((Nibble | EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();

    I2C_Master_Start();
    I2C_Master_Write(i2c_add);
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
