#ifndef CORE_H
#define	CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "hw.h"
#include "ds3231.h"

// temperature timeout
#define TIMEOUT_TEMPERATURE (30 - 1)
#define FORCED_TIMEOUT_TEMPERATURE (5 - 1)

// const for voltage adjust
#define VOLTAGE_ADJUST_CONST_MIN 140
#define VOLTAGE_ADJUST_CONST_MAX 230
#define VOLTAGE_ADJUST_CONST_MAX 230

// default min speed for drive mode (km/h)
#define MIN_SPEED_DEFAULT 5

//show average speed (or fuel consumption) after distance AVERAGE_MIN_DIST * 0.1 km
#define AVERAGE_MIN_DIST 5

// show average fuel consumption after total consumption of AVERAGE_MIN_FUEL * 0,1 litres
#define AVERAGE_MIN_FUEL 1

// max value of trip A odometer
#define MAX_ODO_TRIPA 2000

// max value of trip B odometer
#define MAX_ODO_TRIPB 6000

// max pause for continuing trip C
#define TRIPC_PAUSE_MINUTES 120

// round taho
#define TAHO_ROUND 10
// taho const 
#define TAHO_CONST ((uint32_t) (60 / HW_TAHO_TIMER_PERIOD * HW_TAHO_TIMER_TICKS_PER_PERIOD))

// speed timer counts between speed pulses when speed is X km/h
// (1 / ((config.odo_const * X) / 3600)) / (SPEED_TIMER_PERIOD / SPEED_TIMER_TICKS_PER_PERIOD) = ((3600 / X) / (SPEED_TIMER_PERIOD / SPEED_TIMER_TICKS_PER_PERIOD) / config.odo_const
#define speed_const(x) ((uint32_t) ((3600 / x) / (HW_SPEED_TIMER_PERIOD / HW_SPEED_TIMER_TICKS_PER_PERIOD)))

// power supply threshold 
// with default divider resistor's (8,2k (to Vcc) + 3,6k (to GND)) values
// THRESHOLD_VOLTAGE * (3,6 / (3,6 + 8,2)) * (1024 / 5) = THRESHOLD_VOLTAGE_ADC_VALUE
// 2,048V ~ 128
#define THRESHOLD_VOLTAGE_ADC_VALUE 128

#ifdef ADC_BUTTONS_SUPPORT
// threshold for buttons +-0,2v
#define ADC_BUTTONS_THRESHOLD 40
#define ADC_BUTTONS_1V (1024/5)            
#endif

// misc constants (in seconds)
#define MAIN_INTERVAL ((uint8_t) (1.0f / HW_MAIN_TIMER_PERIOD))
#define DEBOUNCE ((uint8_t) (0.04f / HW_MAIN_TIMER_PERIOD))
#if defined(__DEBUG) || defined(DEBUG)
#define SHORTKEY ((uint8_t) (0.4f / HW_MAIN_TIMER_PERIOD))
#define MULTICLICK ((uint8_t) (0.4f / HW_MAIN_TIMER_PERIOD))
#else
#define SHORTKEY ((uint8_t) (0.2f / HW_MAIN_TIMER_PERIOD))
#define MULTICLICK ((uint8_t) (0.3f / HW_MAIN_TIMER_PERIOD))
#endif
#define LONGKEY ((uint8_t) (1.0f / HW_MAIN_TIMER_PERIOD))
#define KEY_REPEAT_PAUSE ((uint8_t) (0.15f / HW_MAIN_TIMER_PERIOD))
// timeout constant in 0.01 ms resolution
#define INIT_TIMEOUT(t) ((uint8_t) (t * 10.0f * 0.1f / HW_MAIN_TIMER_PERIOD))
// time with power supply measurements lower than threshold before shutdown
#define SHUTDOWN ((uint8_t) (0.25f / HW_MAIN_TIMER_PERIOD))
// timeout when no key pressed
#define DEFAULT_TIMEOUT 10

