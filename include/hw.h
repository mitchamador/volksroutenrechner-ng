#ifndef HW_H
#define	HW_H

#if defined(__XC8)
#include <xc.h>
#if defined(_16F876A) || defined(_16F1936) || defined(_16F1938)
#define __PIC_MIDRANGE
#if defined(_16F876A)
#define LOW_MEM_DEVICE
#endif
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__)
#define __AVR_ATMEGA
#endif
#elif defined(__AVR)
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__)
#define __AVR_ATMEGA
#endif
#include <avr/io.h>
#endif

#if !defined(__PIC_MIDRANGE) && !defined(__AVR_ATMEGA)
#error "device and compiler not supported"
#endif

#include <stdint.h>

// define cpu frequency
#if defined(__PIC_MIDRANGE)
#define delay_ms(ms) __delay_ms(ms)
#define delay_us(us) __delay_us(us)
#define _XTAL_FREQ 20000000

#define __EEDATA(a0,a1,a2,a3,a4,a5,a6,a7) __EEPROM_DATA(a0,a1,a2,a3,a4,a5,a6,a7);

#else
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

#endif

#if defined(__PIC_MIDRANGE)

#if !defined(HW_LEGACY)
#error "only legacy hardware supported with pic midrange"
#endif

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

// i2c software bit bang
#define I2C_SOFTWARE
// lcd parallel interface
#define LCD_LEGACY

//#pragma warning disable 1090

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)

#if defined(_16F1936) || defined(_16F1938)
#pragma config PLLEN = OFF
#endif

// PORTA definitions (a/d channels)
// RA0 as digital power control pin
// RA1 as analog voltage input
// RA2 as analog fuel tank input
// RA5 as digital 1wire pin

// power control pin (RA0)
#define PWR PORTAbits.RA0
#define PWR_MASK  (1 << _PORTA_RA0_POSITION)

// adc power (AN1/RA1)
#define POWER_SUPPLY_TRIS_MASK (1 << _TRISA_TRISA1_POSITION)

// fuel tank (AN2/RA2)
#define FUEL_TANK_TRIS_MASK (1 << _TRISA_TRISA2_POSITION)

// DS18B20 data pin (RA5)
#define ONEWIRE_PIN      PORTAbits.RA5
#define ONEWIRE_PIN_Dir  TRISAbits.TRISA5
#define ONEWIRE_TRIS_MASK (1 << _TRISA_TRISA5_POSITION)

#if defined(_16F876A)
#define ADC_CHANNEL_MASK ((1 << _ADCON0_CHS2_POSITION) | (1 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
#define ADC_CHANNEL_POWER_SUPPLY ((0 << _ADCON0_CHS2_POSITION) | (0 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
#define ADC_CHANNEL_FUEL_TANK ((0 << _ADCON0_CHS2_POSITION) | (1 << _ADCON0_CHS1_POSITION) | (0 << _ADCON0_CHS0_POSITION))
#define ADCON0_INIT ((1 << _ADCON0_ADCS1_POSITION) | (0 << _ADCON0_ADCS0_POSITION))
// PORTA A/D configuration (AN0-AN4 as analog input)
#define ADCON1_INIT ((1 << _ADCON1_ADFM_POSITION) | (0 << _ADCON1_PCFG3_POSITION) | (0 << _ADCON1_PCFG2_POSITION) | (1 << _ADCON1_PCFG1_POSITION) | (0 << _ADCON1_PCFG0_POSITION))
#elif defined(_16F1936) || defined(_16F1938)
#define ADC_CHANNEL_MASK ((1 << _ADCON0_CHS4_POSITION) | (1 << _ADCON0_CHS3_POSITION) | (1 << _ADCON0_CHS2_POSITION) | (1 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
#define ADC_CHANNEL_POWER_SUPPLY ((0 << _ADCON0_CHS4_POSITION) | (0 << _ADCON0_CHS3_POSITION) | (0 << _ADCON0_CHS2_POSITION) | (0 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
#define ADC_CHANNEL_FUEL_TANK ((0 << _ADCON0_CHS4_POSITION) | (0 << _ADCON0_CHS3_POSITION) | (0 << _ADCON0_CHS2_POSITION) | (1 << _ADCON0_CHS1_POSITION) | (0 << _ADCON0_CHS0_POSITION))
#define ADCON0_INIT 0
#define ADCON1_INIT ((1 << _ADCON1_ADFM_POSITION) | (0 << _ADCON1_ADCS2_POSITION) | (1 << _ADCON1_ADCS1_POSITION) | (0 << _ADCON1_ADCS0_POSITION))
// PORTA AN1/RA1 (adc power) and AN2/RA2 as analog input (fuel tank)
#define ANSELA_INIT ((1 << _ANSELA_ANSA1_POSITION) | (1 << _ANSELA_ANSA2_POSITION))
#define ANSELB_INIT 0

#endif

#define set_adc_channel(ch) ADCON0 = ADCON0_INIT | ch

