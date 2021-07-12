#ifndef HW_H
#define	HW_H

#if defined(__XC8)
#include <xc.h>
#if defined(_16F876A) || defined(_16F1936)
#define __PIC_MIDRANGE
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

// define cpu frequency
#if defined(__PIC_MIDRANGE)
#define delay_ms(ms) __delay_ms(ms)
#define delay_us(us) __delay_us(us)
#define _XTAL_FREQ 20000000

#define __START_EEPROM_DATA
#define __END_EEPROM_DATA

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

#define CONCATE1(X,Y) X##Y
#define CONCATE(X,Y) CONCATE1(X,Y)

#define __EEPROM_DATA(a0,a1,a2,a3,a4,a5,a6,a7) volatile const EEMEM char CONCATE(a_,__COUNTER__)[8] = {a0,a1,a2,a3,a4,a5,a6,a7}

#endif

#include <stdint.h>

#if defined(__PIC_MIDRANGE)

#ifdef HW_LEGACY
#if !defined(_16F876A)
#error ("hw legacy support only with pic16f876a")
#endif
// i2c software bit bang
#define I2C_BITBANG
// lcd parallel interface
#define LCD_LEGACY
#endif

//#pragma warning disable 1090

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)


// PORTA definitions (analog input)
#define POWER_SUPPLY PORTAbits.RA0
#define POWER_SUPPLY_TRIS_MASK (1 << _TRISA_TRISA0_POSITION)
#define ADC_BUTTONS PORTAbits.RA1
#define ADC_BUTTONS_TRIS_MASK (1 << _TRISA_TRISA1_POSITION)
#if defined(_16F876A)
#define ADC_CHANNEL_MASK ((1 << _ADCON0_CHS2_POSITION) | (1 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
#define ADC_CHANNEL_POWER_SUPPLY ((0 << _ADCON0_CHS2_POSITION) | (0 << _ADCON0_CHS1_POSITION) | (0 << _ADCON0_CHS0_POSITION))
#define ADC_CHANNEL_BUTTONS ((0 << _ADCON0_CHS2_POSITION) | (0 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
#define ADCON0_INIT ((1 << _ADCON0_ADCS1_POSITION) | (0 << _ADCON0_ADCS0_POSITION))
#define ADCON1_INIT ((1 << _ADCON1_ADFM_POSITION) | (1 << _ADCON1_PCFG3_POSITION) | (1 << _ADCON1_PCFG2_POSITION) | (1 << _ADCON1_PCFG1_POSITION) | (0 << _ADCON1_PCFG0_POSITION))
#else
#define ADC_CHANNEL_MASK ((1 << _ADCON0_CHS4_POSITION) | (1 << _ADCON0_CHS3_POSITION) | (1 << _ADCON0_CHS2_POSITION) | (1 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
#define ADC_CHANNEL_POWER_SUPPLY ((0 << _ADCON0_CHS4_POSITION) | (0 << _ADCON0_CHS3_POSITION) | (0 << _ADCON0_CHS2_POSITION) | (0 << _ADCON0_CHS1_POSITION) | (0 << _ADCON0_CHS0_POSITION))
#define ADC_CHANNEL_BUTTONS ((0 << _ADCON0_CHS4_POSITION) | (0 << _ADCON0_CHS3_POSITION) | (0 << _ADCON0_CHS2_POSITION) | (0 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
#define ADCON0_INIT 0
#define ADCON1_INIT ((1 << _ADCON1_ADFM_POSITION) | (0 << _ADCON1_ADCS2_POSITION) | (1 << _ADCON1_ADCS1_POSITION) | (0 << _ADCON1_ADCS0_POSITION))
#endif

#define set_adc_channel(ch) ADCON0 = (ADCON0 & ~ADC_CHANNEL_MASK) | ch

#define PWR PORTAbits.RA4
#define PWR_MASK  (1 << _PORTA_RA4_POSITION)

