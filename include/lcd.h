#ifndef _LCD_H
#define _LCD_H

#include "main.h"
#include "i2c.h"

#define LCD_I2C_ADDRESS 0x4E

#ifndef LCD_LEGACY
#define RS            (1 << 0)
#define RW            (1 << 1)
#define EN            (1 << 2)
#define LCD_BACKLIGHT (1 << 3)
#endif

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

typedef enum {
    LCD_ALIGN_NONE = 0,
    LCD_ALIGN_LEFT = 0,
    LCD_ALIGN_RIGHT,
    LCD_ALIGN_CENTER
} align_t;

//-----------[ Functions' Prototypes ]--------------

//---[ LCD Routines ]---

// max delays for 190kHz
#define LCD_DELAY_CMD 53
#define LCD_DELAY_CLEAR 2160

#define LCD_Check_Busy() delay_us(LCD_DELAY_CMD)

// delay en strobe delay
#define LCD_delay_en_strobe()
// delay between 4 bits
#define LCD_delay_4bits()

void LCD_Init(void);
void LCD_CMD(char CMD);
void LCD_Clear(void);

#define LCD_Write_String8(Str, len, align) __LCD_Write_String(Str, len, 8, align)
#define LCD_Write_String16(Str, len, align) __LCD_Write_String(Str, len, 16, align)
void __LCD_Write_String(char*, unsigned char, unsigned char, align_t);

#endif