// min rpm
#define TAHO_MIN_RPM 100UL
// min rpm constant (1/(TAHO_MIN_RPM/60sec)/0.01s) 0.01s timer overflow
#define TAHO_OVERFLOW ((uint8_t) ((1.0f / (TAHO_MIN_RPM / 60.0f) ) / HW_TAHO_TIMER_PERIOD))

// minimum pulse width for acceleration measurement calculation (0.1s)
#define ACCEL_MEAS_OVERFLOW_CONST 10            /* (0.1f / SPEED_TIMER_PERIOD) */
#if (65536 / HW_SPEED_TIMER_TICKS_PER_PERIOD) >= ACCEL_MEAS_OVERFLOW_CONST
#define ACCEL_MEAS_OVERFLOW ACCEL_MEAS_OVERFLOW_CONST
#else
#define ACCEL_MEAS_OVERFLOW (65536 / HW_SPEED_TIMER_TICKS_PER_PERIOD)
#endif

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

#define BUZZER_KEY              0
#define BUZZER_LONGKEY          1
#define BUZZER_WARN             2
#define BUZZER_NONE             -1

// 10ms resolution
#define BUZZER_KEY_COUNTER      1
#define BUZZER_KEY_SOUND        8
#define BUZZER_KEY_PAUSE        0
#define BUZZER_LONGKEY_COUNTER  1
#define BUZZER_LONGKEY_SOUND    40
#define BUZZER_LONGKEY_PAUSE    0
#define BUZZER_WARN_COUNTER     3
#define BUZZER_WARN_SOUND       30
#define BUZZER_WARN_PAUSE       20

#define FILTERED_VALUE_FIRST_SAMPLE 0x80

// fuel tank adc interval (*0,01ms)
#define FUEL_TANK_ADC_INTERVAL 100

// ds1307 read timeout
#define TIMEOUT_DS_READ 10

#define EEPROM_CONFIG_ADDRESS           0
#define EEPROM_TRIPS_ADDRESS            (((sizeof(config_t) - 1) / 8 + 1) * 8)
#define EEPROM_SERVICES_ADDRESS         (((sizeof(config_t) - 1) / 8 + 1) * 8) + (((sizeof(trips_t) - 1) / 8 + 1) * 8)
#define EEPROM_DS18B20_ADDRESS          (((sizeof(config_t) - 1) / 8 + 1) * 8) + (((sizeof(trips_t) - 1) / 8 + 1) * 8) + (((sizeof(services_t) - 1) / 8 + 1) * 8)
#define EEPROM_CUSTOM_CHARS_ADDRESS     (EEPROM_DS18B20_ADDRESS + 8 * 3)
#define EEPROM_CONTINUOUS_DATA_ADDRESS  (EEPROM_CUSTOM_CHARS_ADDRESS + 8 * 8)

typedef union {
    // a structure with 16 single bit bit-field objects, overlapping the union member "word"
    uint16_t word;

    struct {
#if defined(ENCODER_SUPPORT)
        unsigned encoder            : 1; // encoder control
#else
        unsigned dummy_encoder      : 1;
#endif
#if defined(ADC_BUTTONS_SUPPORT)
        unsigned adc_buttons        : 1; // use adc buttons
#else
        unsigned dummy_adc_buttons  : 1;
#endif
#if defined(LCD_1602) && defined(LCD_1602_I2C)
        unsigned lcd_1602_i2c       : 1; // use lcd 1602 i2c
#else
        unsigned dummy_lcd_1602_i2c : 1;
#endif
        unsigned dummy              : 3;
        unsigned adc_fuel_normalize : 1; // normalize adc_fuel to adc_voltage
        unsigned ds3231_temp        : 1; // use ds3231 temperature as inner sensor
        unsigned show_inner_temp    : 1; // show inner (outer by default) temperature on first screen
#ifndef SIMPLE_TRIPC_TIME_CHECK
        unsigned daily_tripc        : 1; // use trip C as daily counter (auto reset on next day)
#else
        unsigned dummy_daily_tripc  : 1;
#endif
        unsigned monthly_tripb      : 1; // use trip B as monthly counter (auto reset on next month)
        unsigned dummy1             : 1; // 
        unsigned service_alarm      : 1; // alarm for service counters
        unsigned key_sound          : 1; // keys sound
        unsigned instant_fuel_avg   : 1; // averaging (filtering) instant fuel
        unsigned par_injection      : 1; // pair/parallel injection
    };
} settings_u;

