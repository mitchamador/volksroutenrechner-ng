#if defined(LCD_SSD1322_1602)

#include "core.h"
#include "lcd_ssd1322.h"

#include "fonts/jetbrains_mono_16x32.h"
#include "fonts/source_code_pro_semibold.h"
#include "fonts/noto_mono_16x32.h"

BMFont *bmFont;

void LCD_Init() {

  HW_delay_ms(10);
  
  SSD1322_HW_init();
  
  uint8_t  numCommands, numArgs;
  uint16_t ms;
  uint8_t *addr = (uint8_t *) &SSD1322_init_commands;

  numCommands = pgm_read_byte(addr++);

  while(numCommands--) {
    SSD1322_command(pgm_read_byte(addr++));
    numArgs  = pgm_read_byte(addr++);
    while(numArgs--) SSD1322_data(pgm_read_byte(addr++));
  }

  SSD1322_clear_all();

  //bmFont = &font_JetBrains_Mono;
  //bmFont = &font_Source_Code_Pro_Semibold;
  bmFont = &font_Noto_Mono;

}

void LCD_flush_buffer() {
  SSD1322_start_page();
  do
  {
    for (uint8_t x = 0; x < LCD_WIDTH * 2; x ++) {
      uint16_t _x, _y;
      if (x < LCD_WIDTH) {
         _y = 0;
         _x = x;
      } else {
         _y = 32;
         _x = (x - LCD_WIDTH);
      }
      uint8_t style = BMFONT_DEFAULT;
      if (lcd_cursor.mode != LCD_CURSOR_OFF && x == lcd_cursor.pos) {
         switch (lcd_cursor.mode)
         {
            case LCD_UNDERLINE_ON:
               style = BMFONT_UNDERLINE;
               break;
            case LCD_BLINK_CURSOR_ON:
               style = BMFONT_INVERT_CHAR;
               break;
         }
      }
      bmfont_draw_char(_x * 16, _y, lcd_buf[x], style);
    }
  } while (SSD1322_next_page());
  SSD1322_send_buffer();
}

void bmfont_draw_char(int x, int y, unsigned char c, unsigned char style)
{
  if (x >= SSD1322_LCD_WIDTH || y >= SSD1322_LCD_HEIGHT || x + bmFont->wd - 1 < 0 || y + bmFont->ht - 1 < 0) {
    return;
  }

  // clear background before drawing
  SSD1322_fill_rect(x, y, bmFont->wd, bmFont->ht, BMFONT_FLAG(INVERT_CHAR, style) ? 0x0F : 0x00);

  if (BMFONT_FLAG(UNDERLINE, style)) {
    uint8_t uHt = (bmFont->ht - 1) / 8 + 1;
    SSD1322_fill_rect(x, y + (bmFont->ht - uHt), bmFont->wd, uHt, 0x0F);
  }

  if (c < bmFont->firstCh || c > bmFont->lastCh) {
    return;
  }

  uint16_t charOffset = pgm_read_word(&(bmFont->offs[c - bmFont->firstCh]));

  if (charOffset == 0xFFFF) {
    return;
  }

  uint8_t charX = pgm_read_byte(&bmFont->data[charOffset + 0]);
  uint8_t charY = pgm_read_byte(&bmFont->data[charOffset + 1]);
  uint8_t charW = pgm_read_byte(&bmFont->data[charOffset + 2]);
  uint8_t charH = pgm_read_byte(&bmFont->data[charOffset + 3]);

  // draw character
  SSD1322_draw_bitmap(x + charX, y + charY, charW, charH, (bmFont->flags & BMFONT_DRAWCHAR_MASK) | style, (uint8_t *) &bmFont->data[charOffset + 4]);

}

void SSD1322_transfer_cdata(uint8_t dat, int count) {
  SSD1322_SPI_start_data();
  while (count-- > 0) {
    SSD1322_SPI_transfer(dat);
  }
  SSD1322_SPI_end();
}

void SSD1322_transfer_block(uint8_t *pBuf, int count) {
  SSD1322_SPI_start_data();
  SPI_transfer_block(pBuf, count);
  SSD1322_SPI_end();
}

void SSD1322_command(uint8_t cmd)
{
   SSD1322_SPI_start_command();
   SSD1322_SPI_transfer(cmd);
   SSD1322_SPI_end();
}

void SSD1322_data(uint8_t dat)
{
   SSD1322_SPI_start_data();
   SSD1322_SPI_transfer(dat);
   SSD1322_SPI_end();
}

void SSD1322_HW_init() {
  HW_lcd_spi_gpio_init();
  SPI_init();
}

void SSD1322_set_window(uint8_t Xstart, uint8_t Ystart, uint8_t Xend, uint8_t Yend)
{
  SSD1322_command(0x15);
  SSD1322_data(Xstart + 0x1c);
  SSD1322_data(Xend + 0x1c);
  SSD1322_command(0x75);
  SSD1322_data(Ystart);
  SSD1322_data(Yend);
  SSD1322_command(0x5c); //write ram command
}