// PORTB definitions
#ifdef HW_LEGACY
// key1 and key2 (active zero) (legacy hardware)
#define KEY1 PORTBbits.RB2
#define KEY2 PORTBbits.RB3
#define KEY_TRIS_MASK (1 << _TRISB_TRISB2_POSITION) | (1 << _TRISB_TRISB3_POSITION)
#else
// encoder clock and data (new hardware)
#define ENCODER_CLK PORTBbits.RB4
#define ENCODER_DATA PORTBbits.RB5
#define KEY_TRIS_MASK (1 << _TRISB_TRISB4_POSITION) | (1 << _TRISB_TRISB5_POSITION)
#endif

// speed sensor and injector
#define TX PORTBbits.RB6
#define TX_TRIS_MASK (1 << _TRISB_TRISB6_POSITION)
#define FUEL PORTBbits.RB7
#define FUEL_TRIS_MASK (1 << _TRISB_TRISB7_POSITION)

// DS18B20 data pin is connected to pin RA5
#define ONEWIRE_PIN      RA5
#define ONEWIRE_PIN_Dir  TRISA5

#define SND     RC0
#define SND_TRIS (1 << _TRISC_TRISC0_POISITION)

#ifndef I2C_BITBANG
#define SCL_TRIS_MASK   (1 << _TRISC_TRISC3_POSITION)
#define SDA_TRIS_MASK   (1 << _TRISC_TRISC4_POSITION)
#else
#define SDA       PORTBbits.RB0
#define SDA_TRIS  TRISBbits.TRISB0
#define SDA_TRIS_MASK   0

#define SCL       PORTBbits.RB1
#define SCL_TRIS  TRISBbits.TRISB1
#define SCL_TRIS_MASK   0
#endif

#ifdef LCD_LEGACY
// RS - RC1
// EN - RC3
// data - RC4..RC7
#define LCD_PORT    PORTC
#define RS (1 << 1)
#define EN (1 << 3)
#define LCD_PORT_MASK (0xF0 | RS | EN )
#endif

// init values for port's data direction
#define TRISA_INIT POWER_SUPPLY_TRIS_MASK | ADC_BUTTONS_TRIS_MASK
#define TRISB_INIT KEY_TRIS_MASK | TX_TRIS_MASK | FUEL_TRIS_MASK
#define TRISC_INIT SCL_TRIS_MASK | SDA_TRIS_MASK

// init values for port's data
#define PORTA_INIT PWR_MASK
#define PORTB_INIT 0
#define PORTC_INIT 0

// timer1 compare 10ms
#define TIMER1_VALUE 6250

/* ======================================= */

#define adc_read_value() ((uint16_t) (ADRESH << 8) | ADRESL)

#define start_timer_fuel() T0CS = 0
#define stop_timer_fuel() T0CS = 1
#define start_timer_taho() TMR2 = 0; \
                           TMR2IF = 0; \
                           TMR2ON = 1

#define stop_timer_taho() TMR2ON = 0

#define start_timer1() TMR1ON = 1

#define enable_interrupts() ei();
#define disable_interrupts() di();

#define int_handler_GLOBAL_begin __interrupt() void HW_isr(void) {

#define int_handler_GLOBAL_end }
                                                    \
#if defined(_16F876A)

#define int_handler_fuel_speed_begin                       \
    /* Was it the port B interrupt on change?*/            \
    if (/*RBIE && */RBIF) {                                \
        /* Dummy read of the port, as per datasheet */     \
        asm("movf PORTB,f");                               \

#define int_handler_fuel_speed_end                         \
        /* Reset the interrupt flag */                     \
        RBIF = 0;                                          \
    }                                                      \

#elif defined(_16F1936)

