#ifndef MAIN_H
#define	MAIN_H

#include "hw.h"

// use temp sensor from ds3231
//#define DS3231_TEMP

#ifdef DS3231_TEMP
#define NO_DS18B20
#endif

// disable ds18b20 support with external defines
#ifndef NO_DS18B20
#define DS18B20_SUPPORT
#endif

#ifdef DS18B20_SUPPORT
// disable ds18b20 config screen with external defines
#ifndef NO_DS18B20_CONFIG
#define DS18B20_CONFIG
#endif
#endif

#if defined(DS18B20_SUPPORT) || defined(DS3231_TEMP)
#define TEMPERATURE_SUPPORT
#endif

// temperature timeout
#define TIMEOUT_TEMPERATURE (15 - 1)

// disable sound support with external defines
#ifndef NO_SOUND
#define SOUND_SUPPORT
#endif

// adc voltage filtering (no legacy)
#define ADC_VOLTAGE_FILTERING

// auto calculate day of week
#define AUTO_DAY_OF_WEEK

// min speed settings
#define MIN_SPEED_CONFIG

#if defined(HW_LEGACY)
#if defined(LOW_MEM_DEVICE)
// simple checking time difference (decrease memory usage)
#define SIMPLE_TRIPC_TIME_CHECK
// auto calculate day of week
//#undef AUTO_DAY_OF_WEEK
// min speed settings
//#undef MIN_SPEED_CONFIG
#else
// adc voltage filtering value (power of 2)
//#define ADC_VOLTAGE_FILTER_VALUE 2
#endif
#endif

// power supply threshold 
// with default divider resistor's (8,2k (to Vcc) + 3,6k (to GND)) values
// THRESHOLD_VOLTAGE * (3,6 / (3,6 + 8,2)) * (1024 / 5) = THRESHOLD_VOLTAGE_ADC_VALUE
// 2,048V ~ 128
#define THRESHOLD_VOLTAGE_ADC_VALUE 128
// threshold for buttons +-0,2v
#define ADC_BUTTONS_THRESHOLD 40
#define ADC_BUTTONS_1V (1024/5)            

// misc constants (in seconds)
#define MAIN_INTERVAL ((uint8_t) (2.0f / TIMER1_PERIOD))
#define DEBOUNCE ((uint8_t) (0.04f / TIMER1_PERIOD))
#define SHORTKEY ((uint8_t) (0.5f / TIMER1_PERIOD))
#define LONGKEY ((uint8_t) (1.0f / TIMER1_PERIOD))

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
// min rpm
#define TAHO_MIN_RPM 100UL
// min rpm constant (1/(TAHO_MIN_RPM/60sec)/0.01s) 0.01s timer overflow
#define TAHO_OVERFLOW ((uint8_t) ((1.0f / (TAHO_MIN_RPM / 60.0f) ) / TIMER1_PERIOD))
// taho const 
#define TAHO_CONST ((uint32_t) (60 / TIMER1_PERIOD * TIMER1_VALUE))

// print speed while acceleration's measurement 
#define PRINT_SPEED100
// timer1 counts between speed pulses when speed is 100 km/h
// (1 / ((config.odo_const * 100) / 3600)) / (0.01f/TIMER1_VALUE) = (36 / (0.01f / TIMER1_VALUE) / config.odo_const
#define SPEED100_CONST ((uint32_t) (36 / (TIMER1_PERIOD / TIMER1_VALUE)))

// minimum pulse width for speed100 calculation (10 * 0.01s)
#if (65536 / TIMER1_VALUE) >= 10
#define SPEED100_OVERFLOW 10
#else
#define SPEED100_OVERFLOW (65536 / TIMER1_VALUE)
#endif

typedef struct {
    uint8_t day;
    uint8_t month;
    uint8_t year;
} service_time_t;

typedef struct {
    uint16_t counter;
    uint8_t limit;
    service_time_t time;
    uint8_t dummy[2]; // fill to 8 bytes size
} srv_t;

typedef struct {
    uint32_t counter;
    uint32_t counter_rpm;
    uint16_t limit;
} srv_mh_t;

typedef union {
    // a structure with 8 single bit bit-field objects, overlapping the union member "byte"
    uint8_t byte;

    struct {
        unsigned alt_buttons : 1;
        unsigned daily_tripc : 1;
        unsigned mh_rpm : 1;
        unsigned fast_refresh : 1;
        unsigned service_alarm : 1;
        unsigned key_sound : 1;
        unsigned skip_temp_screen : 1;
        unsigned dual_injection : 1;
    };
} settings_u;

typedef struct {
    uint16_t odo;
    uint16_t odo_temp;
    uint8_t fuel_tmp1, fuel_tmp2;
    uint16_t fuel;
    uint32_t time;
} trip_t;

typedef struct {
    uint8_t minute, hour, day, month, year;
} trip_time_t;

// service counters limits

typedef struct {
    // main odometer
    uint32_t odo;
    uint16_t odo_temp;

    uint16_t odo_const;
    uint8_t fuel_const;
    uint8_t vcc_const;

    settings_u settings;

    // param counter for main screen
    uint8_t selected_param1;
    // param counter for tripC screen
    uint8_t selected_param2;

    // min speed for drive mode
    uint8_t min_speed;

    uint8_t dummy[2];

} config_t;

typedef struct {
    srv_mh_t mh;
    srv_t srv[4];
} services_t;

typedef struct {
    trip_t tripA, tripB, tripC;
    trip_time_t tripC_time;
    uint16_t tripC_max_speed;
} trips_t;

#define EEPROM_DS18B20_ADDRESS (((sizeof(config_t) - 1) / 8 + 1) * 8) + (((sizeof(trips_t) - 1) / 8 + 1) * 8) + (((sizeof(services_t) - 1) / 8 + 1) * 8)
#define EEPROM_CUSTOM_CHARS_ADDRESS (EEPROM_DS18B20_ADDRESS + 8 * 3)

typedef struct {
    void (*screen)(void);
} screen_item_t;

typedef struct {
    uint8_t str_index;
    void (*screen)(void);
} service_screen_item_t;

#endif	/* MAIN_H */

