#ifndef CORE_H
#define	CORE_H

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
#define MAIN_INTERVAL ((uint8_t) (1.0f / MAIN_TIMER_PERIOD))
#define DEBOUNCE ((uint8_t) (0.04f / MAIN_TIMER_PERIOD))
#define SHORTKEY ((uint8_t) (0.5f / MAIN_TIMER_PERIOD))
#define LONGKEY ((uint8_t) (1.0f / MAIN_TIMER_PERIOD))
#define KEY_REPEAT_PAUSE ((uint8_t) (0.15f / MAIN_TIMER_PERIOD))
// timeout constant in 0.01 ms resolution
#define INIT_TIMEOUT(t) ((uint8_t) (t * 10.0f * 0.1f / MAIN_TIMER_PERIOD))
// time with power supply measurements lower than threshold before shutdown
#define SHUTDOWN ((uint8_t) (0.25f / MAIN_TIMER_PERIOD))

// min rpm
#define TAHO_MIN_RPM 100UL
// min rpm constant (1/(TAHO_MIN_RPM/60sec)/0.01s) 0.01s timer overflow
#define TAHO_OVERFLOW ((uint8_t) ((1.0f / (TAHO_MIN_RPM / 60.0f) ) / TAHO_TIMER_PERIOD))

// minimum pulse width for acceleration measurement calculation (0.1s)
#define ACCEL_MEAS_OVERFLOW_CONST 10            /* (0.1f / SPEED_TIMER_PERIOD) */
#if (65536 / SPEED_TIMER_TICKS_PER_PERIOD) >= ACCEL_MEAS_OVERFLOW_CONST
#define ACCEL_MEAS_OVERFLOW ACCEL_MEAS_OVERFLOW_CONST
#else
#define ACCEL_MEAS_OVERFLOW (65536 / SPEED_TIMER_TICKS_PER_PERIOD)
#endif

extern config_t config;
extern trips_t trips;
extern services_t services;

extern volatile __bit taho_fl, drive_fl, motor_fl, save_tripc_time_fl, shutdown_fl, screen_refresh;

// key variables and flags
extern volatile uint8_t key_repeat_counter;
extern volatile __bit key1_press, key2_press, key1_longpress, key2_longpress, key_pressed, key_longpressed;

extern volatile uint24_t taho;
extern volatile uint16_t fuel_duration;

// timeout1 (resolution - 1 s)
extern volatile uint8_t timeout_timer1;
// timeout2 (resolution - 0.01 s)
extern volatile uint8_t timeout_timer2;

extern volatile adc_voltage_t adc_voltage;

#if defined(KEY3_SUPPORT)
extern volatile __bit key3_press, key3_longpress;
#endif

// acceleration measurement flags and variables
extern volatile __bit accel_meas_fl, accel_meas_ok_fl, accel_meas_process_fl, accel_meas_timer_fl, accel_meas_drive_fl;
#ifdef EXTENDED_ACCELERATION_MEASUREMENT
extern volatile uint16_t accel_meas_lower_const;
#endif
extern volatile uint16_t accel_meas_upper_const, accel_meas_timer, accel_meas_speed;

extern volatile uint16_t kmh, fuel;

extern volatile uint8_t timeout_ds_read;

#ifdef SOUND_SUPPORT
extern volatile int8_t buzzer_mode_index;
#endif

#ifdef TEMPERATURE_SUPPORT
extern volatile uint8_t timeout_temperature;
#endif

extern uint8_t fuel1_const;

#ifdef CONTINUOUS_DATA_SUPPORT

// continuous data parameters
#define CD_FILTER_VALUE_MIN      6
#define CD_FILTER_VALUE_MAX      9
#define CD_TIME_THRESHOLD_INIT   ((1 << CD_FILTER_VALUE_MIN) * 6)

extern continuous_data_t cd;
extern __bit continuous_data_fl;
extern volatile uint16_t cd_kmh, cd_fuel;
void cd_init(void);
void cd_increment_filter(void);
#endif


#endif	/* CORE_H */

