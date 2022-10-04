#ifndef MAIN_H
#define	MAIN_H

#include "core.h"

// temperature timeout
#define TIMEOUT_TEMPERATURE (30 - 1)
#define FORCED_TIMEOUT_TEMPERATURE (5 - 1)

// const for voltage adjust
#define VOLTAGE_ADJUST_CONST_MIN 140
#define VOLTAGE_ADJUST_CONST_MAX 230

// default min speed for drive mode (km/h)
#define MIN_SPEED_DEFAULT 5

//show average speed (or fuel consumption) after distance AVERAGE_MIN_DIST * 0.1 km
#define AVERAGE_MIN_DIST 3

// show average fuel consumption after total consumption of AVERAGE_MIN_FUEL * 0,01 litres
#define AVERAGE_MIN_FUEL 5

// max value of trip A odometer
#define MAX_ODO_TRIPA 2000

// max value of trip B odometer
#define MAX_ODO_TRIPB 6000

// max pause for continuing trip C
#define TRIPC_PAUSE_MINUTES 120

// round taho
#define TAHO_ROUND 10
// taho const 
#define TAHO_CONST ((uint32_t) (60 / TAHO_TIMER_PERIOD * TAHO_TIMER_TICKS_PER_PERIOD))

// speed timer counts between speed pulses when speed is X km/h
// (1 / ((config.odo_const * X) / 3600)) / (SPEED_TIMER_PERIOD / SPEED_TIMER_TICKS_PER_PERIOD) = ((3600 / X) / (SPEED_TIMER_PERIOD / SPEED_TIMER_TICKS_PER_PERIOD) / config.odo_const
#define speed_const(x) ((uint32_t) ((3600 / x) / (SPEED_TIMER_PERIOD / SPEED_TIMER_TICKS_PER_PERIOD)))

typedef struct {
    void (*screen)(void);
    uint8_t index;
} screen_item_t;

typedef struct {
    void (*screen)(void);
    uint8_t title_string_index;
} screen_config_item_t;

// ds18b20 temperatures
#define TEMP_NONE -1
#define TEMP_OUT 0
#define TEMP_IN 1
#define TEMP_ENGINE 2
#define TEMP_CONFIG 3

#define PRINT_TEMP_PARAM_HEADER       0x80
#define PRINT_TEMP_PARAM_FRACT        0x40
#define PRINT_TEMP_PARAM_NO_PLUS_SIGN 0x20
#define PRINT_TEMP_PARAM_DEG_SIGN     0x10
#define PRINT_TEMP_PARAM_MASK         0x0F

typedef struct {
    uint8_t *p;         // value pointer
    uint8_t min;        // min value
    uint8_t max;        // max value
    uint8_t pos;        // cursor position
} time_editor_item_t;

#endif	/* MAIN_H */