typedef union {
    uint16_t word;

    struct {
        uint8_t main_param          : 4;
        uint8_t service_param       : 4;
        uint8_t min_speed           : 4;
        uint8_t main_add_param      : 4;
    };
} param_u;

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
    //uint8_t dummy[2];

} config_t;                     // 14 bytes total (16 bytes eeprom block)

typedef struct {
    uint16_t speed;
    uint16_t taho_rpm;
    uint16_t fuel_duration_ms;
    uint16_t fuel_instant;

#ifdef CONTINUOUS_DATA_SUPPORT
    uint16_t cd_speed;
    uint16_t cd_fuel_instant;
#endif

} live_data_t;

typedef struct {
    uint16_t odo;
    uint16_t odo_temp;
    uint8_t fuel_tmp1, fuel_tmp2;
    uint16_t fuel;
    uint32_t time;
} trip_t;                       // 12 bytes

typedef struct {
    uint16_t odo;
    uint16_t average_speed;
    uint16_t average_fuel;
    uint16_t fuel;
    uint16_t time;
    uint16_t dummy;
} print_trip_t;                 // 12 bytes

typedef struct {
    uint8_t minute, hour, day, month, year;
} trip_time_t;                  // 5 bytes

typedef struct {
    trip_t tripA, tripB, tripC; // 12 * 3 bytes
    trip_time_t tripC_time;     // 5 bytes
    uint8_t tripB_month;        // 1 byte
    uint16_t tripC_max_speed;   // 2 bytes
    uint8_t tripC_time_dow;     // 1 byte
} trips_t;                      // 45 bytes total (48 bytes eeprom block)

typedef struct {
    uint32_t time;
    uint16_t limit;
} srv_mh_t;                     // 6 bytes

typedef struct {
    uint16_t counter;
    uint8_t limit;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} srv_t;                        // 6 bytes

typedef struct {
    srv_mh_t mh;                // 6 bytes
    srv_t srv[4];               // 4 * 6 bytes
} services_t;                   // 30 bytes total (32 bytes eeprom block)

typedef struct {
    uint16_t current;
#ifdef MIN_MAX_VOLTAGES_SUPPORT
    uint16_t min;
    uint16_t max;
#endif
} adc_voltage_t;

typedef struct {
    uint32_t lower;
    uint32_t upper;
} accel_meas_limits_t;

typedef struct {
    uint32_t tmp;
    uint8_t filter;                      // filter value (2^filter)
} filtered_value_t;                      // 5 bytes

typedef void (*handle)(filtered_value_t *);

typedef struct {
    void (*handle)(filtered_value_t *f); // handler                  (2 bytes)
    filtered_value_t *f;                 // filtered value struct    (5 bytes)
    uint8_t channel;                     // adc channel              (1 byte)
} adc_item_t;

typedef struct {
    filtered_value_t f_kmh;             // filtered speed (pulses) value
    uint8_t filter;                     // filter value for both speed and fuel (2^filter)
    filtered_value_t f_fuel;            // filtered fuel (timer overflow) value
    uint8_t dummy;                      // dummy for word align
    uint16_t time;                      // current time
    uint16_t time_threshold;            // time threshold for filtered value's readiness/increase filter value
} continuous_data_t;                    // 16 bytes total (16 bytes eeprom block)

extern config_t config;
extern trips_t trips;
extern services_t services;

extern __bank2 print_trip_t ptrip;

extern __bank2 live_data_t data;

__near extern volatile flag_t screen_refresh;

extern volatile flag_t taho_fl, drive_fl, motor_fl, shutdown_fl;
extern volatile flag_t buzzer_fl;

extern volatile flag_t save_tripc_time_fl;

// key variables and flags
extern volatile flag_t key1_press, key2_press, key1_longpress, key2_longpress;

