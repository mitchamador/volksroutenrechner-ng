
#include "lcd.h"
#include "i2c.h"
#include <stdbool.h>

unsigned char i2c_add;

//---------------[ LCD Routines ]----------------
//------------------------------------------------------

void LCD_Init(unsigned char I2C_Add) 
{
  i2c_add = I2C_Add;

  I2C_Master_Start();
  I2C_Master_Write(i2c_add);
  I2C_Master_Write((0x00 | 0x04) | LCD_BACKLIGHT);
  I2C_Master_Stop();

  delay_ms(50);
  LCD_CMD(0x03);
  delay_us(4100);
  LCD_CMD(0x03);
  delay_us(100);
  LCD_CMD(0x03);
  LCD_CMD(LCD_RETURN_HOME);
  delay_us(1520);
  LCD_CMD(LCD_FUNCTION_SET | (LCD_TYPE << 2));
  LCD_CMD(LCD_TURN_ON);
  LCD_CMD(LCD_CLEAR);
  delay_us(1520);
  LCD_CMD(LCD_ENTRY_MODE_SET | LCD_INCREMENT | LCD_NOSHIFT);
  
//  __delay_ms(30);
//  LCD_CMD(0x03);
//  __delay_ms(5);
//  LCD_CMD(0x03);
//  __delay_ms(5);
//  LCD_CMD(0x03);
//  __delay_ms(5);
//  LCD_CMD(LCD_RETURN_HOME);
//  __delay_ms(5);
//  LCD_CMD(0x20 | (LCD_TYPE << 2));
//  __delay_ms(50);
//  LCD_CMD(LCD_TURN_ON);
//  __delay_ms(50);
//  LCD_CMD(LCD_CLEAR);
//  __delay_ms(50);
//  LCD_CMD(LCD_ENTRY_MODE_SET | LCD_INCREMENT | LCD_NOSHIFT);
//  __delay_ms(50);
}

void LCD_Write_4Bit(unsigned char Nibble, unsigned char mode) {

//    I2C_Master_Start();
//    I2C_Master_Write(i2c_add);
//    I2C_Master_Write(Nibble | mode |LCD_BACKLIGHT);
//    I2C_Master_Stop();
    
    I2C_Master_Start();
    I2C_Master_Write(i2c_add);
    I2C_Master_Write((Nibble | EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();
//    delay_us(2);

    I2C_Master_Start();
    I2C_Master_Write(i2c_add);
    I2C_Master_Write((Nibble & ~EN) | mode | LCD_BACKLIGHT);
    I2C_Master_Stop();
//    delay_us(50);
}

void LCD_CMD(unsigned char CMD) 
{
  LCD_Write_4Bit(CMD & 0xF0, 0);
  LCD_Write_4Bit((CMD << 4) & 0xF0, 0);
}

void LCD_Write_Char(char Data)
{
  LCD_Write_4Bit(Data & 0xF0, RS);
  LCD_Write_4Bit((Data << 4) & 0xF0, RS);
}

void LCD_Write_String(char* Str) {
    for (unsigned char i = 0; Str[i] != '\0'; i++) {
        //LCD_Write_Char(Str[i]); 
        LCD_Write_4Bit(Str[i] & 0xF0, RS);
        LCD_Write_4Bit((Str[i] << 4) & 0xF0, RS);
    }
}

void LCD_Write_String_Len(char* Str, unsigned char len) {
    for (unsigned char i = 0; i < len; i++) {
        //LCD_Write_Char(Str[i]); 
        LCD_Write_4Bit(Str[i] & 0xF0, RS);
        LCD_Write_4Bit((Str[i] << 4) & 0xF0, RS);
    }
}

void __LCD_Write_String(char* Str, unsigned char len, unsigned char max, bool right_align) {
    unsigned char i;
    if (right_align) {
        for (i = len; i < max; i++) {
            //LCD_Write_Char(' ');
            LCD_Write_4Bit(' ' & 0xF0, RS);
            LCD_Write_4Bit((' ' << 4) & 0xF0, RS);
        }
    }

    for (i = 0; i < len; i++) {
        //LCD_Write_Char(Str[i]);
        LCD_Write_4Bit(Str[i] & 0xF0, RS);
        LCD_Write_4Bit((Str[i] << 4) & 0xF0, RS);
    }

    if (!right_align) {
        while (++len <= max) {
            //LCD_Write_Char(' ');
            LCD_Write_4Bit(' ' & 0xF0, RS);
            LCD_Write_4Bit((' ' << 4) & 0xF0, RS);
        }
    }
}

void LCD_Write_String8(char* Str, unsigned char len, bool right_align) {
    __LCD_Write_String(Str, len, 8, right_align);
}

void LCD_Write_String16(char* Str, unsigned char len, bool right_align) {
    __LCD_Write_String(Str, len, 16, right_align);
}

void LCD_Write_String0_8(char* Str, bool right_align) {
    unsigned char len = 0;

    char* p = Str;
    while (*p++ != '\0') len++;
    
    __LCD_Write_String(Str, len, 8, right_align);
}

void LCD_Write_String0_16(char* Str, bool right_align) {
    unsigned char len = 0;

    char* p = Str;
    while (*p++ != '\0') len++;
    
    __LCD_Write_String(Str, len, 16, right_align);
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

void LCD_SL(void)
{
  LCD_CMD(0x18);
}

void LCD_SR(void)
{
  LCD_CMD(0x1C);
}

void LCD_Clear(void)
{
  //LCD_CMD(0x01); 
  LCD_Write_4Bit(LCD_CLEAR & 0xF0, 0);
  LCD_Write_4Bit((LCD_CLEAR << 4) & 0xF0, 0);
  delay_ms(2);
}