#define int_handler_fuel_speed_begin                       \
    /* Was it interrupt on change?*/            \
    if (/*IOCIE && */IOCIF) {                                \

#define int_handler_fuel_speed_end                         \
        /* Reset the interrupt flag */                     \
        IOCIF = 0;                                          \
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

#elif defined(_16F1936)

#define int_handler_timer1_begin                           \
    /* Timer1 interrupt */                                 \
    if (/*CCP5IE && */CCP5IF) {                            \
        
#define int_handler_timer1_end                             \
        /* Reset the interrupt flag */                     \
        CCP5IF = 0;                                        \
    }                                                      \

#endif

#define int_handler_timer2_begin                           \
    /* Timer2 interrupt */                                 \
    if (/*TMR2IE && */TMR2IF) {                            \
    
#define int_handler_timer2_end                             \
        /* Reset the interrupt flag */                     \
        TMR2IF = 0;                                        \
    }                                                      \

#define int_handler_adc_begin                              \
    /* ADC interrupt */                                    \
    if (/*ADIE && */ADIF) {                                \
    
#define int_handler_adc_end                                \
        /* Reset the interrupt flag */                     \
        ADIF = 0;                                          \
    }                                                      \

#define TX_ACTIVE   (TX == 1)
#define FUEL_ACTIVE (FUEL == 0)

#ifdef HW_LEGACY
#define KEY1_PRESSED (KEY1 == 0)
#define KEY2_PRESSED (KEY2 == 0)
#endif

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
#define ONEWIRE_VALUE(v) (ONEWIRE_PIN = v)
#define ONEWIRE_GET      (ONEWIRE_PIN)

#elif defined(__AVR_ATMEGA)

#define __bit unsigned char
#define __bank0
#define __bank1
#define __bank2
#define __bank3

#define start_timer_fuel() TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (0 << CS00);

#define stop_timer_fuel()  TCCR0B = (0 << WGM02) | (0 << CS02) | (0 << CS01) | (0 << CS00);

#define start_timer_taho() TCNT2 = 0; TCCR2B = (0 << WGM22) | (0 << CS22) | (1 << CS21) | (0 << CS20);

#define stop_timer_taho()  TCCR2B = (0 << WGM22) | (0 << CS22) | (0 << CS21) | (0 << CS20);

#define start_timer1() TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);

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

#define int_handler_encoder_begin ISR(PCINT2_vect) {        \

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
// mux for power supply pin (PC1/ADC1)
#define ADC_CHANNEL_POWER_SUPPLY ((0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0))
// mux for buttons pin (PC0/ADC0)
#define ADC_CHANNEL_BUTTONS ((0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0))

#define PWR_ON  (PORTC |=  _BV(PORTD3))
#define PWR_OFF (PORTC &= ~_BV(PORTD3))

#define SND_ON  (PORTD |=  _BV(PORTD4))
#define SND_OFF (PORTD &= ~_BV(PORTD4))

#define ONEWIRE_CLEAR    (PORTD &= ~_BV(PORTD5))
#define ONEWIRE_SET      (PORTD |= _BV(PORTD5))

#define ONEWIRE_VALUE(v)                        \
    if ((v & 0x01) != 0) {                      \
        ONEWIRE_CLEAR; DDRD &= ~_BV(DDD5);      \
    } else {                                    \
        ONEWIRE_CLEAR;                          \
    }                                           \

#define ONEWIRE_GET      ((PIND & _BV(PIND5)) != 0 ? 1 : 0)
// configure DS18B20_PIN pin as output
#define ONEWIRE_OUTPUT   (DDRD |= _BV(DDD5))
// configure DS18B20_PIN pin as input
#define ONEWIRE_INPUT    ONEWIRE_CLEAR; DDRD &= ~_BV(DDD5)

// encoder clk (PD6/PCINT22) and data (PD7/PCINT23)
#define ENCODER_CLK  ((PIND & _BV(PIND6)) != 0 ? 1 : 0)
#define ENCODER_DATA ((PIND & _BV(PIND7)) != 0 ? 1 : 0)

// init values for port's data direction
#define DDRB_INIT 0
#define DDRC_INIT 0
#define DDRD_INIT _BV(DDD3) | _BV(DDD4)

// init values for port's data 
#define PORTB_INIT _BV(PORTB1) | _BV(PORTB0)
#define PORTC_INIT _BV(PORTC0) | _BV(PORTC1)
#define PORTD_INIT 0

#endif

void HW_Init(void);
void HW_read_eeprom_block(unsigned char* p, unsigned char ee_addr, unsigned char length);
void HW_write_eeprom_block(unsigned char* p, unsigned char ee_addr, unsigned char length);

#endif	/* HW_H */

