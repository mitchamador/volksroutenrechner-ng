#ifndef HW_AVR_H
#define HW_AVR_H

#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__))

#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

typedef uint8_t flag_t;
typedef uint16_t eeaddr_t;
typedef uint32_t uint24_t;

#define __near
#define __bank0
#define __bank1
#define __bank2
#define __bank3

#define __section(x)

#define __EEDATA(a0,a1,a2,a3,a4,a5,a6,a7) a0,a1,a2,a3,a4,a5,a6,a7,

// timer1 compare 10ms, 2500 with prescaler 1:64 running at 16Mhz
#define HW_MAIN_TIMER_TICKS_PER_PERIOD      2500

// min/max value of adc reading
#define HW_ADC_MIN     0
#define HW_ADC_MAX     1023

// Voltage Reference: AVCC pin, right aligned
#define ADC_VREF_TYPE               ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR))

// mux for power supply pin (PC3/ADC3)
#define HW_ADC_CHANNEL_POWER_SUPPLY ((0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (1 << MUX0))
// mux for buttons pin (PC0/ADC0)
#define HW_ADC_CHANNEL_BUTTONS      ((0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0))
// mux for fuel tank pin (ADC6)
#define HW_ADC_CHANNEL_FUEL_TANK    ((0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0))

#define HW_delay_ms(ms)             _delay_ms(ms)
#define HW_delay_us(us)             _delay_us(us)

#define HW_start_fuel_timer()       TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (0 << CS00);
#define HW_stop_fuel_timer()        TCCR0B = (0 << WGM02) | (0 << CS02) | (0 << CS01) | (0 << CS00);

// start timer with prescaler 1:64
#define HW_start_main_timer()       TCCR1B = (TCCR1B & ~((1 << CS12) | (1 << CS11) | (1 << CS10))) | ((0 << CS12) | (1 << CS11) | (1 << CS10));

#define HW_enable_interrupts()      sei();
#define HW_disable_interrupts()     cli();

// start adc
#define HW_adc_start()              ADCSRA = ADCSRA | (1 << ADSC);
// set adc channel
#define HW_adc_set_channel(ch)      ADMUX = ADC_VREF_TYPE | ch
// read adc
#define HW_adc_read()               (ADCW)

// PB0/PCINT0
#define HW_tx_active()              ((PINB & _BV(PINB0)) != 0)
// PB1/PCINT1
#define HW_fuel_active()            ((PINB & _BV(PINB1)) == 0)

// digital button read
// PC0 - OK, PC1 - NEXT, PC2 - prev
#define HW_key1_pressed()           ((PINC & _BV(PINC1)) == 0)
// analog/digital button OK
#define HW_key2_pressed()           ((PINC & _BV(PINC0)) == 0)
#define HW_key3_pressed()           ((PINC & _BV(PINC2)) == 0)

// power pin settings
#define HW_pwr_on()                 (PORTD |=  _BV(PORTD5))
#define HW_pwr_off()                (PORTD &= ~_BV(PORTD5))

// buzzer pin settings
#define HW_snd_on()                 (PORTD |=  _BV(PORTD6))
#define HW_snd_off()                (PORTD &= ~_BV(PORTD6))

// onewire bit set low
#define HW_1wire_clear()            (PORTD &= ~_BV(PORTD7))
// onewire bit get
#define HW_1wire_get()              ((PIND & _BV(PIND7)) != 0 ? 1 : 0)
// configure DS18B20_PIN pin as output
#define HW_1wire_output()           (DDRD |= _BV(DDD7))
// configure DS18B20_PIN pin as input
#define HW_1wire_input()            (DDRD &= ~_BV(DDD7))

// encoder support
#if !defined(ENCODER_SUPPORT)
#define ENCODER_ENABLED             0
#else
#define ENCODER_ENABLED             1
// encoder data (PC1/PCINT9) and clk (PC2/PCINT10)
#define HW_encoder_get_data()       ((PINC & _BV(PINC1)) != 0 ? 1 : 0)
#define HW_encoder_get_clk()        ((PINC & _BV(PINC2)) != 0 ? 1 : 0)
#endif
#define PCINT_ENCODER               (ENCODER_ENABLED << PCINT9) | (ENCODER_ENABLED << PCINT10)

