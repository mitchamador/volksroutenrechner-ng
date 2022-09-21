#ifndef MAIN_H
#define	MAIN_H

#include "config.h"
#include "hw.h"

// temperature timeout
#define TIMEOUT_TEMPERATURE (30 - 1)
#define FORCED_TIMEOUT_TEMPERATURE (5 - 1)

// ds1307 read timeout
#define TIMEOUT_DS_READ 10

// power supply threshold 
// with default divider resistor's (8,2k (to Vcc) + 3,6k (to GND)) values
// THRESHOLD_VOLTAGE * (3,6 / (3,6 + 8,2)) * (1024 / 5) = THRESHOLD_VOLTAGE_ADC_VALUE
// 2,048V ~ 128
#define THRESHOLD_VOLTAGE_ADC_VALUE 128

// const for voltage adjust
#define VOLTAGE_ADJUST_CONST_MIN 140
#define VOLTAGE_ADJUST_CONST_MAX 230

#ifdef ADC_BUTTONS
// threshold for buttons +-0,2v
#define ADC_BUTTONS_THRESHOLD 40
#define ADC_BUTTONS_1V (1024/5)            
#endif

// misc constants (in seconds)
#define MAIN_INTERVAL ((uint8_t) (1.0f / TIMER_MAIN_PERIOD))
#define DEBOUNCE ((uint8_t) (0.04f / TIMER_MAIN_PERIOD))
#define SHORTKEY ((uint8_t) (0.5f / TIMER_MAIN_PERIOD))
#define LONGKEY ((uint8_t) (1.0f / TIMER_MAIN_PERIOD))
#define KEY_REPEAT_PAUSE ((uint8_t) (0.15f / TIMER_MAIN_PERIOD))
// timeout constant in 0.01 ms resolution
#define INIT_TIMEOUT(t) ((uint8_t) (t * 10.0f * 0.1f / TIMER_MAIN_PERIOD))
// time with power supply measurements lower than threshold before shutdown
#define SHUTDOWN ((uint8_t) (0.25f / TIMER_MAIN_PERIOD))

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
// min rpm
#define TAHO_MIN_RPM 100UL
// min rpm constant (1/(TAHO_MIN_RPM/60sec)/0.01s) 0.01s timer overflow
#define TAHO_OVERFLOW ((uint8_t) ((1.0f / (TAHO_MIN_RPM / 60.0f) ) / TIMER_MAIN_PERIOD))
// taho const 
#define TAHO_CONST ((uint32_t) (60 / TIMER_MAIN_PERIOD * TIMER_MAIN_TICKS_PER_PERIOD))

// timer1 counts between speed pulses when speed is X km/h
// (1 / ((config.odo_const * X) / 3600)) / (0.01f/TIMER_MAIN_TICKS_PER_PERIOD) = ((3600 / X) / (0.01f / TIMER_MAIN_TICKS_PER_PERIOD) / config.odo_const
#define speed_const(x) ((uint32_t) ((3600 / x) / (TIMER_MAIN_PERIOD / TIMER_MAIN_TICKS_PER_PERIOD)))

// minimum pulse width for acceleration measurement calculation (10 * 0.01s)
#if (65536 / TIMER_MAIN_TICKS_PER_PERIOD) >= 10
#define ACCEL_MEAS_OVERFLOW 10
#else
#define ACCEL_MEAS_OVERFLOW (65536 / TIMER_MAIN_TICKS_PER_PERIOD)
#endif

typedef union {
    // a structure with 16 single bit bit-field objects, overlapping the union member "word"
    uint16_t word;

    struct {
        unsigned dummy : 6;
        unsigned adc_fuel_normalize : 1;    // normalize adc_fuel to adc_voltage
        unsigned ds3231_temp : 1;           // use ds3231 temperature as inner
        unsigned show_inner_temp : 1;       // show inner (outer by default) temperature on first screen
        unsigned daily_tripc : 1;           // use trip C as dayly counter (auto reset on next day)
        unsigned monthly_tripb : 1;         // use trip B as monthly counter (auto reset on next month)
        unsigned mh_rpm : 1;                // show motorhours based on rpm (96000 per hour)
        unsigned service_alarm : 1;         // alarm for service counters
        unsigned key_sound : 1;             // keys sound
        unsigned show_misc_screen : 1;      // show temperature/misc screen
        unsigned par_injection : 1;         // pair/parallel injection
    };
} settings_u;

