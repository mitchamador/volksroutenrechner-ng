#ifndef HW_PIC_H
#define HW_PIC_H

#if defined(__XC8) && (defined(_16F876A) || defined(_16F1936) || defined(_16F1938) || defined(_18F252))

#include <stdint.h>
#include <xc.h>

#if defined(_16F876A)
// PIC16F876A Configuration Bit Settings
// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#elif defined(_16F1936) || defined(_16F1938)
// PIC16F1938 Configuration Bit Settings
// CONFIG1
#pragma config FOSC = HS        // Oscillator Selection (HS Oscillator, High-speed crystal/resonator connected between OSC1 and OSC2 pins)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable (All VCAP pin functionality is disabled)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

#elif defined(_18F252)
// PIC18F252 Configuration Bit Settings

// CONFIG1H
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#pragma config OSCS = OFF       // Oscillator System Clock Switch Enable bit (Oscillator system clock switch option is disabled (main oscillator is source))

// CONFIG2L
#pragma config PWRT = ON        // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOR = ON         // Brown-out Reset Enable bit (Brown-out Reset enabled)
#pragma config BORV = 27        // Brown-out Reset Voltage bits (VBOR set to 2.7V)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 128      // Watchdog Timer Postscale Select bits (1:128)

// CONFIG3H
#pragma config CCP2MUX = ON     // CCP2 Mux bit (CCP2 input/output is multiplexed with RC1)

// CONFIG4L
#pragma config STVR = ON        // Stack Full/Underflow Reset Enable bit (Stack Full/Underflow will cause RESET)
#pragma config LVP = OFF        // Low Voltage ICSP Enable bit (Low Voltage ICSP disabled)

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000200-001FFFh) not code protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot Block (000000-0001FFh) not code protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000200-001FFFh) not write protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0001FFh) not write protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000200-001FFFh) not protected from Table Reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from Table Reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from Table Reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from Table Reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0001FFh) not protected from Table Reads executed in other blocks)

#endif

#if !defined(HW_LEGACY)
#error "only legacy hardware supported with pic16f or pic18f"
#endif

// define cpu frequency
#define _XTAL_FREQ 20000000

#define delay_ms(ms) __delay_ms(ms)
#define delay_us(us) __delay_us(us)

#define __EEDATA(a0,a1,a2,a3,a4,a5,a6,a7) __EEPROM_DATA(a0,a1,a2,a3,a4,a5,a6,a7);

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

typedef unsigned char eeaddr_t;

// i2c software bit bang
#define I2C_SOFTWARE
// lcd parallel interface
#define LCD_LEGACY

// min/max value of adc reading
#define ADC_MIN     0
#define ADC_MAX     1023

// PORTA definitions (a/d channels)
// RA0 as digital power control pin
// RA1 as analog voltage input
// RA2 as analog fuel tank input
// RA5 as digital 1wire pin

// power control pin (RA0)
#if defined(LATA)
#define PWR                     LATAbits.LATA0
#else
#define PWR                     PORTAbits.RA0
#endif
#define PWR_MASK                _PORTA_RA0_MASK

// adc power (AN1/RA1)
#define POWER_SUPPLY_TRIS_MASK  (1 << _TRISA_TRISA1_POSITION)

// fuel tank (AN3/RA3)
#define FUEL_TANK_TRIS_MASK     (1 << _TRISA_TRISA3_POSITION)

// DS18B20 data pin (RA5)
#if defined(LATA)
#define ONEWIRE_PIN_LAT         LATAbits.LATA5
#endif
#define ONEWIRE_PIN             PORTAbits.RA5
#define ONEWIRE_PIN_MASK        _PORTA_RA5_MASK
#define ONEWIRE_PIN_DIR         TRISAbits.TRISA5
#define ONEWIRE_TRIS_MASK       (1 << _TRISA_TRISA5_POSITION)

#if defined(_16F876A) || defined(_18F252)

