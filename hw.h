/* 
 * File:   hw.h
 * Author: victor
 *
 * Created on 4 июня 2021 г., 23:50
 */

#ifndef HW_H
#define	HW_H

#include <xc.h>

#include <stdint.h>
#include "i2c.h"

//#pragma warning disable 1090

// define cpu frequency
#define _XTAL_FREQ 20000000
// define i2c bus frequency
#define I2C_BaudRate 100000

// PORTA definitions (analog input)
#define POWER_SUPPLY PORTAbits.RA1
#define POWER_SUPPLY_TRIS (1 << _TRISA_TRISA1_POSITION)
#define PWR PORTAbits.RA0
#define ONEWIRE PORTAbits.RA5

#define POWER_SUPPLY_ACTIVE (POWER_SUPPLY == 1)
#define PWR_ON PWR = 1
#define PWR_OFF PWR = 0

// PORTB definitions
// key1 and key2 (active ground)
#define KEY1 PORTBbits.RB2
#define KEY2 PORTBbits.RB3
#define KEY_TRIS (1 << _TRISB_TRISB2_POSITION) | (1 << _TRISB_TRISB3_POSITION)

#define KEY1_PRESSED (KEY1 == 0)
#define KEY2_PRESSED (KEY2 == 0)

// speed sensor and injector
#define TX PORTBbits.RB6
#define TX_TRIS (1 << _TRISB_TRISB6_POSITION)
#define FUEL PORTBbits.RB7
#define FUEL_TRIS (1 << _TRISB_TRISB7_POSITION)

#define TX_ACTIVE TX == 1
#define FUEL_ACTIVE FUEL == 0

// DS18B20 data pin is connected to pin RA5
#define DS18B20_PIN      RA5
#define DS18B20_PIN_Dir  TRISA5

#define SND     RC0
#define SND_TRIS (1 << _TRISC_TRISC0_POISITION)

#define SND_ON SND = 1
#define SND_OFF SND = 0

#if defined(_16F1936)
#define RBIF IOCIF
#define RBIE IOCIE
#define GO_DONE GO_nDONE
#define _INTCON_RBIE_POSITION _INTCON_IOCIE_POSITION
#define _OPTION_REG_nRBPU_POSITION _OPTION_REG_nWPUEN_POSITION
#endif

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// timer1 overflow 10ms
#define TIMER1_VALUE (65536 - 6250)

#define start_timer_fuel() T0CS = 0
#define stop_timer_fuel() T0CS = 1
#define start_timer_taho() TMR2 = 0; \
                           TMR2IF = 0; \
                           TMR2ON = 1

#define stop_timer_taho() TMR2ON = 0

#define enable_interrupts() ei();
#define disable_interrupts() di();

#define int_handler_GLOBAL_begin __interrupt() void HW_isr(void) {

#define int_handler_GLOBAL_end }
                                                    \

#define int_handler_fuel_speed_begin                       \
    /* Was it the port B interrupt on change?*/            \
    if (/*RBIE && */RBIF) {                                \
        /* Dummy read of the port, as per datasheet */     \
        asm("movf PORTB,f");                               \

#define int_handler_fuel_speed_end                         \
        /* Reset the interrupt flag */                     \
        RBIF = 0;                                          \
    }                                                      \

#define int_handler_timer0_begin                           \
    /* Timer0 interrupt */                                 \
    if (/*T0IE && */T0IF) {                                \

#define int_handler_timer0_end                             \
        T0IF = 0;                                          \
    }                                                      \

#define int_handler_timer1_begin                           \
    /* Timer1 interrupt */                                 \
    if (/*TMR1IE && */TMR1IF) {                            \
        TMR1 = TIMER1_VALUE;                               \
        
#define int_handler_timer1_end                             \
        /* Reset the interrupt flag */                     \
        TMR1IF = 0;                                        \
    }                                                      \

#define int_handler_timer2_begin                           \
    /* Timer2 interrupt */                                 \
    if (/*TMR2IE && */TMR2IF) {                            \
    
#define int_handler_timer2_end                             \
        /* Reset the interrupt flag */                     \
        TMR2IF = 0;                                        \
    }                                                      \

uint16_t HW_adc_read(void);
void HW_Init(void);
void HW_read_eeprom_block(unsigned char* p, unsigned char ee_addr, unsigned char length);
void HW_write_eeprom_block(unsigned char* p, unsigned char ee_addr, unsigned char length);

#endif	/* HW_H */