// little endian
typedef union {
    uint16_t word;
    
    struct {
        uint8_t main_param : 4;
        uint8_t service_param : 4;
        uint8_t min_speed : 4;
        uint8_t misc_param : 4;
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

    // selected params + min speed
    param_u selected_param;

    // dummy bytes
    uint8_t dummy[2];

} config_t;                         // 16 bytes total (16 bytes eeprom block)

typedef struct {
    uint16_t odo;
    uint16_t odo_temp;
    uint8_t fuel_tmp1, fuel_tmp2;
    uint16_t fuel;
    uint32_t time;
} trip_t;                           // 12 bytes

typedef struct {
    uint8_t minute, hour, day, month, year;
} trip_time_t;                      // 5 bytes

typedef struct {
    trip_t tripA, tripB, tripC;     // 12 * 3 bytes
    trip_time_t tripC_time;         // 5 bytes
    uint16_t tripC_max_speed;       // 2 bytes
    uint8_t tripB_month;            // 1 byte
} trips_t;                          // 44 bytes total (48 bytes eeprom block)

typedef struct {
    uint8_t day;
    uint8_t month;
    uint8_t year;
} service_time_t;                   // 3 bytes

typedef struct {
    uint16_t counter;
    uint8_t limit;
    service_time_t time;
    uint8_t dummy[2];               // fill to 8 bytes size
} srv_t;                            // 8 bytes

typedef struct {
    uint32_t time;
    uint32_t rpm;
    uint16_t limit;
} srv_mh_t;                         // 10 bytes

typedef struct {
    srv_mh_t mh;                    // 10 bytes
    srv_t srv[4];                   // 4 * 8 bytes
} services_t;                       // 42 bytes total (48 bytes eeprom block)

typedef struct {
    uint16_t current;
#ifdef MIN_MAX_VOLTAGES_SUPPORT
    uint16_t min;
    uint16_t max;
#endif
} adc_voltage_t;

// 0 - 100
#define ACCEL_MEAS_LOWER_0 0
#define ACCEL_MEAS_UPPER_0 100
// 0 - 60    
#define ACCEL_MEAS_LOWER_1 0
#define ACCEL_MEAS_UPPER_1 60
// 60 - 100
#define ACCEL_MEAS_LOWER_2 60
#define ACCEL_MEAS_UPPER_2 100
// 80 - 120
#define ACCEL_MEAS_LOWER_3 80
#define ACCEL_MEAS_UPPER_3 120

typedef struct {
    uint32_t lower;
    uint32_t upper;
} accel_meas_limits_t;

#define EEPROM_CONFIG_ADDRESS           0
#define EEPROM_TRIPS_ADDRESS            (((sizeof(config_t) - 1) / 8 + 1) * 8)
#define EEPROM_SERVICES_ADDRESS         (((sizeof(config_t) - 1) / 8 + 1) * 8) + (((sizeof(trips_t) - 1) / 8 + 1) * 8)
#define EEPROM_DS18B20_ADDRESS          (((sizeof(config_t) - 1) / 8 + 1) * 8) + (((sizeof(trips_t) - 1) / 8 + 1) * 8) + (((sizeof(services_t) - 1) / 8 + 1) * 8)
#define EEPROM_CUSTOM_CHARS_ADDRESS     (EEPROM_DS18B20_ADDRESS + 8 * 3)
#define EEPROM_CONTINUOUS_DATA_ADDRESS  (EEPROM_CUSTOM_CHARS_ADDRESS + 8 * 8)

typedef struct {
    void (*screen)(void);
} screen_item_t;

typedef struct {
    uint8_t str_index;
    void (*screen)(void);
} config_screen_item_t;

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

#define BUZZER_KEY              0
#define BUZZER_LONGKEY          1
#define BUZZER_WARN             2
#define BUZZER_NONE             -1

#define BUZZER_KEY_COUNTER      1
#define BUZZER_KEY_SOUND        1
#define BUZZER_KEY_PAUSE        1
#define BUZZER_LONGKEY_COUNTER  1
#define BUZZER_LONGKEY_SOUND    4
#define BUZZER_LONGKEY_PAUSE    1
#define BUZZER_WARN_COUNTER     3
#define BUZZER_WARN_SOUND       3
#define BUZZER_WARN_PAUSE       2

typedef struct {
    uint8_t *p;         // value pointer
    uint8_t min;        // min value
    uint8_t max;        // max value
    uint8_t pos;        // cursor position
} time_editor_item_t;

#define FILTERED_VALUE_FIRST_SAMPLE 0x80

typedef struct {
    uint32_t tmp;
    uint8_t filter;     // filter value (2^filter)
} filtered_value_t;     // 3 bytes

typedef void (*handle)(filtered_value_t *);

typedef struct {
    void (*handle)(filtered_value_t *f);    // handler                  (2 bytes)
    filtered_value_t *f;                    // filtered value struct    (5 bytes)
    uint8_t channel;                        // adc channel              (1 byte)
} adc_item_t;

// continuous data parameters
#define CD_FILTER_VALUE_MIN      6
#define CD_FILTER_VALUE_MAX      9
#define CD_TIME_THRESHOLD_INIT   ((1 << CD_FILTER_VALUE_MIN) * 6)

typedef struct {
    filtered_value_t f_kmh;                 // filtered speed (pulses) value
    filtered_value_t f_fuel;                // filtered fuel (timer overflow) value
    uint16_t time;                          // current time
    uint16_t time_threshold;                // time threshold for filtered value's readiness/increase filter value
    uint8_t filter;                         // filter value for both speed and fuel (2^filter)
} continuous_data_t;                        // 15 bytes total (16 bytes eeprom block)

// fuel tank adc interval (*0,01ms)
#define FUEL_TANK_ADC_INTERVAL 100

typedef struct {
    uint8_t status;                 // 1 byte
    trip_time_t start_time;         // 5 byte
    trip_t trip;                    // 12 bytes
} journal_trip_item_t;              // 18 bytes total

typedef struct {
    uint8_t status;                 // 1 byte
    trip_time_t start_time;         // 5 byte
    uint8_t lower;                  // 1 byte
    uint8_t upper;                  // 1 byte
    uint16_t time;                  // 2 byte
} journal_accel_item_t;             // 10 bytes total

typedef struct {
    uint8_t current;
    uint8_t max;
} journal_type_pos_t;

typedef union {
    uint8_t byte;

    struct {
        uint8_t dummy : 8;
    };
} journal_header_flags_u;

typedef struct {
    journal_type_pos_t journal_type_pos[4]; // 8
    trip_time_t time_trip_c;                // 5
    trip_time_t time_trip_a;                // 5
    trip_time_t time_trip_b;                // 5
    journal_header_flags_u flags;           // 1
} journal_header_t;                         // 24

#ifdef JOURNAL_EEPROM_INTERNAL

// internal eeprom for atmega328p (768 bytes)

#define JOURNAL_read_eeprom_block(p, ee_addr, length) HW_read_eeprom_block(p, ee_addr, length);
#define JOURNAL_write_eeprom_block(p, ee_addr, length) HW_write_eeprom_block(p, ee_addr, length);

#define J_EEPROM_START 256
#define J_EEPROM_LENGTH 768

// 768 - 32 = 736 bytes for data

// (20 + 12 + 6) * 18 + 5 * 10 = 734
#define J_EEPROM_TRIPC_COUNT 20
#define J_EEPROM_TRIPA_COUNT 12
#define J_EEPROM_TRIPB_COUNT 6
#define J_EEPROM_ACCEL_COUNT 5

#else

// i2c eeprom 24lc16 ((2048-32) / 16 bytes per block = 126 data blocks)

#define JOURNAL_read_eeprom_block(p, ee_addr, length) I2C_read_eeprom_block(p, ee_addr, length);
#define JOURNAL_write_eeprom_block(p, ee_addr, length) I2C_write_eeprom_block(p, ee_addr, length);

#define J_EEPROM_START 0
#define J_EEPROM_LENGTH 2048

// 2048 - 32 = 2016 bytes for data

// (64 + 30 + 12) * 18 + 10 * 10 = 2008
#define J_EEPROM_TRIPC_COUNT 64
#define J_EEPROM_TRIPA_COUNT 30
#define J_EEPROM_TRIPB_COUNT 12
#define J_EEPROM_ACCEL_COUNT 10

#endif

#define J_EEPROM_MARK_POS J_EEPROM_START
#define J_EEPROM_DATA J_EEPROM_START + 32

#define JOURNAL_ITEM_OK 0xA5

#endif	/* MAIN_H */

