#ifndef HW_H
#define	HW_H

#include "config.h"

// main timer period 10ms
#define HW_MAIN_TIMER_PERIOD               0.01f

#if defined(_MPC_)
// pic
#include "hw-pic.h"
#elif defined(__AVR)
// avr
#include "hw-avr.h"
#else
#error "device not supported"
#endif

#if !defined(TAHO_TIMER)
// use main timer for taho
#define HW_TAHO_TIMER_PERIOD               HW_MAIN_TIMER_PERIOD
#define HW_TAHO_TIMER_TICKS_PER_PERIOD     HW_MAIN_TIMER_TICKS_PER_PERIOD
#define taho_timer                         main_timer
#endif

#if !defined(SPEED_TIMER)
// use main timer for speed
#define HW_SPEED_TIMER_PERIOD              HW_MAIN_TIMER_PERIOD
#define HW_SPEED_TIMER_TICKS_PER_PERIOD    HW_MAIN_TIMER_TICKS_PER_PERIOD
#define speed_timer                        main_timer
#endif

void HW_Init(void);
void HW_read_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length);
void HW_write_eeprom_block(unsigned char* p, eeaddr_t ee_addr, unsigned char length);

#include "i2c.h"

#endif	/* HW_H */