#if defined(LCD_1602) && defined(LCD_1602_I2C)
extern volatile flag_t use_lcd_1602_i2c_fl;
#define use_lcd_1602_i2c() use_lcd_1602_i2c_fl
#endif

#if defined(ADC_BUTTONS_SUPPORT)
extern volatile flag_t use_adc_buttons_fl;
#define use_adc_buttons() use_adc_buttons_fl
#else
#define use_adc_buttons() 0
#endif

#if defined(ENCODER_SUPPORT)
extern volatile flag_t key2_doubleclick;
extern volatile flag_t use_encoder_fl;
#define use_encoder() use_encoder_fl
#else
#define use_encoder() 0
#endif

#if defined(KEY3_SUPPORT)
extern volatile flag_t key3_press, key3_longpress;
#endif

extern volatile uint24_t taho;
extern volatile uint16_t fuel_duration;

// timeout1 (resolution - 1 s)
extern volatile uint8_t timeout_timer1;
// timeout2 (resolution - 0.01 s)
extern volatile uint8_t timeout_timer2;

extern volatile adc_voltage_t adc_voltage;


// acceleration measurement flags and variables
extern volatile flag_t accel_meas_fl, accel_meas_ok_fl, accel_meas_process_fl, accel_meas_timer_fl, accel_meas_drive_fl;
#ifdef EXTENDED_ACCELERATION_MEASUREMENT
extern volatile uint16_t accel_meas_lower_const;
#endif
extern volatile uint16_t accel_meas_upper_const, accel_meas_timer, accel_meas_speed;

extern volatile uint16_t kmh, fuel;

extern uint8_t drive_min_speed;

extern volatile uint8_t timeout_ds_read;

#ifdef SOUND_SUPPORT
extern volatile int8_t buzzer_mode_index;
#endif

#ifdef TEMPERATURE_SUPPORT
extern volatile uint8_t timeout_temperature;
#endif

#ifdef CONTINUOUS_DATA_SUPPORT

// continuous data parameters
#define CD_FILTER_VALUE_MIN      6
#define CD_FILTER_VALUE_MAX      9
#define CD_TIME_THRESHOLD_INIT   ((1 << CD_FILTER_VALUE_MIN) * 6)

extern continuous_data_t cd;
extern flag_t continuous_data_fl;
extern volatile uint16_t cd_kmh, cd_fuel;
void cd_init(void);
void cd_increment_filter(void);
#endif

#if defined(KEY3_SUPPORT) || defined(ADC_BUTTONS_SUPPORT)
#define no_key_pressed() (key1_press == 0 && key2_press == 0 && key3_press == 0)
#if defined(ENCODER_SUPPORT)
#define clear_keys_state() key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0; key2_doubleclick = 0; key3_press = 0; key3_longpress = 0
#else
#define clear_keys_state() key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0; key3_press = 0; key3_longpress = 0
#endif
#else
#define no_key_pressed() (key1_press == 0 && key2_press == 0)
#define clear_keys_state() key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0
#endif

void handle_keys_next_prev(uint8_t *v, uint8_t min_value, uint8_t max_value, uint8_t timeout);
void handle_keys_up_down(uint8_t *v, uint8_t min_value, uint8_t max_value, uint8_t timeout);

void int_capture_injector_level_change(void);
void int_taho_timer_overflow(void);
void int_capture_speed_level_change(void);
void int_speed_timer_overflow(void);
void int_fuel_timer_overflow(void);
void int_main_timer_overflow(void);
void int_adc_finish(void);
void int_change_encoder_level(void);

extern volatile uint16_t main_timer;
#ifndef taho_timer
extern volatile uint16_t taho_timer;
#endif
#ifndef speed_timer
extern volatile uint16_t speed_timer;
#endif

void read_ds_time(void);

void fill_trip_time(trip_time_t *);

void fill_print_trip(print_trip_t* pt, trip_t* t);

uint16_t get_voltage_value(uint16_t *adc_voltage);

void fill_live_data(void);

void set_consts(void);

uint16_t round_div(uint16_t dividend, uint16_t divisor);

#endif	/* CORE_H */