#define ADC_CHANNEL_MASK            ((1 << _ADCON0_CHS2_POSITION) | (1 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
// AN1
#define ADC_CHANNEL_POWER_SUPPLY    ((0 << _ADCON0_CHS2_POSITION) | (0 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
// AN3
#define ADC_CHANNEL_FUEL_TANK       ((0 << _ADCON0_CHS2_POSITION) | (1 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
#define ADCON0_INIT                 ((1 << _ADCON0_ADCS1_POSITION) | (0 << _ADCON0_ADCS0_POSITION))
// PORTA A/D configuration (AN0/AN1/AN3 as analog input)
#define ADCON1_INIT                 ((0 << _ADCON1_ADCS2_POSITION) | (1 << _ADCON1_ADFM_POSITION) | (0 << _ADCON1_PCFG3_POSITION) | (1 << _ADCON1_PCFG2_POSITION) | (0 << _ADCON1_PCFG1_POSITION) | (0 << _ADCON1_PCFG0_POSITION))
#elif defined(_16F1936) || defined(_16F1938)
#define ADC_CHANNEL_MASK            ((1 << _ADCON0_CHS4_POSITION) | (1 << _ADCON0_CHS3_POSITION) | (1 << _ADCON0_CHS2_POSITION) | (1 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
// AN1
#define ADC_CHANNEL_POWER_SUPPLY    ((0 << _ADCON0_CHS4_POSITION) | (0 << _ADCON0_CHS3_POSITION) | (0 << _ADCON0_CHS2_POSITION) | (0 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
// AN3
#define ADC_CHANNEL_FUEL_TANK       ((0 << _ADCON0_CHS4_POSITION) | (0 << _ADCON0_CHS3_POSITION) | (0 << _ADCON0_CHS2_POSITION) | (1 << _ADCON0_CHS1_POSITION) | (1 << _ADCON0_CHS0_POSITION))
#define ADCON0_INIT                 0
#define ADCON1_INIT                 ((1 << _ADCON1_ADFM_POSITION) | (0 << _ADCON1_ADCS2_POSITION) | (1 << _ADCON1_ADCS1_POSITION) | (0 << _ADCON1_ADCS0_POSITION))
// PORTA AN1/RA1 (adc power) and AN3/RA3 as analog input (fuel tank)
#define ANSELA_INIT                 ((1 << _ANSELA_ANSA1_POSITION) | (1 << _ANSELA_ANSA3_POSITION))
#define ANSELB_INIT                 0

#endif

// start adc
#if defined(_16F876A) || defined(_18F252)
#define start_adc()                 ADCON0bits.GO_DONE = 1;
#else
#define start_adc()                 ADCON0bits.GO = 1;
#endif
// set adc channel
#define set_adc_channel(ch)         ADCON0 = (ADCON0 & ~ADC_CHANNEL_MASK) | ch
// read adc value
#define adc_read_value()    ((uint16_t) (ADRESH << 8) | ADRESL)

// PORTB definitions
// RB0, RB1 - SCL/SDA for software i2c
// RB2, RB3 - keys
// RB6 - speed sensor
// RB7 - fuel injector

#define SDA             PORTBbits.RB0
#define SDA_MASK        _PORTB_RB0_MASK
#define SDA_TRIS        TRISBbits.TRISB0
#define SDA_TRIS_MASK   (1 << _TRISB_TRISB0_POSITION)

#define SCL             PORTBbits.RB1
#define SCL_MASK        _PORTB_RB1_MASK
#define SCL_TRIS        TRISBbits.TRISB1
#define SCL_TRIS_MASK   (0 << _TRISB_TRISB1_POSITION)

#define SDA_INPUT       SDA_TRIS = 1
#define SDA_OUTPUT      SDA_TRIS = 0
#define SDA_HIGH        SDA = 1
#define SDA_LOW         SDA = 0
#define SDA_GET()       (SDA)

#define SCL_INPUT       SCL_TRIS = 1
#define SCL_OUTPUT      SCL_TRIS = 0
#define SCL_HIGH        SCL = 1
#define SCL_LOW         SCL = 0

// key1 and key2 (active ground) (legacy hardware)
#define KEY1            PORTBbits.RB2
#define KEY2            PORTBbits.RB3
#ifdef KEY3_SUPPORT
#define KEY3            PORTBbits.RB4
#define KEY_TRIS_MASK   (1 << _TRISB_TRISB2_POSITION) | (1 << _TRISB_TRISB3_POSITION) | (1 << _TRISB_TRISB4_POSITION)
#define KEY_MASK        (1 << _PORTB_RB2_POSITION) | (1 << _PORTB_RB3_POSITION) | (1 << _PORTB_RB4_POSITION)
#else
#define KEY_TRIS_MASK   (1 << _TRISB_TRISB2_POSITION) | (1 << _TRISB_TRISB3_POSITION)
#define KEY_MASK        (1 << _PORTB_RB2_POSITION) | (1 << _PORTB_RB3_POSITION)
#endif