void SSD1322_clear_all()
{
  SSD1322_set_window(0, 0, 63, 127);
  SSD1322_transfer_cdata(0x00, 16384);
}

#define pagePtr 0

uint8_t page_buf[1024];

int16_t _dy = 0;

int16_t pageY = 0;

#define _x 0
#define _y 0
#define _w 256
#define _h 8

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

void SSD1322_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, int8_t c)
{
  int16_t xs = MAX(x,_x);
  int16_t xe = MIN(x + w, _x + _w);
  int16_t ys = MAX(y, (_y + _dy));
  int16_t ye = MIN(y + h, (_y + _dy) + _h);
  if (xe >= xs && ye >= ys)
  {
    uint8_t* _ptr = &page_buf[pagePtr + (ys - _dy)*256/2];

    for (uint16_t _yy = ys; _yy < ye; _yy++)
    {
      uint8_t* __ptr = _ptr + xs / 2;
      for (uint16_t _xx = xs; _xx < xe; _xx++)
      {
        uint8_t d = *__ptr;
        if ((_xx & 0x01) != 0)
        {
          *__ptr++ = (d & 0xF0) | c;
        }
        else
        {
          *__ptr = (d & 0x0F) | (c << 4);
        }
      }
      _ptr += 256 / 2;
    }
  }
}

void SSD1322_draw_bitmap(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t flags, uint8_t *data) {
{
  int16_t xs = MAX(x,_x);
  int16_t xe = MIN(x + w, _x + _w);
  int16_t ys = MAX(y, (_y + _dy));
  int16_t ye = MIN(y + h, (_y + _dy) + _h);
  if (xe >= xs && ye >= ys)
  {
      uint8_t* _dest = &page_buf[pagePtr + (ys - _dy) * 256 / 2];

    //uint8_t row_w = /*mode == BMFONT_PACKED ||*/ (w & 0x01) == 0 ? w : (w + 1);
    uint8_t row_w = w;

    uint16_t srcy = (ys - y) * row_w;

    uint8_t bitDepth = flags & BMFONT_DEPTH_MASK;

    uint8_t xDiv = 2;
    if (bitDepth == BMFONT_DEPTH_4BIT) {
      xDiv = 2;
    } else if (bitDepth == BMFONT_DEPTH_2BIT) {
      xDiv = 4;
    } else if (bitDepth == BMFONT_DEPTH_1BIT) {
      xDiv = 8;
    }

    for (uint16_t _yy = ys; _yy < ye; _yy++)
    {
      uint8_t src;

      uint8_t* __dest = _dest + xs / 2;

      for (uint16_t _xx = xs; _xx < xe; _xx++)
      {
        uint16_t srcx = _xx - x + srcy;
        
        uint8_t c = pgm_read_byte(data + srcx / xDiv);

        if (xDiv == 2) {
          if ((srcx & 0x01) == 0) {
            c >>= 4;
          }
          c &= 0x0F;
        } else if (xDiv == 4) {
#if 1
          switch (srcx & 0x03) {
            case 0:
              c = c >> 6;
              break;
            case 1:
              c = c >> 4;
              break;
            case 2:
              c = c >> 2;
              break;
          }
          c &= 0x03;
#else
          c = (c >> ((3 - (srcx & 0x03)) * 2))/* & 0x03*/;
#endif
          // convert 2 bit colors from 0x00,0x01,0x02,0x03 to 0x00,0x05,0x0A,0xFF
          c |= c << 2; 
        }

        if (BMFONT_FLAG(LOW_BRIGHTNESS, flags)) {
          c = (c >> 2) & 0x03; // lowering brightness
        }

        if (BMFONT_FLAG(INVERT_CHAR, flags)) {
          c = ~c & 0x0F;
        }

        uint8_t d = *__dest;
        if ((_xx & 0x01) != 0)
        {
          *__dest++ = (d & 0xF0) | c;
        }
        else
        {
          *__dest = (d & 0x0F) | (c << 4);
        }
      }
      _dest += 256 / 2;

      srcy += row_w;
    }
  }
}
}

void SSD1322_start_page()
{
  _dy = 0;
  pageY = 64 - pageY;
}

bool SSD1322_next_page()
{
  uint8_t *pBuf = &page_buf[pagePtr];

  uint8_t row, col;

  SSD1322_set_window(0, pageY + _dy, 255 / 4, pageY + _dy + 7);

  SSD1322_transfer_block(pBuf, 1024);

  _dy += 8;

  memset(page_buf, 0, sizeof(page_buf));

  return _dy < 64;
}

void SSD1322_send_buffer() {
  SSD1322_command(0xA1); /*start line*/ 
  SSD1322_data(pageY); 
}


#endif