#if defined(LCD_SSD1322_1602)

#define OLED_DC_PIN PORTD2
#define OLED_DC_DDR DDD2

#if defined(SPI_UART)
#define OLED_CS_PIN PORTD0
#define OLED_CS_DDR DDD0
#else
#define OLED_CS_PIN PORTB2
#define OLED_CS_DDR DDB2
#endif

#define HW_lcd_spi_gpio_init()

#define HW_lcd_spi_dc_low()         PORTD &= ~_BV(OLED_DC_PIN);
#define HW_lcd_spi_dc_high()        PORTD |= _BV(OLED_DC_PIN);

#if defined(SPI_UART)
#define HW_lcd_spi_cs_low()         PORTD &= ~_BV(OLED_CS_PIN);
#define HW_lcd_spi_cs_high()        PORTD |= _BV(OLED_CS_PIN);
#else
#define HW_lcd_spi_cs_low()         PORTB &= ~_BV(OLED_CS_PIN);
#define HW_lcd_spi_cs_high()        PORTB |= _BV(OLED_CS_PIN);
#endif

#if defined(SPI_UART)
#define PORTB_LCD_INIT              0
#define DDRB_LCD_INIT               0
#define PORTD_LCD_INIT              _BV(OLED_CS_PIN)
#define DDRD_LCD_INIT               _BV(OLED_CS_DDR) | _BV(OLED_DC_DDR)   
#else
#define PORTB_LCD_INIT              _BV(OLED_CS_PIN)
#define DDRB_LCD_INIT               _BV(OLED_CS_DDR)
#define PORTD_LCD_INIT              0
#define DDRD_LCD_INIT               _BV(OLED_DC_DDR)   
#endif

#elif defined(LCD_1602)

// 4-bit 1602 LCD definitions
// rs - PB2
// rw - PB3
// en - PB4
// data - PD0..PD3

#define HW_lcd_set_data(data)       PORTD = (PORTD & ~0x0F) | ((data >> 4) & 0x0F);

#define HW_lcd_rs_low()             (PORTB &= ~_BV(PORTB2))
#define HW_lcd_rs_high()            (PORTB |=  _BV(PORTB2))
#define HW_lcd_rw_low()             (PORTB &= ~_BV(PORTB3))
#define HW_lcd_rw_high()            (PORTB |=  _BV(PORTB3))
#define HW_lcd_en_low()             (PORTB &= ~_BV(PORTB4))
#define HW_lcd_en_high()            (PORTB |=  _BV(PORTB4))

#define PORTB_LCD_INIT              _BV(PORTB4)
#define DDRB_LCD_INIT               _BV(DDB2) | _BV(DDB3) | _BV(DDB4)
#define PORTD_LCD_INIT              0
#define DDRD_LCD_INIT               _BV(DDD0) | _BV(DDD1) | _BV(DDD2) | _BV(DDD3)  

#else

#define PORTB_LCD_INIT     0
#define DDRB_LCD_INIT      0
#define PORTD_LCD_INIT     0
#define DDRD_LCD_INIT      0   

#endif

// DDRx: 0 - input, 1 - output
// init values for port's data direction
#define DDRB_INIT                   DDRB_LCD_INIT
#define DDRC_INIT                   0
#define DDRD_INIT                   DDRD_LCD_INIT | _BV(DDD5) | _BV(DDD6)

// init values for port's data 
#define PORTB_INIT                  PORTB_LCD_INIT /* | _BV(PORTB0) | _BV(PORTB1) */
// digital buttons needs external pullups
#define PORTC_INIT                  0
#define PORTD_INIT                  PORTD_LCD_INIT | _BV(PORTD5)

#else
#error "device not supported"
#endif

#endif 	/* HW_AVR_H */