// PORTB definitions
// RB0, RB1 - SCL/SDA for software i2c
// RB2, RB3 - keys
// RB6 - speed sensor
// RB7 - fuel injector

#define SDA       PORTBbits.RB0
#define SDA_MASK  (1 << _PORTB_RB0_POSITION)
#define SDA_TRIS  TRISBbits.TRISB0
#define SDA_TRIS_MASK   (1 << _TRISB_TRISB0_POSITION)

#define SCL       PORTBbits.RB1
#define SCL_MASK  (1 << _PORTB_RB1_POSITION)
#define SCL_TRIS  TRISBbits.TRISB1
#define SCL_TRIS_MASK   (0 << _TRISB_TRISB1_POSITION)

#define SDA_INPUT SDA_TRIS = 1
#define SDA_OUTPUT SDA_TRIS = 0
#define SDA_HIGH SDA = 1
#define SDA_LOW SDA = 0
#define SDA_GET() (SDA)

#define SCL_INPUT SCL_TRIS = 1
#define SCL_OUTPUT SCL_TRIS = 0
#define SCL_HIGH SCL = 1
#define SCL_LOW SCL = 0

// key1 and key2 (active ground) (legacy hardware)
#define KEY1 PORTBbits.RB2
#define KEY2 PORTBbits.RB3
#define KEY_TRIS_MASK (1 << _TRISB_TRISB2_POSITION) | (1 << _TRISB_TRISB3_POSITION)

// speed sensor and injector
#define TX PORTBbits.RB6
#define TX_TRIS_MASK (1 << _TRISB_TRISB6_POSITION)
#define FUEL PORTBbits.RB7
#define FUEL_TRIS_MASK (1 << _TRISB_TRISB7_POSITION)


// PORTC definitions
// RC0 - sound

#define SND     PORTCbits.RC0
#define SND_TRIS (1 << _TRISC_TRISC0_POSITION)

#ifdef LCD_LEGACY
// LCD definitions
// rs - RC1
// rw - RC2
// en - RC3
// data - RC4..RC7

#define LCD_DATA_PORT       PORTC
#define LCD_DATA_PORT_SHIFT 0
#define LCD_DATA_PORT_MASK  (0xF0 >> LCD_DATA_PORT_SHIFT)

#define RS_LOW  PORTCbits.RC1=0
#define RS_HIGH PORTCbits.RC1=1
#define RW_LOW  PORTCbits.RC2=0
#define RW_HIGH PORTCbits.RC2=1
#define EN_LOW  PORTCbits.RC3=0
#define EN_HIGH PORTCbits.RC3=1

#endif

// init values for port's data direction
#define TRISA_INIT ONEWIRE_TRIS_MASK | POWER_SUPPLY_TRIS_MASK | FUEL_TANK_TRIS_MASK
#define TRISB_INIT KEY_TRIS_MASK | TX_TRIS_MASK | FUEL_TRIS_MASK | SCL_TRIS_MASK | SDA_TRIS_MASK
#define TRISC_INIT 0

// init values for port's data
#define PORTA_INIT PWR_MASK
#define PORTB_INIT 0 | SDA_MASK | SCL_MASK
#define PORTC_INIT 0

// timer1 compare 10ms, 6250 with prescaler 1:8 at 20MHz
#define TIMER1_PERIOD 0.01f
#define TIMER1_VALUE 6250

/* ======================================= */

#define adc_read_value() ((uint16_t) (ADRESH << 8) | ADRESL)

#define start_timer_fuel() T0CS = 0
#define stop_timer_fuel() T0CS = 1

#define start_timer1() TMR1ON = 1

// read 16bit TMR1 value (read TMR1H, TMR1L; if TMR1L == 0x00, re-read TMR1H)
// if overflow occurs during reading (between start of interrupt and TMR1 reading) - set to max value
#define get_timer1(_timer1)                                 \
    *((uint8_t*)(&_timer1) + 1) = TMR1H;                    \
    *((uint8_t*)(&_timer1) + 0) = TMR1L;                    \
    if (*((uint8_t*)(&_timer1) + 0) == 0x00) {              \
        *((uint8_t*)(&_timer1) + 1) = TMR1H;                \
    }                                                       \
    if (timer1_overflow()) {                                \
        _timer1 = TIMER1_VALUE;                             \
    }                                                       \

#if defined(_16F876A)
#define timer1_overflow() CCP2IF
#elif defined(_16F1936) || defined(_16F1938)
#define timer1_overflow() CCP5IF
#endif

#define enable_interrupts() ei();
#define disable_interrupts() di();

#define int_handler_GLOBAL_begin __interrupt() void HW_isr(void) {

#define int_handler_GLOBAL_end }

#if defined(_16F876A)

#define int_handler_fuel_speed_begin                       \
    /* Was it the port B interrupt on change?*/            \
    if (/*RBIE && */RBIF) {                                \
        /* Dummy read of the port, as per datasheet */     \
        asm("movf PORTB,f");                               \
        /* Reset the interrupt flag */                     \
        RBIF = 0;                                          \

#define int_handler_fuel_speed_end                         \
    }                                                      \

