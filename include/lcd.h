#ifndef _LCD_H
#define _LCD_H

#include <stdbool.h>

#pragma warning disable 520

#define RS (1 << 0)
#define EN (1 << 2)

#define LCD_BACKLIGHT          0x08
#define LCD_NOBACKLIGHT        0x00
#define LCD_FIRST_ROW          0x80
#define LCD_SECOND_ROW         0xC0
#define LCD_THIRD_ROW          0x94
#define LCD_FOURTH_ROW         0xD4
#define LCD_CLEAR              0x01
#define LCD_RETURN_HOME        0x02
#define LCD_ENTRY_MODE_SET     0x04
#define LCD_CURSOR_OFF         0x0C
#define LCD_UNDERLINE_ON       0x0E
#define LCD_BLINK_CURSOR_ON    0x0F
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

//-----------[ Functions' Prototypes ]--------------

//---[ LCD Routines ]---

void LCD_Init(unsigned char I2C_Add);
void LCD_Write_4Bit(unsigned char Nibble, unsigned char mode);
void LCD_CMD(unsigned char CMD);
void LCD_Set_Cursor(unsigned char ROW, unsigned char COL);
void LCD_Write_Char(char);
void LCD_Write_String(char*);
void LCD_Write_String_Len(char* Str, unsigned char len);
void __LCD_Write_String(char*, unsigned char, unsigned char, bool);
void LCD_Write_String8(char*, unsigned char, bool);
void LCD_Write_String16(char*, unsigned char, bool);
void LCD_Write_String0_8(char*, bool);
void LCD_Write_String0_16(char*, bool);
void Backlight(void);
void noBacklight(void);
void LCD_SR(void);
void LCD_SL(void);
void LCD_Clear(void);

#endif