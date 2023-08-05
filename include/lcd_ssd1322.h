#ifndef LCD_SSD1322_1602_H
#define LCD_SSD1322_1602_H

#include "lcd.h"

//BMFont

#define BMFONT_FLAG(f,x) ((x & BMFONT_##f) != 0) 

#define BMFONT_DEFAULT 0

#define BMFONT_DEPTH_MASK    0x03
#define BMFONT_DEPTH_DEFAULT 0x00
#define BMFONT_DEPTH_1BIT    0x01
#define BMFONT_DEPTH_2BIT    0x02
#define BMFONT_DEPTH_4BIT    0x03

#define BMFONT_PACKED (1 << 2)
#define BMFONT_UPPERCASE (1 << 3)
#define BMFONT_FIXED (1 << 4)

#define BMFONT_DRAWCHAR_MASK 0x07

#define BMFONT_UNDERLINE (1 << 5)
#define BMFONT_LOW_BRIGHTNESS (1 << 6)
#define BMFONT_INVERT_CHAR (1 << 7)

typedef struct {
  uint8_t flags;
  uint8_t wd;
  uint8_t ht;
  uint8_t firstCh;
  uint8_t lastCh;
  const uint8_t *data;
  const uint16_t *offs;
} BMFont;

void bmfont_draw_char(int x, int y, unsigned char c, unsigned char style);

// SSD1322

#define SSD1322_SPI_start_command()          \
/* set OLED_DC_PIN to low */                 \
  HW_lcd_spi_dc_low();                       \
/* set OLED_CS_PIN to low */                 \
  HW_lcd_spi_cs_low();                       \

#define SSD1322_SPI_start_data()             \
/* set OLED_DC_PIN to high */                \
  HW_lcd_spi_dc_high();                      \
/* set OLED_CS_PIN to low */                 \
  HW_lcd_spi_cs_low();                       \

#define SSD1322_SPI_end()                    \
/* some delay before raising CS to high */   \
  _delay_us(1);                              \
/* set OLED_CS_PIN to high */                \
  HW_lcd_spi_cs_high();                      \

#define SSD1322_SPI_transfer(data) SPI_transfer(data)

static const uint8_t SSD1322_init_commands[] PROGMEM = {
  18,
  0xFD, 0x01, /*SET COMMAND LOCK*/
  0x12,       /* UNLOCK */
  0xAE, 0x00, /*DISPLAY OFF*/
  0xB3, 0x01, /*DISPLAYDIVIDE CLOCKRADIO/OSCILLATAR FREQUANCY*/
  0x91,
  0xCA, 0x01, /*multiplex ratio*/
  0x3F,       /*duty = 1/64*/
  0xA2, 0x01, /*set offset*/
  0x00,
  0xA1, 0x01, /*start line*/
  0x00,
  0xA0, 0x02, /*set remap*/
  0x14,
  0x11,

  0xAB, 0x01, /*funtion selection*/
  0x01,       /* selection external vdd */
  0xB4, 0x02, /* */
  0xA0,
  0xfd,
  0xC1, 0x01, /*set contrast current */
  0xff,
  0xC7, 0x01, /*master contrast current control*/
  0x0f,

  0xB1, 0x01, /*SET PHASE LENGTH*/
  0xE2,
  0xD1, 0x02, /**/
  0x82,
  0x20,
  0xBB, 0x01, /*SET PRE-CHANGE VOLTAGE*/
  0x1F,
  0xB6, 0x01, /*SET SECOND PRE-CHARGE PERIOD*/
  0x08,
  0xBE, 0x01, /* SET VCOMH */
  0x07,
  0xA6, 0x00,  /*normal display*/
  0xAF, 0x00, /*display ON*/

};

void SSD1322_HW_init();
void SSD1322_transfer_cdata(uint8_t dat, int count);
void SSD1322_transfer_block(uint8_t *pBuf, int count);                                           
void SSD1322_command(uint8_t cmd);
void SSD1322_data(uint8_t dat);

#define SSD1322_LCD_WIDTH  256  //OLED width
#define SSD1322_LCD_HEIGHT  64   //OLED height

void SSD1322_set_window(uint8_t Xstart, uint8_t Ystart, uint8_t Xend, uint8_t Yend); // окно вывода в единицах "4 пиксекля по горизонтали"
void SSD1322_clear_all(); // очистка двух страниц дисплея

void SSD1322_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, int8_t c);
void SSD1322_draw_bitmap(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t flags, uint8_t *data);

void SSD1322_start_page(void);
bool SSD1322_next_page(void);
void SSD1322_send_buffer(void);

#endif