#elif defined(_16F1936) || defined(_16F1938)

#define int_handler_fuel_speed_begin                       \
    /* Was it interrupt on change?*/                       \
    if (/*IOCIE && */IOCIF) {                              \
        /* Clear interrupt flags*/                         \
        IOCBF = 0;                                         \

#define int_handler_fuel_speed_end                         \
    }                                                      \

#endif

#define int_handler_timer0_begin                           \
    /* Timer0 interrupt */                                 \
    if (/*T0IE && */T0IF) {                                \

#define int_handler_timer0_end                             \
        T0IF = 0;                                          \
    }                                                      \

#if defined(_16F876A)

#define int_handler_timer1_begin                           \
    /* Timer1 interrupt */                                 \
    if (/*CCP2IE && */CCP2IF) {                            \
        
#define int_handler_timer1_end                             \
        /* Reset the interrupt flag */                     \
        CCP2IF = 0;                                        \
    }                                                      \

#elif defined(_16F1936) || defined(_16F1938)

#define int_handler_timer1_begin                           \
    /* Timer1 interrupt */                                 \
    if (/*CCP5IE && */CCP5IF) {                            \
        
#define int_handler_timer1_end                             \
        /* Reset the interrupt flag */                     \
        CCP5IF = 0;                                        \
    }                                                      \

#endif

#define int_handler_adc_begin                              \
    /* ADC interrupt */                                    \
    if (/*ADIE && */ADIF) {                                \
    
#define int_handler_adc_end                                \
        /* Reset the interrupt flag */                     \
        ADIF = 0;                                          \
    }                                                      \

#define TX_ACTIVE   (TX == 1)
#define FUEL_ACTIVE (FUEL == 0)

#define KEY1_PRESSED (KEY1 == 0)
#define KEY2_PRESSED (KEY2 == 0)

#define PWR_ON  (PWR = 1)
#define PWR_OFF (PWR = 0)

#define SND_ON  (SND = 1)
#define SND_OFF (SND = 0)

// configure DS18B20_PIN pin as output
#define ONEWIRE_OUTPUT   (ONEWIRE_PIN_Dir = 0)
// configure DS18B20_PIN pin as input
#define ONEWIRE_INPUT    (ONEWIRE_PIN_Dir = 1)

#define ONEWIRE_CLEAR    (ONEWIRE_PIN = 0)
#define ONEWIRE_SET      (ONEWIRE_PIN = 1)
#define ONEWIRE_GET      (ONEWIRE_PIN)

#elif defined(__AVR_ATMEGA)

#define __bit unsigned char
#define __bank0
#define __bank1
#define __bank2
#define __bank3

#define start_timer_fuel() TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (0 << CS00);

#define stop_timer_fuel()  TCCR0B = (0 << WGM02) | (0 << CS02) | (0 << CS01) | (0 << CS00);

// timer1 compare 10ms, 2500 with prescaler 1:64 running at 16Mhz
#define TIMER1_PERIOD 0.01f
#define TIMER1_VALUE 2500

#define get_timer1(_timer1)                         \
    _timer1 = TCNT1;                                \
    if (timer1_overflow()) {                        \
        _timer1 = TIMER1_VALUE;                     \
    }                                               \

#define timer1_overflow() ((TIFR1 & (1 << OCF1A)) != 0)

// start timer with prescaler 1:64
#define start_timer1() TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (1 << WGM12) | (0 << CS12) | (1 << CS11) | (1 << CS10);

#ifdef __XC8
#define enable_interrupts() ei();
#define disable_interrupts() di();
#else
#define enable_interrupts() sei();
#define disable_interrupts() cli();
#endif

#define int_handler_GLOBAL_begin

#define int_handler_GLOBAL_end

#define int_handler_fuel_speed_begin ISR(PCINT0_vect) {     \

#define int_handler_fuel_speed_end }                        \

#define int_handler_encoder_begin ISR(PCINT1_vect) {        \

#define int_handler_encoder_end }                           \

#define int_handler_timer0_begin ISR(TIMER0_COMPA_vect) {   \
    
#define int_handler_timer0_end }                            \

#define int_handler_timer1_begin ISR(TIMER1_COMPA_vect) {   \
    
#define int_handler_timer1_end }                            \

#define int_handler_timer2_begin ISR(TIMER2_COMPA_vect) {   \
    
#define int_handler_timer2_end }                            \

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
// clear OCF1B for ADC Auto Trigger
#define restart_adc_event() TIFR1 = (1 << OCF1B)
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
#define ONEWIRE_SET      (PORTD |= _BV(PORTD7))

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

#endif

#if defined(__AVR)
typedef unsigned int eeaddr_t;
#else
typedef unsigned char eeaddr_t;
#endif

void HW_Init(void);
void HW_read_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length);
void HW_write_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length);

#include "i2c.h"

#endif	/* HW_H */