// speed sensor and injector
#define TX              PORTBbits.RB6
#define TX_TRIS_MASK    (1 << _TRISB_TRISB6_POSITION)
#define FUEL            PORTBbits.RB7
#define FUEL_TRIS_MASK  (1 << _TRISB_TRISB7_POSITION)


// PORTC definitions
// RC0 - sound
#define SND             PORTCbits.RC0
#define SND_TRIS        (1 << _TRISC_TRISC0_POSITION)

#ifdef LCD_LEGACY
// LCD definitions
// rs - RC1
// rw - RC2
// en - RC3
// data - RC4..RC7

#define LCD_DATA(data) PORTC = (PORTC & ~0xF0) | (data & 0xF0);

#define RS_LOW              (PORTCbits.RC1 = 0)
#define RS_HIGH             (PORTCbits.RC1 = 1)
#define RW_LOW              (PORTCbits.RC2 = 0)
#define RW_HIGH             (PORTCbits.RC2 = 1)
#define EN_LOW              (PORTCbits.RC3 = 0)
#define EN_HIGH             (PORTCbits.RC3 = 1)

#endif

#define TX_ACTIVE           (TX == 1)
#define FUEL_ACTIVE         (FUEL == 0)

#define KEY1_PRESSED        (KEY1 == 0)
#define KEY2_PRESSED        (KEY2 == 0)
#ifdef KEY3_SUPPORT
#define KEY3_PRESSED        (KEY3 == 0)
#endif

#define PWR_ON              (PWR = 1)
#define PWR_OFF             (PWR = 0)

#define SND_ON              (SND = 1)
#define SND_OFF             (SND = 0)

// configure DS18B20_PIN pin as output
#define ONEWIRE_OUTPUT      (ONEWIRE_PIN_DIR = 0)
// configure DS18B20_PIN pin as input
#define ONEWIRE_INPUT       (ONEWIRE_PIN_DIR = 1)

#if defined(ONEWIRE_PIN_LAT)
#define ONEWIRE_CLEAR       (ONEWIRE_PIN_LAT = 0)
#else
// RA0 is configured as analog pin, so read-modify-write PORTA (set or clear onewire pin) forces RA0 to 0 (analog inputs read as '0')
#define ONEWIRE_CLEAR       PORTA = (PORTA & ONEWIRE_PIN_MASK) | PWR_MASK;
#endif
#define ONEWIRE_GET         (ONEWIRE_PIN)

// init values for port's data direction
#define TRISA_INIT ONEWIRE_TRIS_MASK | POWER_SUPPLY_TRIS_MASK | FUEL_TANK_TRIS_MASK
#define TRISB_INIT KEY_TRIS_MASK | TX_TRIS_MASK | FUEL_TRIS_MASK | SCL_TRIS_MASK | SDA_TRIS_MASK
#define TRISC_INIT 0

// init values for port's data
#define PORTA_INIT PWR_MASK
#define PORTB_INIT KEY_MASK | SDA_MASK | SCL_MASK
#define PORTC_INIT 0

// timer1 compare 10ms, 6250 with prescaler 1:8 at 20MHz
#define TIMER_MAIN_PERIOD 0.01f
#define TIMER_MAIN_TICKS_PER_PERIOD 6250

/* ======================================= */

#if defined(_18F252)
#define start_fuel_timer()  (TMR0ON = 1)
#define stop_fuel_timer()   (TMR0ON = 0)
#else
#define start_fuel_timer()  (T0CS = 0)
#define stop_fuel_timer()   (T0CS = 1)
#endif

#define start_main_timer()  (TMR1ON = 1)

#define enable_interrupts() ei();
#define disable_interrupts() di();

#if defined(_16F876A) || defined(_18F252)
#define PORT_CHANGE_IF          RBIF
#define PORT_CHANGE_CLEAR_IF    RBIF
#define TIMER_MAIN_IF           CCP2IF
#elif defined(_16F1936) || defined(_16F1938)
#define PORT_CHANGE_IF          IOCIF
#define PORT_CHANGE_CLEAR_IF    IOCBF
#define TIMER_MAIN_IF           CCP5IF
#endif

#if defined(_18F252)
#define TIMER_FUEL_IF           TMR0IF
#else
#define TIMER_FUEL_IF           T0IF
#endif

#else
#error "device not supported"
#endif

#endif /* HW_PIC_H */