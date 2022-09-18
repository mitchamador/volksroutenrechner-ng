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
#define delay_ms(ms) _delay_ms(ms)
#define delay_us(us) _delay_us(us)

#define __EEDATA(a0,a1,a2,a3,a4,a5,a6,a7) a0,a1,a2,a3,a4,a5,a6,a7,

typedef uint32_t uint24_t;
typedef uint16_t eeaddr_t;

#define __bit uint8_t
#define __bank0
#define __bank1
#define __bank2
#define __bank3

#define __section(x)

// min/max value of adc reading
#define ADC_MIN     0
#define ADC_MAX     1023

#define start_fuel_timer() TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (0 << CS00);

#define stop_fuel_timer()  TCCR0B = (0 << WGM02) | (0 << CS02) | (0 << CS01) | (0 << CS00);

// timer1 compare 10ms, 2500 with prescaler 1:64 running at 16Mhz
#define TIMER_MAIN_PERIOD 0.01f
#define TIMER_MAIN_TICKS_PER_PERIOD 2500

// start timer with prescaler 1:64
#define start_main_timer() TCCR1B = (TCCR1B & ~((1 << CS12) | (1 << CS11) | (1 << CS10))) | ((0 << CS12) | (1 << CS11) | (1 << CS10));

#if defined(PROTEUS_DEBUG)
#define main_timer_overflow() ((TIFR1 & (1 << OCF1A)) != 0)
#else
#define main_timer_overflow() ((TIFR1 & (1 << ICF1)) != 0)
#endif

#define capture_main_timer(_main_timer)             \
    _main_timer = TCNT1;                            \
    if (main_timer_overflow()) {                    \
        _main_timer += TIMER_MAIN_TICKS_PER_PERIOD; \
    }                                               \


// start adc
#define start_adc() ADCSRA = ADCSRA | (1 << ADSC);

#define enable_interrupts() sei();
#define disable_interrupts() cli();

#define int_handler_GLOBAL_begin

#define int_handler_GLOBAL_end

#define int_handler_fuel_speed_begin ISR(PCINT0_vect) {     \

#define int_handler_fuel_speed_end }                        \

#define int_handler_encoder_begin ISR(PCINT1_vect) {        \

#define int_handler_encoder_end }                           \

#define int_handler_fuel_timer_overflow_begin ISR(TIMER0_COMPA_vect) {   \
    
#define int_handler_fuel_timer_overflow_end }                            \

#if defined(PROTEUS_DEBUG)

#define int_handler_main_timer_overflow_begin ISR(TIMER1_COMPA_vect) {   \
      TIFR1 = (1 << OCF1B);                                              \

#define int_handler_main_timer_overflow_end }                            \

#else

#define int_handler_main_timer_overflow_begin ISR(TIMER1_CAPT_vect) {    \

#define int_handler_main_timer_overflow_end }                            \

#endif
    
#define int_handler_adc_begin ISR(ADC_vect) {               \
    
#define int_handler_adc_end }                               \

// DDRx: 0 - input, 1 - output

// PB0/PCINT0
#define TX_ACTIVE   ((PINB & _BV(PINB0)) != 0)
// PB1/PCINT1
#define FUEL_ACTIVE ((PINB & _BV(PINB1)) == 0)

#define set_adc_channel(ch) ADMUX = ADC_VREF_TYPE | ch

#define adc_read_value() (ADCW)

// Voltage Reference: AVCC pin, right aligned
#define ADC_VREF_TYPE ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR))
// mux for power supply pin (PC3/ADC3)
#define ADC_CHANNEL_POWER_SUPPLY ((0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (1 << MUX0))
// mux for buttons pin (PC0/ADC0)
#define ADC_CHANNEL_BUTTONS ((0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0))
// mux for fuel tank pin (ADC6)
#define ADC_CHANNEL_FUEL_TANK ((0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0))

// power pin settings
#define PWR_ON  (PORTD |=  _BV(PORTD5))
#define PWR_OFF (PORTD &= ~_BV(PORTD5))

// buzzer pin settings
#define SND_ON  (PORTD |=  _BV(PORTD6))
#define SND_OFF (PORTD &= ~_BV(PORTD6))

// onewire pin settings
#define ONEWIRE_CLEAR    (PORTD &= ~_BV(PORTD7))

#define ONEWIRE_GET      ((PIND & _BV(PIND7)) != 0 ? 1 : 0)
// configure DS18B20_PIN pin as output
#define ONEWIRE_OUTPUT   (DDRD |= _BV(DDD7))
// configure DS18B20_PIN pin as input
#define ONEWIRE_INPUT    (DDRD &= ~_BV(DDD7))

// init values for port's data direction
#define DDRB_INIT _BV(DDB2) | _BV(DDB3) | _BV(DDB4)
#define DDRC_INIT 0
#define DDRD_INIT _BV(DDD0) | _BV(DDD1) | _BV(DDD2) | _BV(DDD3) | _BV(DDD5) | _BV(DDD6)

// init values for port's data 
#define PORTB_INIT _BV(PORTB0) | _BV(PORTB1)
#ifdef ADC_BUTTONS
#define PORTC_INIT 0
#else
#define PORTC_INIT _BV(PORTC0) | _BV(PORTC1) | _BV(PORTC2)
#endif
#define PORTD_INIT _BV(PORTD5)

#ifndef ADC_BUTTONS
// digital button read
// PC0 - OK, PC1 - NEXT, PC2 - prev
#define KEY2_PRESSED ((PINC & _BV(PINC0)) == 0)
#ifndef ENCODER_SUPPORT
#define KEY1_PRESSED ((PINC & _BV(PINC1)) == 0)
#define KEY3_PRESSED ((PINC & _BV(PINC2)) == 0)
#endif
#endif

// analog/digital button OK
#define KEY_OK_PRESSED ((PINC & _BV(PINC0)) == 0)

// encoder support
#if !defined(ENCODER_SUPPORT)
#define ENCODER_ENABLED 0
#else
#define ENCODER_ENABLED 1
// encoder data (PC1/PCINT9) and clk (PC2/PCINT10)
#define ENCODER_DATA ((PINC & _BV(PINC1)) != 0 ? 1 : 0)
#define ENCODER_CLK  ((PINC & _BV(PINC2)) != 0 ? 1 : 0)
#endif

#define PCINT_ENCODER (ENCODER_ENABLED << PCINT9) | (ENCODER_ENABLED << PCINT10)

#ifdef LCD_LEGACY

// 4-bit 1602 LCD definitions
// rs - PB2
// rw - PB3
// en - PB4
// data - PD0..PD3

#define LCD_DATA_PORT       PORTD
#define LCD_DATA_PORT_SHIFT 4
#define LCD_DATA_PORT_MASK  (0xF0 >> LCD_DATA_PORT_SHIFT)

#define RS_LOW  (PORTB &= ~_BV(PORTB2))
#define RS_HIGH (PORTB |=  _BV(PORTB2))
#define RW_LOW  (PORTB &= ~_BV(PORTB3))
#define RW_HIGH (PORTB |=  _BV(PORTB3))
#define EN_LOW  (PORTB &= ~_BV(PORTB4))
#define EN_HIGH (PORTB |=  _BV(PORTB4))

#endif

#else
#error "device not supported"
#endif

#endif 	/* HW_AVR_H */