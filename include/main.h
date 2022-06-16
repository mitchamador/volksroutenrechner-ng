#ifndef MAIN_H
#define	MAIN_H

#if defined(__AVR)

#ifdef ARDUINO
// 1602 lcd i2c
#define LCD_1602_I2C
// adc buttons connected to PC0/ADC0
#define ADC_BUTTONS
#else
// 1602 lcd 4bit
#define LCD_LEGACY
#endif

// support for prev key
#define KEY3_SUPPORT

#endif

#include <hw.h>

// disable temperature support
#if !defined(NO_TEMPERATURE_SUPPORT)

// use temp sensor from ds3231
#ifndef NO_DS3231
#define DS3231_TEMP
#endif

// disable ds18b20 support
#ifndef NO_DS18B20
#define DS18B20_TEMP
#endif

#ifdef DS18B20_TEMP
// disable ds18b20 config screen
#ifndef NO_DS18B20_CONFIG
#define DS18B20_CONFIG
#endif
#endif

#if defined(DS18B20_TEMP) || defined(DS3231_TEMP)
#define TEMPERATURE_SUPPORT
#endif

// temperature timeout
#define TIMEOUT_TEMPERATURE (15 - 1)

#endif

// disable all service counters support
#ifndef NO_SERVICE_COUNTERS
#define SERVICE_COUNTERS_SUPPORT
#endif

// disable service counters' configuration and checking
#ifndef NO_SERVICE_COUNTERS_CONFIG
#define SERVICE_COUNTERS_CONFIG_SUPPORT
#endif

// disable sound support
#ifndef NO_SOUND
#define SOUND_SUPPORT
#endif

// fuel duration measurement
#define FUEL_DURATION
// 1/10 rounding
//#define FUEL_DURATION_SMALL_FRACTION

// auto calculate day of week
#define AUTO_DAY_OF_WEEK

// min speed settings
#define MIN_SPEED_CONFIG

#if defined(HW_LEGACY)

// simple adc handler
#define SIMPLE_ADC

// pic16f876a undefs
#if defined(LOW_MEM_DEVICE) && defined(SERVICE_COUNTERS_SUPPORT)

#if defined(DS18B20_TEMP) && defined(DS3231_TEMP)
// ds3231 temperature sensor support
#undef DS3231_TEMP
#endif

#if defined(DS18B20_TEMP) && defined(SERVICE_COUNTERS_CONFIG_SUPPORT)

// skip oled lcd reset sequence (though works ok without it after power up with EH1602 REV.J)
#define NO_LCD_OLED_RESET
// simple checking time difference (decrease memory usage)
#define SIMPLE_TRIPC_TIME_CHECK

#endif

// fuel duration measurement
#if defined(DS18B20_CONFIG) && defined(SERVICE_COUNTERS_CONFIG_SUPPORT)
#undef FUEL_DURATION
#endif

// auto calculate day of week
#if defined(FUEL_DURATION) && defined(DS18B20_TEMP)
//#undef AUTO_DAY_OF_WEEK
#endif

// min speed settings
//#undef MIN_SPEED_CONFIG

#endif

#endif


// power supply threshold 
// with default divider resistor's (8,2k (to Vcc) + 3,6k (to GND)) values
// THRESHOLD_VOLTAGE * (3,6 / (3,6 + 8,2)) * (1024 / 5) = THRESHOLD_VOLTAGE_ADC_VALUE
// 2,048V ~ 128
#define THRESHOLD_VOLTAGE_ADC_VALUE 128
#ifdef ADC_BUTTONS
// threshold for buttons +-0,2v
#define ADC_BUTTONS_THRESHOLD 40
#define ADC_BUTTONS_1V (1024/5)            
#endif

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
    // a structure with 16 single bit bit-field objects, overlapping the union member "word"
    uint16_t word;

    struct {
        unsigned dummy : 7;
        unsigned ds3231_temp : 1;
        unsigned show_inner_temp : 1;
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

typedef union {
    uint8_t byte;
    
    struct {
        uint8_t main_param : 4;
        uint8_t service_param : 4;
    };
} param_u;

// settings (16 bytes)
typedef struct {
    // main odometer
    uint32_t odo;
    // fractional part of main odometer
    uint16_t odo_temp;

    // odo const
    uint16_t odo_const;
    // fuel const
    uint8_t fuel_const;
    // vcc const
    uint8_t vcc_const;

    // settings (uint16_t)
    settings_u settings;

    // selected params
    param_u selected_param;

    // min speed for drive mode
    uint8_t min_speed;

    // dummy bytes
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
} config_screen_item_t;

#endif	/* MAIN_H */

