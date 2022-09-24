#include "main.h"
#include "locale.h"
#include "eeprom.h"
#include "i2c.h"
#include "lcd.h"
#include "ds1307.h"
#include "ds18b20.h"
#include "i2c-eeprom.h"
#include "utils.h"
#include <stdbool.h>
#include <string.h>

config_t config;
trips_t trips;
services_t services;

//========================================================================================

#ifdef ADC_BUTTONS

#define ADC_KEY_OK 2
#define ADC_KEY_NEXT 1
#define ADC_KEY_PREV 3

volatile uint8_t adc_key;

#define KEY1_PRESSED (adc_key == ADC_KEY_NEXT)
#define KEY2_PRESSED (adc_key == ADC_KEY_OK)
#define KEY3_PRESSED (adc_key == ADC_KEY_PREV)

#endif

#if defined(KEY3_SUPPORT) || defined(ADC_BUTTONS)
#define NO_KEY_PRESSED (key1_press == 0 && key2_press == 0 && key3_press == 0)
#define clear_keys_state() key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0; key3_press = 0; key3_longpress = 0
#else
#define NO_KEY_PRESSED (key1_press == 0 && key2_press == 0)
#define clear_keys_state() key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0
#endif

// key variables and flags
volatile uint8_t key_repeat_counter;
volatile __bit key1_press, key2_press, key1_longpress, key2_longpress, key_pressed, key_longpressed;

#if defined(KEY3_SUPPORT)
volatile __bit key3_press, key3_longpress;
#endif

volatile uint8_t acc_power_off_counter;

// main interval
volatile __bit screen_refresh;

// timeout1 (resolution - 1 s)
volatile uint8_t timeout_timer1;
// timeout2 (resolution - 0.01 s)
volatile uint8_t timeout_timer2;

// misc flags
volatile __bit odom_fl, drive_fl, motor_fl, fuel_fl, taho_fl, taho_measure_fl, drive_min_speed_fl, acc_power_off_fl, shutdown_fl;

// acceleration measurement flags and variables
volatile __bit accel_meas_fl, accel_meas_ok_fl, accel_meas_process_fl, accel_meas_timer_fl, accel_meas_drive_fl, accel_meas_final_fl, _accel_meas_exit;
#ifdef EXTENDED_ACCELERATION_MEASUREMENT
volatile uint16_t accel_meas_lower_const;
#endif
volatile uint16_t accel_meas_upper_const, accel_meas_timer, accel_meas_speed;
volatile uint16_t speed_timer_prev, speed_timer_ticks;
volatile uint8_t speed_timer_ofl;

volatile uint16_t kmh_tmp, fuel_tmp;
volatile uint16_t kmh, fuel;

#ifdef MIN_MAX_VOLTAGES_SUPPORT
volatile adc_voltage_t adc_voltage = {0, ADC_MAX, ADC_MIN};
#else
volatile adc_voltage_t adc_voltage = {0};
#endif

volatile uint16_t main_timer;
#ifndef taho_timer
volatile uint16_t taho_timer;
#endif
#ifndef speed_timer
volatile uint16_t speed_timer;
#endif

volatile uint24_t taho, taho_timer_ticks;
volatile uint16_t taho_timer_prev;
volatile uint8_t taho_timer_ofl;

// uint16_t - max ~104 ms for pic16f targets (* 2.5 for atmega)
volatile uint16_t fuel_duration;

volatile __bit save_tripc_time_fl = 0;
uint16_t speed;

volatile uint8_t timeout_ds_read = 0;

#ifdef TEMPERATURE_SUPPORT
volatile uint8_t timeout_temperature;
__bit temperature_conv_fl;

uint16_t _t;
uint16_t temps[4] = {DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE};
#endif

#ifdef SOUND_SUPPORT
volatile int8_t buzzer_mode_index = BUZZER_NONE;
#endif

uint8_t tmp_param = 0, main_param = 0, misc_param = 0;
#ifdef SERVICE_COUNTERS_SUPPORT
uint8_t service_param = 0;
#endif
        
// buffer for strings
char buf[16];
uint8_t len;

#if defined(_16F876A)
__bank2 ds_time time;
#else
ds_time time;
#endif

void read_ds_time(void);
void fill_trip_time(trip_time_t *);

uint8_t fuel1_const, fuel2_const;
uint16_t odo_con4;

void screen_main(void);
void screen_tripC(void);
void screen_misc(void);
void screen_max(void);
void screen_tripA(void);
void screen_tripB(void);
void screen_time(void);
void screen_service_counters(void);
void screen_journal_viewer(void);
void screen_trip(void);

// max screen in drive mode
typedef enum {
    drive_mode_screen_main,
    drive_mode_screen_tripC,
#if defined(TEMPERATURE_SUPPORT) || defined(MIN_MAX_VOLTAGES_SUPPORT) || defined(CONTINUOUS_DATA_SUPPORT)
    drive_mode_screen_misc,
#endif
    drive_mode_screen_max
} drive_mode_screens;

typedef enum {
    SCREEN_INDEX_MAIN = 0,
    SCREEN_INDEX_TRIP_C,
    SCREEN_INDEX_MISC,
    SCREEN_INDEX_TRIP_A,
    SCREEN_INDEX_TRIP_B,
    SCREEN_INDEX_TIME,
    SCREEN_INDEX_SERVICE_COUNTERS,
    SCREEN_INDEX_JOURNAL
} screen_item_index;

const screen_item_t items_main[] = {
    {screen_main, SCREEN_INDEX_MAIN},
    {screen_trip, SCREEN_INDEX_TRIP_C},
#if defined(TEMPERATURE_SUPPORT) || defined(MIN_MAX_VOLTAGES_SUPPORT) || defined(CONTINUOUS_DATA_SUPPORT)
    {screen_misc, SCREEN_INDEX_MISC},
#endif
    {screen_trip, SCREEN_INDEX_TRIP_A},
    {screen_trip, SCREEN_INDEX_TRIP_B},
    {screen_time, SCREEN_INDEX_TIME},
#ifdef SERVICE_COUNTERS_SUPPORT   
    {screen_service_counters, SCREEN_INDEX_SERVICE_COUNTERS},
#endif
#ifdef JOURNAL_SUPPORT
    {screen_journal_viewer, SCREEN_INDEX_JOURNAL},
#endif
};

void config_screen_fuel_constant(void);
void config_screen_vss_constant(void);
void config_screen_total_trip(void);
void config_screen_settings_bits(void);
void config_screen_temp_sensors(void);
void config_screen_service_counters(void);
void config_screen_ua_const(void);
void config_screen_min_speed(void);
void config_screen_version(void);

const screen_config_item_t items_service[] = {
    {config_screen_fuel_constant, FUEL_CONSTANT_INDEX},
    {config_screen_vss_constant, VSS_CONSTANT_INDEX},
    {config_screen_total_trip, TOTAL_TRIP_INDEX},
    {config_screen_ua_const, VOLTAGE_ADJUST_INDEX},
    {config_screen_settings_bits, SETTINGS_BITS_INDEX},
#ifdef MIN_SPEED_CONFIG
    {config_screen_min_speed, MIN_SPEED_INDEX},
#endif
#if defined(DS18B20_TEMP) && defined(DS18B20_CONFIG)
    {config_screen_temp_sensors, TEMP_SENSOR_INDEX},
#endif
#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
    {config_screen_service_counters, SERVICE_COUNTERS_INDEX},
#endif
    {config_screen_version, VERSION_INFO_INDEX},
};

uint8_t c_item = 0;
uint8_t c_item_prev = 0;
__bit item_skip;

uint8_t request_screen(char *);

void config_screen();

uint16_t get_speed(uint16_t);

void journal_save_trip(trip_t *trip);
void journal_save_accel(uint8_t index);

void save_eeprom(void);
void save_eeprom_trips(void);
void save_eeprom_config(void);

uint16_t calc_filtered_value(filtered_value_t *, uint16_t);

void adc_handler_voltage(filtered_value_t *f);
void adc_handler_buttons(filtered_value_t *f);
void adc_handler_fuel_tank(filtered_value_t *f);

#if !defined(SIMPLE_ADC)

uint16_t adc_value;

filtered_value_t f_voltage = {0, 1 | FILTERED_VALUE_FIRST_SAMPLE};

#if defined(ADC_BUTTONS) || defined(FUEL_TANK_SUPPORT)

uint8_t _adc_ch;

#ifdef ADC_BUTTONS
filtered_value_t f_buttons = {0, 0};
#endif

#if defined(FUEL_TANK_SUPPORT)
uint8_t adc_fuel_tank_counter;
uint16_t adc_fuel_tank;
filtered_value_t f_fuel_tank = {0, 3 | FILTERED_VALUE_FIRST_SAMPLE};
#endif


adc_item_t adc_items[] = {
    {adc_handler_voltage, &f_voltage, ADC_CHANNEL_POWER_SUPPLY},
#ifdef ADC_BUTTONS
    {adc_handler_buttons, &f_buttons, ADC_CHANNEL_BUTTONS},
#endif
#ifdef FUEL_TANK_SUPPORT
    {adc_handler_fuel_tank, &f_fuel_tank, ADC_CHANNEL_FUEL_TANK},
#endif
};
#else
adc_item_t adc_item = {adc_handler_voltage, &f_voltage, ADC_CHANNEL_POWER_SUPPLY};
#endif

#endif

#ifdef CONTINUOUS_DATA_SUPPORT
continuous_data_t cd;
__bit continuous_data_fl, drive_min_cd_speed_fl;
volatile uint16_t cd_kmh, cd_fuel;
uint16_t cd_speed;
void cd_init(void);
void cd_increment_filter(void);
#endif

// interrupt routines starts

void int_capture_injector_level_change() {
    // fuel injector
    if (FUEL_ACTIVE) {
        if (fuel_fl == 0) {
            // start fuel timer
            start_fuel_timer();
            fuel_fl = 1;
            motor_fl = 1;
            save_tripc_time_fl = 1;

#ifdef SERVICE_COUNTERS_SUPPORT
            services.mh.rpm++;
            if (config.settings.par_injection == 0) {
                services.mh.rpm++;
            }
#endif

            // new taho calculation based on captured value of 0.01s timer
            if (taho_measure_fl == 0) {
                taho_measure_fl = 1;

                taho_timer_ticks = 0;
                taho_timer_prev = taho_timer;
                taho_timer_ofl = 0;
            } else {
                taho = taho_timer_ticks + taho_timer - taho_timer_prev;
                taho_fl = 1;

                taho_timer_ticks = 0;
                taho_timer_prev = taho_timer;
                taho_timer_ofl = 0;
            }
        }
    } else {
        if (fuel_fl != 0) {
            // stop fuel timer
            stop_fuel_timer();
            fuel_fl = 0;
            // measure fuel duration                
            if (taho_measure_fl != 0) {
                fuel_duration = (uint16_t) (taho_timer_ticks + taho_timer - taho_timer_prev);
            }
        }
    }
}

__section("text999") void int_taho_timer_overflow() {
    if (taho_measure_fl != 0) {
        if (++taho_timer_ofl == TAHO_OVERFLOW) {
            taho_measure_fl = 0;
            stop_fuel_timer();
            taho_fl = 0;
            fuel_duration = 0;
            motor_fl = 0;
        } else {
            taho_timer_ticks += TAHO_TIMER_TICKS_PER_PERIOD;
        }
    }
}

void int_capture_speed_level_change() {
    // speed sensor
    if (TX_ACTIVE) {
        if (odom_fl == 0) {
            odom_fl = 1;
            drive_fl = 1;

            // new speed 100 calculation based on captured value of 0.01s timer
            if (accel_meas_process_fl != 0) {
                if (accel_meas_fl == 0) {
                    accel_meas_fl = 1;
                    speed_timer_prev = speed_timer;
                    speed_timer_ticks = 0;
                    speed_timer_ofl = 0;
                } else {
                    accel_meas_speed = speed_timer_ticks + speed_timer - speed_timer_prev;
                    accel_meas_drive_fl = 1;
#ifdef EXTENDED_ACCELERATION_MEASUREMENT
                    if (accel_meas_timer_fl == 0 && (accel_meas_speed <= accel_meas_lower_const || accel_meas_lower_const == 0)) {
                        accel_meas_timer_fl = 1;
                    }
#else
                    accel_meas_timer_fl = 1;
#endif
                    if (accel_meas_speed <= accel_meas_upper_const) {
                        accel_meas_ok_fl = 1;
                        accel_meas_timer_fl = 0;
                        accel_meas_process_fl = 0;
                    } else {
                        speed_timer_prev = speed_timer;
                        speed_timer_ticks = 0;
                        speed_timer_ofl = 0;
                    }
                }
            } else {
                accel_meas_fl = 0;
            }

            kmh_tmp++;

        }
    } else {
        odom_fl = 0;
    }
}

__section("text999") void int_speed_timer_overflow() {
    if (accel_meas_fl != 0) {
        if (++speed_timer_ofl == ACCEL_MEAS_OVERFLOW) {
            accel_meas_fl = 0;
            accel_meas_drive_fl = 0;
        } else {
            speed_timer_ticks += SPEED_TIMER_TICKS_PER_PERIOD;
        }
    }
}

void int_fuel_timer_overflow() {
    fuel_tmp++;

    // trip A
    if (--trips.tripA.fuel_tmp1 == 0) {
        trips.tripA.fuel_tmp1 = fuel1_const;
        if (++trips.tripA.fuel_tmp2 >= config.fuel_const) {
            trips.tripA.fuel_tmp2 = 0;
            trips.tripA.fuel++;
        }
    }

    // trip B
    if (--trips.tripB.fuel_tmp1 == 0) {
        trips.tripB.fuel_tmp1 = fuel1_const;
        if (++trips.tripB.fuel_tmp2 >= config.fuel_const) {
            trips.tripB.fuel_tmp2 = 0;
            trips.tripB.fuel++;
        }
    }

    // trip C
    if (--trips.tripC.fuel_tmp1 == 0) {
        trips.tripC.fuel_tmp1 = fuel1_const;
        if (++trips.tripC.fuel_tmp2 >= config.fuel_const) {
            trips.tripC.fuel_tmp2 = 0;
            trips.tripC.fuel++;
        }
    }
}

void int_main_timer_overflow() {
    static uint8_t key1_counter = 0, key2_counter = 0;
#ifdef KEY3_SUPPORT
    static uint8_t key3_counter = 0;
#endif
    static uint8_t main_interval_counter = MAIN_INTERVAL;

    if (key_repeat_counter == 0) {
#ifndef ENCODER_SUPPORT
        if (KEY1_PRESSED) // key pressed
        {
            if (key1_counter <= LONGKEY) {
                key1_counter++;
            }
            if (key1_counter == LONGKEY) {
                // long keypress
                key1_longpress = 1;
                key_longpressed = 1;
            }
        } else // key released
        {
            if (key1_counter > DEBOUNCE && key1_counter <= SHORTKEY) {
                // key press
                key1_press = 1;
                key_pressed = 1;
            }
            key1_counter = 0;
        }
#endif

        if (KEY2_PRESSED) // key pressed
        {
            if (key2_counter <= LONGKEY) {
                key2_counter++;
            }
            if (key2_counter == LONGKEY) {
                // long keypress
                key2_longpress = 1;
                key_longpressed = 1;
            }
        } else // key released
        {
            if (key2_counter > DEBOUNCE && key2_counter <= SHORTKEY) {
                // key press
                key2_press = 1;
                key_pressed = 1;
            }
            key2_counter = 0;
        }

#if defined(KEY3_SUPPORT) && !defined(ENCODER_SUPPORT)
        if (KEY3_PRESSED) // key pressed
        {
            if (key3_counter <= LONGKEY) {
                key3_counter++;
            }
            if (key3_counter == LONGKEY) {
                // long keypress
                key3_longpress = 1;
                key_longpressed = 1;
            }
        } else // key released
        {
            if (key3_counter > DEBOUNCE && key3_counter <= SHORTKEY) {
                // key press
                key3_press = 1;
                key_pressed = 1;
            }
            key3_counter = 0;
        }
#endif
    } else {
        key_repeat_counter--;
    }

    if (key_pressed != 0 || key_longpressed != 0) {
#ifdef SOUND_SUPPORT
        if (key_pressed != 0) {
            buzzer_mode_index = BUZZER_KEY;
        } else {
            buzzer_mode_index = BUZZER_LONGKEY;
        }
#endif
        key_pressed = 0;
        key_longpressed = 0;
        screen_refresh = 1;
        key_repeat_counter = KEY_REPEAT_PAUSE;
    }

    if (acc_power_off_fl != 0) {
        if (acc_power_off_counter++ == SHUTDOWN) {
            screen_refresh = 1;
            timeout_timer1 = 0;
            shutdown_fl = 1;
        }
    } else {
        acc_power_off_counter = 0;
    }

    if (--main_interval_counter == 0) {
        main_interval_counter = MAIN_INTERVAL;

        // screen refresh_flag
        screen_refresh = 1;

        // increment time counters
        if (motor_fl != 0 || drive_fl != 0) {
            services.mh.time++;
            trips.tripA.time++;
            trips.tripB.time++;
            trips.tripC.time++;

#ifdef CONTINUOUS_DATA_SUPPORT
            if (cd.filter < CD_FILTER_VALUE_MAX) {
                if (cd.time < cd.time_threshold) {
                    cd.time++;
                } else {
                    cd_increment_filter();
                }
            }
            cd_fuel = calc_filtered_value(&cd.f_fuel, fuel_tmp);
            cd_kmh = calc_filtered_value(&cd.f_kmh, kmh_tmp);
#endif
        }

        // copy temp interval variables to main
        fuel = fuel_tmp;
        fuel_tmp = 0;

        if (kmh_tmp != 0) {
            // main odometer
            config.odo_temp += kmh_tmp;
            if (config.odo_temp >= config.odo_const) {
                config.odo_temp -= config.odo_const;
                // increment odometer counters
                config.odo++;
                services.srv[0].counter++;
                services.srv[1].counter++;
                services.srv[2].counter++;
                services.srv[3].counter++;
            }

            // trip A
            trips.tripA.odo_temp += kmh_tmp;
            if (trips.tripA.odo_temp >= config.odo_const) {
                trips.tripA.odo_temp -= config.odo_const;
                trips.tripA.odo++;
            }

            // trip B
            trips.tripB.odo_temp += kmh_tmp;
            if (trips.tripB.odo_temp >= config.odo_const) {
                trips.tripB.odo_temp -= config.odo_const;
                trips.tripB.odo++;
            }

            // trip C
            trips.tripC.odo_temp += kmh_tmp;
            if (trips.tripC.odo_temp >= config.odo_const) {
                trips.tripC.odo_temp -= config.odo_const;
                trips.tripC.odo++;
            }
        }

        kmh = kmh_tmp;
        kmh_tmp = 0;

#ifdef TEMPERATURE_SUPPORT
        if (timeout_temperature > 0) {
            timeout_temperature--;
        }
#endif                
        if (timeout_ds_read > 0) {
            timeout_ds_read--;
        }

        if (timeout_timer1 > 0) {
            timeout_timer1--;
        }

    }

    if (timeout_timer2 > 0) {
        timeout_timer2--;
    }

    if (accel_meas_timer_fl != 0) {
        accel_meas_timer++;
    }

#ifdef SOUND_SUPPORT

    static __bit buzzer_fl, buzzer_init_fl;
    static __bit buzzer_snd_fl, buzzer_repeat_fl;
    static uint8_t buzzer_counter_r;
    static uint8_t buzzer_counter, buzzer_counter_01sec;

    static uint8_t buzzer_mode_counter, buzzer_mode_sound, buzzer_mode_pause;

    if (buzzer_mode_index != BUZZER_NONE) {
        if (config.settings.key_sound != 0 || buzzer_mode_index == BUZZER_WARN) {
            buzzer_fl = 1;
            buzzer_init_fl = 0;
            switch (buzzer_mode_index) {
                case BUZZER_KEY:
                    buzzer_mode_counter = BUZZER_KEY_COUNTER;
                    buzzer_mode_sound = BUZZER_KEY_SOUND;
                    buzzer_mode_pause = BUZZER_KEY_PAUSE;
                    break;
                case BUZZER_LONGKEY:
                    buzzer_mode_counter = BUZZER_LONGKEY_COUNTER;
                    buzzer_mode_sound = BUZZER_LONGKEY_SOUND;
                    buzzer_mode_pause = BUZZER_LONGKEY_PAUSE;
                    break;
                case BUZZER_WARN:
                    buzzer_mode_counter = BUZZER_WARN_COUNTER;
                    buzzer_mode_sound = BUZZER_WARN_SOUND;
                    buzzer_mode_pause = BUZZER_WARN_PAUSE;
                    break;
            }
            buzzer_counter_01sec = 1;
        }
        buzzer_mode_index = BUZZER_NONE;
    }

    if (buzzer_fl != 0) {
        if (--buzzer_counter_01sec == 0) {
            buzzer_counter_01sec = INIT_TIMEOUT(0.1f);
            if (buzzer_init_fl == 0) {
                buzzer_init_fl = 1;
                buzzer_repeat_fl = 1;
                buzzer_counter_r = buzzer_mode_counter + 1;
            }

            if (buzzer_repeat_fl != 0) {
                buzzer_repeat_fl = 0;
                if (--buzzer_counter_r > 0) {
                    buzzer_snd_fl = 1;
                    buzzer_counter = buzzer_mode_sound;
                } else {
                    buzzer_fl = 0;
                    buzzer_init_fl = 0;
                }
            }

            if (buzzer_snd_fl != 0) {
                if (buzzer_counter == 0) {
                    buzzer_snd_fl = 0;
                    buzzer_counter = buzzer_mode_pause - 1;
                }
                SND_ON;
            }
            if (buzzer_snd_fl == 0) {
                if (buzzer_counter == 0) {
                    buzzer_repeat_fl = 1;
                }
                SND_OFF;
            }
            buzzer_counter--;
        }
    }
#endif
}

void int_adc_finish() {
#if defined(SIMPLE_ADC)

    adc_voltage.current = adc_read_value();

#ifdef MIN_MAX_VOLTAGES_SUPPORT
    if (adc_voltage.current < adc_voltage.min) {
        adc_voltage.min = adc_voltage.current;
    } else if (adc_voltage.current > adc_voltage.max) {
        adc_voltage.max = adc_voltage.current;
    }
#endif

    // read power supply status
    if (adc_voltage.current > THRESHOLD_VOLTAGE_ADC_VALUE) {
        acc_power_off_fl = 0;
    } else {
        acc_power_off_fl = 1;
    }

#else
    adc_value = adc_read_value();
#if defined(ADC_BUTTONS) || defined(FUEL_TANK_SUPPORT)        
    adc_item_t* h = &adc_items[_adc_ch];

    if (++_adc_ch >= sizeof (adc_items) / sizeof (adc_item_t)) {
        _adc_ch = 0;
    }

    // set next channel
    set_adc_channel(adc_items[_adc_ch].channel);

    h->handle(h->f);

    if (_adc_ch != 0) {
        // start next adc channel (first channel is started from auto trigger)
        start_adc();
    }
#else
    adc_item.handle(adc_item.f);
#endif

#endif
}

// interrupt routines ends

#if !defined(SIMPLE_ADC)

void adc_handler_voltage(filtered_value_t *f) {
    adc_voltage.current = calc_filtered_value(f, adc_value);
    //adc_voltage.current = adc_value;

#ifdef MIN_MAX_VOLTAGES_SUPPORT
    if (adc_voltage.current < adc_voltage.min) {
        adc_voltage.min = adc_voltage.current;
    } else if (adc_voltage.current > adc_voltage.max) {
        adc_voltage.max = adc_voltage.current;
    }
#endif

    if (adc_voltage.current > THRESHOLD_VOLTAGE_ADC_VALUE) {
        acc_power_off_fl = 0;
    } else {
        acc_power_off_fl = 1;
    }
};

#ifdef ADC_BUTTONS
void adc_handler_buttons(filtered_value_t *f) {
    if (/*   adc_value >= (ADC_BUTTONS_1V * 0 - ADC_BUTTONS_THRESHOLD) && */adc_value <= (ADC_BUTTONS_1V * 0 + ADC_BUTTONS_THRESHOLD)) {
        if (adc_key == 0 || adc_key == ADC_KEY_OK)
            adc_key = ADC_KEY_OK;
    } else if (adc_value >= (ADC_BUTTONS_1V * 1 - ADC_BUTTONS_THRESHOLD) && adc_value <= (ADC_BUTTONS_1V * 1 + ADC_BUTTONS_THRESHOLD)) {
        if (adc_key == 0 || adc_key == ADC_KEY_NEXT)
            adc_key = ADC_KEY_NEXT;
    } else if (adc_value >= (ADC_BUTTONS_1V * 2 - ADC_BUTTONS_THRESHOLD) && adc_value <= (ADC_BUTTONS_1V * 2 + ADC_BUTTONS_THRESHOLD)) {
        if (adc_key == 0 || adc_key == ADC_KEY_PREV)
            adc_key = ADC_KEY_PREV;
    } else {
        adc_key = 0;
    }
}
#endif

#ifdef FUEL_TANK_SUPPORT
void adc_handler_fuel_tank(filtered_value_t *f) {
    if (adc_fuel_tank_counter-- == 0) {
        adc_fuel_tank_counter = FUEL_TANK_ADC_INTERVAL;
        // normalize with adc_voltage.current
        if (config.settings.adc_fuel_normalize != 0) {
            adc_value = (uint16_t) ((uint24_t) adc_value * 1024 / adc_voltage.current);
        }
        adc_fuel_tank = calc_filtered_value(f, adc_value);
    }
}
#endif

#endif

uint16_t calc_filtered_value(filtered_value_t *f, uint16_t v) {
    if (f->filter == 0) {
        f->tmp = v;
        return (uint16_t) f->tmp;
    } else {
        if ((f->filter & FILTERED_VALUE_FIRST_SAMPLE) != 0) {
            f->filter &= ~FILTERED_VALUE_FIRST_SAMPLE;
            if (f->filter != 0) {
                f->tmp = v << f->filter;
            }
        } else {
            f->tmp = f->tmp - ((f->tmp + (uint16_t) (1 << (f->filter - 1))) >> f->filter) + v;
        }
        return (uint16_t) ((f->tmp + (uint16_t) (1 << (f->filter - 1))) >> f->filter);
    }
}

#ifdef ENCODER_SUPPORT
// A valid CW or CCW move returns 1, invalid returns 0.
void int_change_encoder_level() {
    static int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    static uint8_t prevNextCode = 0;
    static uint8_t store = 0;

    prevNextCode <<= 2;
    if (ENCODER_DATA) prevNextCode |= 0x02;
    if (ENCODER_CLK) prevNextCode |= 0x01;
    prevNextCode &= 0x0f;

    // If valid then store as 16 bit data.
    if (rot_enc_table[prevNextCode]) {
        store <<= 4;
        store |= prevNextCode;
        if (key_repeat_counter == 0) {
            if (store == 0x2b) {
                key3_press = 1;
            }
            if (store == 0x17) {
                key1_press = 1;
            }
            if (key1_press != 0 || key3_press != 0) {
                key_pressed = 1;
#ifdef SOUND_SUPPORT
                buzzer_mode_index = BUZZER_KEY;
#endif
            }
        }
    }
}
#endif

void handle_keys_up_down(uint8_t *v, uint8_t min_value, uint8_t max_value) {
    uint8_t _v = *v;
    if (key1_press != 0) {
        if (_v++ == max_value) {
            _v = min_value;
        }
        timeout_timer1 = 5;
    }
#if !defined(KEY3_SUPPORT)
    if (key2_press != 0) {
#elif defined(ENCODER_SUPPORT)
    if (key2_press != 0) {
        timeout_timer1 = 0;
    }
    if (key3_press != 0) {
#else
    if (key2_press != 0 || key3_press != 0) {
#endif
        if (_v-- == min_value) {
            _v = max_value;
        }
        timeout_timer1 = 5;
    }
    *v = _v;
}

void handle_keys_next_prev(uint8_t *v, uint8_t min_value, uint8_t max_value) {
    uint8_t _v = *v;
    // change cursor to next position
    if (key1_press != 0) {
        if (_v++ == max_value) {
            _v = min_value;
        }
        timeout_timer1 = 5;
    }

#if defined(KEY3_SUPPORT)
    // change cursor to prev position
    if (key3_press != 0) {
        if (_v-- == min_value) {
            _v = max_value;
        }
        timeout_timer1 = 5;
    }
#endif
    *v = _v;
}

/**
 * print symbols from symbols_str with [index] in buf at [len] position
 * @param len
 * @param index
 * @return 
 */
#define print_symbols_str(len, index) strcpy2(&buf[len], (char *)symbols_array, index)

/**
 * print aligned string from buffer (16 bytes total length)
 * @param len
 * @param align
 */
void _print16(uint8_t len, align_t align) {
    LCD_Write_String(buf, len, 16, align);
}

/**
 * print aligned string from buffer (8 bytes total length)
 * @param len
 * @param align
 */
void _print8(uint8_t len, align_t align) {
    LCD_Write_String(buf, len, 8, align);
}

/**
 * print aligned string with suffix from buffer (8 bytes total length)
 * @param len
 * @param pos
 * @param align
 * @return 
 */
uint8_t _print8_suffix(uint8_t len, unsigned char pos, align_t align) {
    len += print_symbols_str(len, pos);
    LCD_Write_String(buf, len, 8, align);
    return len;
}

/**
 * print fractional number [num/10^frac].[num%10^frac]
 * @param buf
 * @param num
 * @param frac numbers after '.'
 * @return 
 */
uint8_t print_fract(char * buf, uint16_t num, uint8_t frac) {
    uint8_t len = ultoa2(buf, num, 10);

    // add leading zeroes
    if (len < frac + 1) {
        add_leading_symbols(buf, '0', len, frac + 1);
        len = frac + 1;
    }

    // shift right and add '.'
    char *dst = &buf[len];
    char *src = dst - 1;
    while (frac--) {
        *dst-- = *src--;
    }
    *dst = '.';
    return len + 1;
}

void print_time_hm(uint8_t hour, uint8_t minute, align_t align) {
    bcd8_to_str(buf, hour);
    buf[2] = ':';
    bcd8_to_str(&buf[3], minute);
    _print8(5, align);
}

void print_time_dmy(uint8_t day, uint8_t month, uint8_t year) {
    if (day == 0x00 || day == 0xFF) {
        strcpy2(buf, (char*) &no_time_string, 0);
    } else {
        bcd8_to_str(buf, day);
        strcpy2(&buf[2], (char *) &month_array, bcd8_to_bin(month));
        buf[5] = '\'';
        bcd8_to_str(&buf[6], year);
    }
    _print8(8, ALIGN_LEFT);
}

void print_time(ds_time* time) {
    LCD_CMD(0x80);
    print_time_hm(time->hour, time->minute, ALIGN_LEFT);

    LCD_CMD(0x88);
    print_time_dmy(time->day, time->month, time->year);

    LCD_CMD(0xC0);
    _print16(strcpy2((char *) buf, (char*) day_of_week_array, time->day_of_week), ALIGN_LEFT);
}

uint16_t get_trip_average_speed(trip_t* t) {
    uint16_t average_speed = 0;
    if (t->time > 0) {
        average_speed = (uint16_t) ((uint32_t) ((t->odo * 36000UL) + (t->odo_temp * 36000UL / config.odo_const)) / t->time);
    }
    return average_speed;
}

uint16_t get_trip_odometer(trip_t* t) {
    //     //bug(?) in xc8 or proteus for 16f1938
    //     return (uint16_t) (t->odo * 10UL + (uint16_t) (t->odo_temp * 10UL / config.odo_const));

    uint16_t int_part = t->odo * 10;
    uint16_t frac_part = (uint16_t) (t->odo_temp * 10UL / config.odo_const);
    return int_part + frac_part;
}

/**
 * print trip time
 * @param t
 * @param align
 */
uint8_t print_trip_time(trip_t* t, align_t align) {
    uint16_t time = (uint16_t) (t->time / 60);

    len = ultoa2(buf, (uint16_t) (time / 60), 10);
    buf[len++] = ':';

    bcd8_to_str(&buf[len], bin8_to_bcd(time % 60));
    len += 2;
   
    //_print8(len, align);
    return _print8_suffix(len, POS_NONE, align);
}

/**
 * print trip average speed
 * @param t
 * @param align
 */
uint8_t print_trip_average_speed(trip_t* t, align_t align) {
    
    uint16_t average_speed = get_trip_average_speed(t);
    
    if (average_speed == 0) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        len = print_fract(buf, average_speed, 1);
    }

    return _print8_suffix(len, POS_KMH, align);
}

/**
 * print trip odometer (km)
 * @param t
 * @param align
 */
uint8_t print_trip_odometer(trip_t* t, align_t align) {
    uint16_t odo = get_trip_odometer(t);
    len = print_fract(buf, odo, 1);

    return _print8_suffix(len, POS_KM, align);
}

/**
 * print trip total fuel consumption (l)
 * @param t
 * @param align
 */
uint8_t print_trip_total_fuel(trip_t* t, align_t align) {
    len = print_fract(buf, t->fuel / 10, 1);

    return _print8_suffix(len, POS_LITR, align);
}

/**
 * print trip average fuel consumption (l/100km)
 * @param t
 * @param align
 */
uint8_t print_trip_average_fuel(trip_t* t, align_t align) {
    uint16_t odo = get_trip_odometer(t);
    if (t->fuel < AVERAGE_MIN_FUEL || odo < AVERAGE_MIN_DIST) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        len = print_fract(buf, (uint16_t) (t->fuel * 100UL / odo), 1);
    }

    return _print8_suffix(len, POS_LKM, align);
}

/**
 * print speed
 * @param speed
 * @param pos_prefix
 * @param frac
 * @param align
 */
uint8_t print_speed(uint16_t speed, uint8_t pos_prefix, uint8_t frac, align_t align) {
    len = print_symbols_str(0, pos_prefix);

    // use fractional by default
    len += print_fract(&buf[len], speed, 1);

    if (speed > 1000 || frac == 0) {
        // more than 100 km/h (or current speed), skip fractional
        len -= 2;
    }

    return _print8_suffix(len, POS_KMH, align);
}

/**
 * print taho
 * @param align
 */
uint8_t print_taho(align_t align) {
    if (taho_fl == 0) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        unsigned short res = (unsigned short) (((config.settings.par_injection != 0 ? (TAHO_CONST) : (TAHO_CONST*2)) / taho));
#ifdef TAHO_ROUND
        res = (res + TAHO_ROUND / 2) / TAHO_ROUND * TAHO_ROUND;        
#endif
        len = ultoa2(buf, res, 10);
    }

    return _print8_suffix(len, POS_OMIN, align);
}

/**
 * print fuel duration
 * @param align
 */
uint8_t print_fuel_duration(align_t align) {
//    if (fuel_duration == 0) {
//        len = strcpy2(buf, (char *) &empty_string, 0);
//    } else
    {
        uint16_t res = (uint16_t) (fuel_duration * (1000 / 250) / (TAHO_TIMER_TICKS_PER_PERIOD / 250));
        len = print_fract(buf, res, 2);
    }
    return _print8_suffix(len, POS_MS, align);
}

/**
 * print fuel consumption (l/h or l/100km)
 * @param fuel
 * @param kmh
 * @param drive_fl
 * @param align
 */
uint8_t print_fuel(uint16_t fuel, uint16_t kmh, uint8_t drive_fl, align_t align) {
    uint8_t pos;
    uint16_t t;
    if (drive_fl == 0) {
        // l/h
        t = (uint16_t) (((uint32_t) fuel * (uint32_t) fuel2_const / (uint32_t) config.fuel_const) / 10UL);
        pos = POS_LH;
    } else {
        // l/100km
        t = (uint16_t) ((((uint32_t) fuel * (uint32_t) odo_con4) / (uint32_t) kmh) / (uint8_t) config.fuel_const);
        pos = POS_LKM;
    }

    return _print8_suffix(print_fract(buf, t, 1), pos, align);
}

/**
 * print main odometer
 * @param align
 */
uint8_t print_main_odo(align_t align) {
    len = ultoa2(buf, (unsigned long) config.odo, 10);

    return _print8_suffix(len, POS_KM, align);
}

#if defined(TEMPERATURE_SUPPORT)
uint8_t print_temp(uint8_t index, align_t align) {

    uint8_t param = index & ~PRINT_TEMP_PARAM_MASK;
    index &= PRINT_TEMP_PARAM_MASK;

    _t = temps[index];

    if (_t == DS18B20_TEMP_NONE) {
        len = strcpy2(buf, (char *) &temp_sensors_array, 1);
    } else {
        uint8_t _pos = 0;
        if (_t & 0x8000)         // if the temperature is negative
        {
            _t = (~_t) + 1;      // change temperature value to positive form
            buf[_pos++] = '-';   // put minus sign (-)
        } else {
            if ((param & PRINT_TEMP_PARAM_NO_PLUS_SIGN) == 0) {
              buf[_pos++] = '+'; // put plus sign (+)
            }
        }
        _t = (unsigned short) ((_t >> 4) * 10 + (((_t & 0x000F) * 10) >> 4));

        // if ((param & PRINT_TEMP_PARAM_FRACT) != 0)
        // {
        //     len = print_fract(&buf[_pos], _t, 1) + 1;
        // }
        // else
        {
            len = ultoa2(&buf[_pos], _t / 10, 10) + _pos;
        }
    }

    if ((param & PRINT_TEMP_PARAM_HEADER) != 0) {
        add_leading_symbols(buf, ' ', len, 8);
        strcpy2(buf, (char *) &temp_sensors_array, (index + 1) + 1);
        len = 8;
    }

    uint8_t pos_suffix;
    if ((param & PRINT_TEMP_PARAM_DEG_SIGN) != 0) {
        pos_suffix = POS_CELS;
    } else {
        pos_suffix = POS_NONE;
    }
    
    return _print8_suffix(len, pos_suffix, align);
}

#endif

uint8_t print_voltage(uint16_t *adc_voltage, uint8_t prefix_pos, align_t align) {
    len = print_symbols_str(0, prefix_pos);
    len += print_fract(&buf[len], (uint16_t) (*adc_voltage << 5) / (uint8_t) (VOLTAGE_ADJUST_CONST_MAX - (config.vcc_const - VOLTAGE_ADJUST_CONST_MIN)), 1);

    return _print8_suffix(len, POS_VOLT, align);
}

void wait_refresh_timeout() {

    if (key2_longpress != 0) {
        screen_refresh = 1;
        timeout_timer1 = 0;
    }

    clear_keys_state();
    while (screen_refresh == 0 && timeout_timer1 != 0);
}

const time_editor_item_t time_editor_items_array[] = {
    {&time.hour, 0, 23, 0x81},
    {&time.minute, 0, 59, 0x84},
    {&time.day, 1, 31, 0x89},
    {&time.month, 1, 12, 0x8c},
    {&time.year, VERSION_YEAR, VERSION_YEAR + 10, 0x8f},
    {&time.day_of_week, 0, 23, 0xc0},
};

void screen_time(void) {

    read_ds_time();

    time_editor_item_t *time_editor_item;

    if (request_screen((char *) &time_correction_string) != 0) {

        uint8_t c = 0, save_time = 0;

#if defined(ENCODER_SUPPORT)
        uint8_t edit_mode = 0;
#endif
        LCD_Clear();
        timeout_timer1 = 5;
        while (timeout_timer1 != 0) {
            screen_refresh = 0;
#if defined(ENCODER_SUPPORT)
            if (key2_press != 0) {
                edit_mode = ~edit_mode;
            }
            if (edit_mode == 0) {
                handle_keys_next_prev(&c, 0, 6 - 1);
                time_editor_item = (time_editor_item_t *) &time_editor_items_array[c];
            } else if (key1_press != 0 || key3_press != 0) {           
#else
            handle_keys_next_prev(&c, 0, 6 - 1);
            time_editor_item = (time_editor_item_t *) &time_editor_items_array[c];

            if (key2_press) {
#endif

                save_time = 1;
#if !defined(ENCODER_SUPPORT)
                *time_editor_item->p = bcd8_inc(*time_editor_item->p, time_editor_item->min, time_editor_item->max);
#else
                if (key3_press != 0) {
                    *time_editor_item->p = bcd8_dec(*time_editor_item->p, time_editor_item->min, time_editor_item->max);
                } else if (key1_press != 0) {
                    *time_editor_item->p = bcd8_inc(*time_editor_item->p, time_editor_item->min, time_editor_item->max);
                }
#endif

#ifdef AUTO_DAY_OF_WEEK
                if (c >= 2 && c <= 4) {
                    //set day of week
                    const uint8_t mArr[12] = {6, 2, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

                    uint8_t tYear = bcd8_to_bin(time.year);
                    uint8_t tMonth = bcd8_to_bin(time.month);

                    uint8_t dow;
                    dow = tYear;
                    dow += tYear / 4;
                    dow += bcd8_to_bin(time.day);
                    dow += mArr[tMonth - 1];
                    if (((tYear % 4) == 0) && (tMonth < 3))
                        dow -= 1;
                    while (dow >= 7)
                        dow -= 7;
                    time.day_of_week = dow + 1;

                }
#endif                
                timeout_timer1 = 5;
            }


            LCD_CMD(LCD_CURSOR_OFF);
            print_time(&time);
            LCD_CMD(time_editor_item->pos);
#if defined(ENCODER_SUPPORT)
            LCD_CMD(edit_mode == 0 ? LCD_UNDERLINE_ON : LCD_BLINK_CURSOR_ON);
#else
            LCD_CMD(LCD_BLINK_CURSOR_ON);
#endif
            
            wait_refresh_timeout();
        }
        LCD_CMD(LCD_CURSOR_OFF);
        // save time
        if (save_time != 0) {
            set_ds_time(&time);
        }
        screen_refresh = 1;

    } else {
        print_time(&time);
    }
}

typedef enum {
    CHAREDIT_MODE_NONE = POS_NONE,
    CHAREDIT_MODE_KMH = POS_KMH,
    CHAREDIT_MODE_10000KM = POS_KM
} edit_value_char_t;


unsigned char edit_value_char(unsigned char v, edit_value_char_t mode, unsigned char min_value, unsigned char max_value) {
    timeout_timer1 = 5;
    while (timeout_timer1 != 0) {
        screen_refresh = 0;

        handle_keys_up_down(&v, min_value, max_value);

        len = ultoa2(buf, (unsigned long) (mode == CHAREDIT_MODE_10000KM ? (v * 1000L) : v), 10);

        LCD_CMD(0xC4);
        _print8_suffix(len, (unsigned char) mode, ALIGN_RIGHT);

        wait_refresh_timeout();
    }

    screen_refresh = 1;

    return v;
}

unsigned long edit_value_long(unsigned long v, unsigned long max_value) {
    // number of symbols to edit
    unsigned char max_len = ultoa2(buf, max_value, 10);
#ifdef KEY3_SUPPORT        
    unsigned char _max_symbol_pos0 = buf[0];
#endif    
#if defined(ENCODER_SUPPORT)
    uint8_t edit_mode = 0, buf_prev;
#endif
    if (v > max_value) {
        v = max_value;
    }

    // convert value
    unsigned char v_len = ultoa2(buf, v, 10);

    add_leading_symbols(buf, '0', v_len, max_len);

    unsigned char cursor_pos = 0xC0 + (16 - max_len) / 2U;
    unsigned char pos = 0;

    timeout_timer1 = 5;
    while (timeout_timer1 != 0) {
        screen_refresh = 0;

#if defined(ENCODER_SUPPORT)
        if (edit_mode == 0) {
            handle_keys_next_prev(&pos, 0, max_len - 1);
        } else {
            buf_prev = buf[pos];
            handle_keys_next_prev(&buf[pos], '0', '9');
        }
#else
        handle_keys_next_prev(&pos, 0, max_len - 1);
#endif

        // edit number in cursor position
        if (key2_press != 0) {
            key2_press = 0;
#if defined(ENCODER_SUPPORT)
            edit_mode = ~edit_mode;
#else
            if (++buf[pos] > '9') {
                buf[pos] = '0';
            }
#endif            

            timeout_timer1 = 5;
        }

        unsigned long _t = strtoul2(buf);
        if (_t > max_value) {
#if defined(ENCODER_SUPPORT)
            buf[pos] = buf_prev;
#else
            buf[pos] = '0';
#endif
        }

        LCD_CMD(LCD_CURSOR_OFF);

        LCD_CMD(cursor_pos);
        _print8(max_len, ALIGN_LEFT);

        LCD_CMD(cursor_pos + pos);
#if defined(ENCODER_SUPPORT)
        LCD_CMD(edit_mode == 0 ? LCD_UNDERLINE_ON : LCD_BLINK_CURSOR_ON);
#else
        LCD_CMD(LCD_BLINK_CURSOR_ON);
#endif
        wait_refresh_timeout();
    }

    LCD_CMD(LCD_CURSOR_OFF);
    screen_refresh = 1;

    return strtoul2(buf);
}

uint16_t edit_value_bits(uint16_t v, char* str) {

    uint8_t pos = 0;

    timeout_timer1 = 5;
    while (timeout_timer1 != 0) {
        screen_refresh = 0;

        handle_keys_next_prev(&pos, 0, 16 - 1);

        // edit number in cursor position
        if (key2_press != 0) {
            v ^= (1 << (15 - pos));
            timeout_timer1 = 5;
        }

        LCD_CMD(LCD_CURSOR_OFF);

        add_leading_symbols(buf, '0', ultoa2(buf, v, 2), 16);

        unsigned char _onoff = (buf[pos] - '0') + 1;

        char *p = &buf[0];
        uint8_t _i = 16;
        while (_i-- > 0) {
            // replace 0 with '-' and '1' with '*'
            if (*p == '0') {
                *p++ = '-';
            } else {
                *p++ = '*';
            }
        }

        LCD_CMD(0xC0);
        _print16(16, ALIGN_NONE);

        _memset(buf, ' ', 16);
        len = strcpy2((char*) buf, (char *) str, pos + 1);
        if (len != 0) {
            // print on/off
            strcpy2((char*) &buf[12], (char *) &on_off_array, _onoff);
        }
        LCD_CMD(0x80);
        _print16(16, ALIGN_NONE);

        LCD_CMD(0xC0 + pos);
        LCD_CMD(LCD_BLINK_CURSOR_ON);

        wait_refresh_timeout();
    }

    LCD_CMD(LCD_CURSOR_OFF);
    screen_refresh = 1;

    return v;
}

unsigned char request_screen(char* request_str) {
    unsigned char res = 0;
    if (key2_longpress != 0) {
        clear_keys_state();

        LCD_Clear();

        LCD_CMD(0x80);
        _print16(strcpy2(buf, request_str, 0), ALIGN_CENTER);

        timeout_timer1 = 5;
        while (timeout_timer1 != 0 && NO_KEY_PRESSED);

        if (key2_press != 0) {
            res = 1;
        }

        clear_keys_state();
        screen_refresh = 1;
    }
    return res;
}

unsigned char select_param(unsigned char* param, unsigned char total) {
    if (key2_press != 0) {
        key2_press = 0;
        *param = *param + 1;
    }
    if (*param >= total) {
        *param = 0;
    }
    return *param;
}

typedef enum {
#if defined(TEMPERATURE_SUPPORT)
    selected_param_temp,
#endif
    selected_param_voltage,
    selected_param_trip_time,
    selected_param_trip_odometer,
    selected_param_trip_average_fuel,
    selected_param_trip_average_speed,
    selected_param_trip_total_fuel,
    selected_param_max_speed,
    selected_param_fuel_duration,
    selected_param_total
} selected_param_t;

void print_selected_param1(align_t align) {
    switch (select_param(&main_param, selected_param_total)) {
#if defined(TEMPERATURE_SUPPORT)
        case selected_param_temp:
            print_temp((config.settings.show_inner_temp != 0 ? TEMP_IN : TEMP_OUT) | PRINT_TEMP_PARAM_FRACT | PRINT_TEMP_PARAM_DEG_SIGN, align);
            break;
#endif            
        case selected_param_voltage:
            print_voltage((uint16_t *) &adc_voltage.current, POS_NONE, align);
            break;
        case selected_param_trip_time:
            print_trip_time(&trips.tripC, align);
            break;
        case selected_param_trip_odometer:
            print_trip_odometer(&trips.tripC, align);
            break;
        case selected_param_trip_average_fuel:
            print_trip_average_fuel(&trips.tripC, align);
            break;
        case selected_param_trip_average_speed:
            print_trip_average_speed(&trips.tripC, align);
            break;
        case selected_param_trip_total_fuel:
            print_trip_total_fuel(&trips.tripC, align);
            break;
        case selected_param_max_speed:
            print_speed(trips.tripC_max_speed, POS_MAX, 1, align);
            break;
        case selected_param_fuel_duration:
            print_fuel_duration(align);
            break;
    }
}

#ifdef EXTENDED_ACCELERATION_MEASUREMENT
accel_meas_limits_t accel_meas_limits[4] = {
    {ACCEL_MEAS_LOWER_0,              speed_const(ACCEL_MEAS_UPPER_0)},
    {ACCEL_MEAS_LOWER_1,              speed_const(ACCEL_MEAS_UPPER_1)},
    {speed_const(ACCEL_MEAS_LOWER_2), speed_const(ACCEL_MEAS_UPPER_2)},
    {speed_const(ACCEL_MEAS_LOWER_3), speed_const(ACCEL_MEAS_UPPER_3)},
};
#endif

void acceleration_measurement(uint8_t index) {
    LCD_CMD(0x80);
    _print16(strcpy2(buf, (char *) &accel_meas_wait_string, 0), ALIGN_CENTER);

    accel_meas_fl = 0; accel_meas_ok_fl = 0; accel_meas_timer = 0; accel_meas_final_fl = 0; _accel_meas_exit = 0;

#ifdef EXTENDED_ACCELERATION_MEASUREMENT
    accel_meas_lower_const = (unsigned short) (accel_meas_limits[index].lower / config.odo_const);
    accel_meas_upper_const = (unsigned short) (accel_meas_limits[index].upper / config.odo_const);
#endif
    
    _memset(buf, '=', 16);

    // 16 sec waiting for start
    timeout_timer1 = 16;

    timeout_timer2 = 0;
    while (_accel_meas_exit == 0 && NO_KEY_PRESSED) {
        if (timeout_timer1 != 0 && drive_fl == 0) {
            if (timeout_timer2 == 0) {
                timeout_timer2 = INIT_TIMEOUT(0.25f);
                LCD_CMD(0xC0);
                _print16(timeout_timer1, ALIGN_LEFT);
            }
        } else {
            if (timeout_timer1 != 0) {
                if (accel_meas_ok_fl == 0 && accel_meas_process_fl == 0) {
                    // 30 sec for acceleration measurement
                    timeout_timer1 = 30;
                    accel_meas_process_fl = 1;

                    LCD_Clear();
                }

                len = print_fract((char*) buf, accel_meas_timer, 2);
                len += print_symbols_str(len, POS_SEC);
                add_leading_symbols(buf, ' ', len, 16);

                len = ultoa2((char*) buf, accel_meas_drive_fl != 0 ? (uint16_t) ((360000UL * SPEED_TIMER_TICKS_PER_PERIOD / config.odo_const) / accel_meas_speed) : 0, 10); // integer
                /*len += */print_symbols_str(len, POS_KMH);

                LCD_CMD(0xC0);
                _print16(16, ALIGN_CENTER);
            }

            if (timeout_timer1 != 0 && accel_meas_ok_fl == 0) {
                timeout_timer2 = INIT_TIMEOUT(0.1f); while (timeout_timer2 != 0);
            } else {
                if (accel_meas_ok_fl != 0 && accel_meas_final_fl == 0) {
                    // print final result
                    accel_meas_final_fl = 1;
                } else {
                    // timeout or 100 km/h
                    accel_meas_process_fl = 0;
                    if (timeout_timer1 == 0) {
                        // timeout
                        LCD_CMD(0xC0);
                        _print16(strcpy2(buf, (char *) &timeout_string, 0), ALIGN_CENTER);
                    }
#ifdef JOURNAL_SUPPORT
                    else {
#ifdef EXTENDED_ACCELERATION_MEASUREMENT
                        journal_save_accel(index);
#else
                        journal_save_accel(0);
#endif
                    }
#endif
#ifdef SOUND_SUPPORT
                    buzzer_mode_index = BUZZER_WARN;
#endif
                    timeout_timer1 = 10; while (timeout_timer1 != 0 && NO_KEY_PRESSED);
                    _accel_meas_exit = 1;
                }
            }
        }
    }
    accel_meas_process_fl = 0;
    clear_keys_state();
}

#ifdef EXTENDED_ACCELERATION_MEASUREMENT
void select_acceleration_measurement() {
    clear_keys_state();

    LCD_Clear();

    uint8_t v = 0, max_value = sizeof (accel_meas_limits) / sizeof (accel_meas_limits[0]) - 1, index = 0xFF;

    timeout_timer1 = 5;
    while (timeout_timer1 != 0) {
        screen_refresh = 0;

        handle_keys_next_prev(&v, 0, max_value);

        if (key2_press != 0) {
            timeout_timer1 = 0;
            index = v;
        }

        len = strcpy2(buf, (char *) &accel_meas_timing_string, 0);
        len += strcpy2(&buf[len], (char *) accel_meas_array, v + 1);

        LCD_CMD(0x80);
        _print16(len, ALIGN_CENTER);

        wait_refresh_timeout();
    }

    if (index != 0xFF) {
        acceleration_measurement(index);
    }
}
#endif

void screen_main(void) {

    //  1) drive_min_speed_fl == 0 && motor_fl == 0 (engine off)
    //  time        main_odo
    //  temp/odo    voltage
    //
    //  2) drive_min_speed_fl == 0 && motor_fl != 0 (idling)
    //  time        taho
    //  param1      fuel_lh
    //
    //  3) drive_min_speed_fl != 0                  (moving)
    //  speed       taho
    //  param1      fuel_km

    LCD_CMD(0x80);
    if (drive_min_speed_fl == 0) {
        read_ds_time();
        print_time_hm(time.hour, time.minute, ALIGN_LEFT);
    } else {
        print_speed(speed, POS_NONE, 0, ALIGN_LEFT);
    }
    
    if (drive_min_speed_fl == 0 && motor_fl == 0) {
        print_main_odo(ALIGN_RIGHT);
    } else {
        print_taho(ALIGN_RIGHT);
    }

    LCD_CMD(0xC0);
    if (drive_min_speed_fl == 0 && motor_fl == 0) {
#if defined(TEMPERATURE_SUPPORT)
        print_temp((config.settings.show_inner_temp ? TEMP_IN : TEMP_OUT) | PRINT_TEMP_PARAM_FRACT | PRINT_TEMP_PARAM_DEG_SIGN, ALIGN_LEFT);
#else
        print_trip_odometer(&trips.tripC, ALIGN_LEFT);
#endif
        print_voltage((uint16_t *) &adc_voltage.current, POS_NONE, ALIGN_RIGHT);
    } else {
        print_selected_param1(ALIGN_LEFT);
        print_fuel(fuel, kmh, drive_min_speed_fl, ALIGN_RIGHT);
    }

#ifdef EXTENDED_ACCELERATION_MEASUREMENT
    if (drive_fl == 0 && motor_fl != 0 && key2_longpress != 0) {
        select_acceleration_measurement();
    }
#else
    if (drive_fl == 0 && motor_fl != 0 && request_screen((char *) &accel_meas_string) != 0) {
        acceleration_measurement(0);
    }
#endif        
}

void clear_trip(bool force, trip_t* trip) {
    if (force || request_screen((char *) &reset_string) != 0) {
#ifdef JOURNAL_SUPPORT
        journal_save_trip(trip);
#endif
        _memset(trip, 0, sizeof(trip_t));
        
        save_eeprom_trips();
    }
}

void screen_trip() {
    trip_t *trip;
    uint8_t trips_pos;
    
    uint8_t item_index = items_main[c_item].index;

    if (item_index == SCREEN_INDEX_TRIP_C) {
            trip = &trips.tripC;
            trips_pos = config.settings.daily_tripc ? TRIPS_POS_DAY : TRIPS_POS_CURR;
    } else if (item_index == SCREEN_INDEX_TRIP_A) {
            trip = &trips.tripA;
            trips_pos = TRIPS_POS_A;
    } else /*if (item_index == SCREEN_INDEX_TRIP_B) */{
            trip = &trips.tripB;
            trips_pos = TRIPS_POS_B;
    };

    len = strcpy2(buf, (char *) &trip_string, 0);
    len += strcpy2(&buf[len], (char *) trips_array, trips_pos);

    LCD_CMD(0x80);
    _print8(len, ALIGN_LEFT);

    print_trip_odometer(trip, ALIGN_RIGHT);
    
    LCD_CMD(0xC0);
    switch (select_param(&tmp_param, 2)) {
        case 0:
            print_trip_average_fuel(trip, ALIGN_LEFT);
            print_trip_average_speed(trip, ALIGN_RIGHT);
            break;
        case 1:
            print_trip_time(trip, ALIGN_LEFT);
            print_trip_total_fuel(trip, ALIGN_RIGHT);
            break;
    }

    clear_trip(false, trip);
}

#if defined(TEMPERATURE_SUPPORT) || defined(MIN_MAX_VOLTAGES_SUPPORT) || defined(CONTINUOUS_DATA_SUPPORT)

typedef enum {
#if defined(TEMPERATURE_SUPPORT)
    screen_misc_temperature,
#endif
#if defined(MIN_MAX_VOLTAGES_SUPPORT)
    screen_misc_voltages,
#endif
#if defined(CONTINUOUS_DATA_SUPPORT)
    screen_misc_cd,
#endif
    screen_misc_max
} screen_misc_items;

void screen_misc() {
    if (config.settings.show_misc_screen == 0) {
        item_skip = 1;
    } else {
        LCD_CMD(0x80);
        switch(select_param(&misc_param, screen_misc_max)) {
#if defined(TEMPERATURE_SUPPORT)
            case screen_misc_temperature:
                // show temp sensors config on ok key longpress
#if defined(DS18B20_CONFIG)
                if (key2_longpress != 0) {
                    key2_longpress = 0;

                    _print16(strcpy2(buf, (char *) config_menu_array, TEMP_SENSOR_INDEX), ALIGN_LEFT);
                    config_screen_temp_sensors();
                    clear_keys_state();
                    screen_refresh = 1;
                } else
#endif
                {
                    print_temp(TEMP_OUT | PRINT_TEMP_PARAM_HEADER | PRINT_TEMP_PARAM_FRACT, ALIGN_RIGHT);
                    _print8(0, ALIGN_RIGHT); //empty 

                    LCD_CMD(0xC0);
                    print_temp(TEMP_IN | PRINT_TEMP_PARAM_HEADER | PRINT_TEMP_PARAM_FRACT, ALIGN_RIGHT);
                    print_temp(TEMP_ENGINE | PRINT_TEMP_PARAM_HEADER | PRINT_TEMP_PARAM_NO_PLUS_SIGN, ALIGN_RIGHT);

                    // force faster temperature update
                    if (temperature_conv_fl == 0 && timeout_temperature > FORCED_TIMEOUT_TEMPERATURE) {
                        timeout_temperature = FORCED_TIMEOUT_TEMPERATURE;
                    }
                }
                break;
#endif
#if defined(MIN_MAX_VOLTAGES_SUPPORT)
            case screen_misc_voltages:
                if (request_screen((char *) &reset_string) != 0) {
                    adc_voltage.min = adc_voltage.max = adc_voltage.current;
                }
                _print8(strcpy2(buf, (char *) voltage_string, 0), ALIGN_LEFT);
                print_voltage((uint16_t*) &adc_voltage.current, POS_NONE, ALIGN_RIGHT);

                LCD_CMD(0xC0);
                print_voltage((uint16_t*) &adc_voltage.min, POS_MIN, ALIGN_LEFT);
                print_voltage((uint16_t*) &adc_voltage.max, POS_MAX, ALIGN_RIGHT);
                break;
#endif
#if defined(CONTINUOUS_DATA_SUPPORT)
            case screen_misc_cd:
                if (request_screen((char *) &reset_string) != 0) {
                    cd.filter = 0;
                    cd_init();
                }

#ifdef FUEL_TANK_SUPPORT
                add_leading_symbols(buf, ' ', ultoa2(buf, adc_fuel_tank, 10), 16);
                strcpy2(buf, (char *) continuous_data_string, 0);
                _print16(16, ALIGN_NONE);
#else
                _print16(strcpy2(buf, (char *) continuous_data_string, 0), ALIGN_LEFT);
#endif
                
                LCD_CMD(0xC0);
                if (continuous_data_fl != 0) {
                    print_speed(cd_speed, POS_NONE, 0, ALIGN_LEFT);
                    print_fuel(cd_fuel, cd_kmh, drive_min_cd_speed_fl, ALIGN_RIGHT);
                } else {
                    len = strcpy2(buf, (char *) &empty_string, 0);
                    _print8_suffix(len, POS_KMH, ALIGN_LEFT);
                    _print8_suffix(len, POS_LKM, ALIGN_RIGHT);
                }
                break;
#endif
        }

    }
}
#endif

#ifdef SERVICE_COUNTERS_SUPPORT
/**
 * get motor hours (based on rpm or time)
 * @return 
 */
uint16_t get_mh() {
    if (config.settings.mh_rpm != 0) {
        // rpm based motor hours (rpm / 96000)
        return (uint16_t) (services.mh.rpm / 96000UL);
    } else {
        // time based motor hours
        return (uint16_t) (services.mh.time / 3600UL);
    }
}

void screen_service_counters() {
    
    srv_t* srv;
    service_time_t s_time;
    unsigned short v;
    
    select_param(&service_param, 5);

    LCD_CMD(0x80);
    _print16(strcpy2((char*)buf, (char *) &service_counters_array, service_param + 1), ALIGN_LEFT);

    if (service_param == 0) {
        srv = &services.srv[0];
        v = get_mh();
    } else {
        srv = &services.srv[service_param - 1];
        v = srv->counter;
    }

    len = ultoa2(buf, v, 10);

    len += print_symbols_str(len, service_param == 0 ? POS_HOUR : POS_KM);

    buf[len++] = ' ';
    LCD_CMD(0xC0);
    _print8(len, ALIGN_RIGHT);
    
    s_time = srv->time;

    LCD_CMD(0xC8);
    print_time_dmy(s_time.day, s_time.month, s_time.year);
    
    if (request_screen((char *) &reset_string) != 0) {
        read_ds_time();
        if (service_param == 0 || service_param == 1) {
            services.mh.time = 0;
            services.mh.rpm = 0;
        }
        srv->counter = 0;
        srv->time.day = time.day;
        srv->time.month = time.month;
        srv->time.year = time.year;
    }
}
#endif

#ifdef JOURNAL_SUPPORT

__bit journal_support;

journal_header_t journal_header = {
    // journal_type_pos_t
    {
        {0xFF, J_EEPROM_TRIPC_COUNT},
        {0xFF, J_EEPROM_TRIPA_COUNT},
        {0xFF, J_EEPROM_TRIPB_COUNT},
        {0xFF, J_EEPROM_ACCEL_COUNT}
    },
};

void journal_update_header() {
    JOURNAL_write_eeprom_block((unsigned char *) &journal_header, J_EEPROM_MARK_POS + 8, sizeof (journal_header));
}

void journal_check_eeprom() {
    // check mark
    bool init_fl = true;
    while (1) {
        JOURNAL_read_eeprom_block((unsigned char *) &buf, J_EEPROM_MARK_POS, 8);
        if (memcmp(&buf, &journal_mark, (sizeof(journal_mark) - 1) <= 8 ? (sizeof (journal_mark) - 1) : 8) != 0) {
            if (init_fl) {
                // save init values on first attempt
                memcpy(&buf, &journal_mark, (sizeof (journal_mark) - 1) <= 8 ? (sizeof (journal_mark) - 1) : 8);
                JOURNAL_write_eeprom_block((unsigned char *) &buf, J_EEPROM_MARK_POS, 8);

                read_ds_time();
                fill_trip_time(&journal_header.time_trip_c);
                fill_trip_time(&journal_header.time_trip_a);
                fill_trip_time(&journal_header.time_trip_b);

                journal_update_header();
            } else {
                // set flag for no journal eeprom
                journal_support = 0;
                break;
            }
            init_fl = false;
        } else {
            // set flag for journal support
            journal_support = 1;
            break;
        }
    }
    if (journal_support != 0) {
        // read journal header
        JOURNAL_read_eeprom_block((unsigned char *) &journal_header, J_EEPROM_MARK_POS + 8, sizeof (journal_header));
    }
}

uint16_t journal_find_eeaddr(uint8_t index, int8_t item_index) {
    uint16_t eeaddr = J_EEPROM_DATA;
    for (uint8_t i = 0; i < index; i++) {
        eeaddr += sizeof(journal_trip_item_t) * journal_header.journal_type_pos[i].max;
    }
    journal_type_pos_t *pos = &journal_header.journal_type_pos[index];
    if (item_index == -1) {
        if (++pos->current >= pos->max) {
            pos->current = 0;
        }
        item_index = (int8_t) pos->current;
    }
    return eeaddr + (uint16_t) ((index == 3 ? sizeof(journal_accel_item_t) : sizeof(journal_trip_item_t)) * ((uint8_t) item_index));
}

void journal_save_trip(trip_t *trip) {
    if (journal_support == 0) return;
    
    uint8_t index;
    trip_time_t *trip_time;

    journal_trip_item_t trip_item;

    if (trip == &trips.tripC) {
        index = 0;
        trip_item.start_time = journal_header.time_trip_c;
        trip_time = &journal_header.time_trip_c;
    } else if (trip == &trips.tripA) {
        index = 1;
        trip_item.start_time = journal_header.time_trip_a;
        trip_time = &journal_header.time_trip_a;
    } else if (trip == &trips.tripB) {
        index = 2;
        trip_item.start_time = journal_header.time_trip_b;
        trip_time = &journal_header.time_trip_b;
    } else {
        return;
    }

    uint16_t odo = get_trip_odometer(trip);

    // skip if zero distance
    if (odo != 0) {
        trip_item.status = JOURNAL_ITEM_OK;
        memcpy(&trip_item.trip, trip, sizeof(trip_t));
        // save trip item
        JOURNAL_write_eeprom_block((unsigned char *) &trip_item, journal_find_eeaddr(index, -1), sizeof(journal_trip_item_t));
    }
    
    // update header time
    read_ds_time();
    fill_trip_time(trip_time);

    // update header
    journal_update_header();

}

void journal_save_accel(uint8_t index) {
    if (journal_support == 0) return;

    journal_accel_item_t accel_item;

    accel_item.status = JOURNAL_ITEM_OK;
    accel_item.time = accel_meas_timer;

#ifdef EXTENDED_ACCELERATION_MEASUREMENT
    switch (index) {
        case 0:
            accel_item.lower = ACCEL_MEAS_LOWER_0;
            accel_item.upper = ACCEL_MEAS_UPPER_0;
            break;
        case 1:
            accel_item.lower = ACCEL_MEAS_LOWER_1;
            accel_item.upper = ACCEL_MEAS_UPPER_1;
            break;
        case 2:
            accel_item.lower = ACCEL_MEAS_LOWER_2;
            accel_item.upper = ACCEL_MEAS_UPPER_2;
            break;
        case 3:
            accel_item.lower = ACCEL_MEAS_LOWER_3;
            accel_item.upper = ACCEL_MEAS_UPPER_3;
            break;
    }
#else
    accel_item.lower = 0;
    accel_item.upper = 100;
#endif

    read_ds_time();
    fill_trip_time(&accel_item.start_time);

    // save accel item
    JOURNAL_write_eeprom_block((unsigned char *) &accel_item, journal_find_eeaddr(3, -1), sizeof (journal_accel_item_t));
    
    // update header
    journal_update_header();
}


uint8_t journal_print_item_time(char *buf, trip_time_t *trip_time) {
    uint8_t len = 0;
    bcd8_to_str(&buf[len], trip_time->day);
    len += 2;
    {
        len += strcpy2(&buf[len], (char *) &month_array, bcd8_to_bin(trip_time->month));
        buf[len++] = '\'';
    }
//    {
//        buf[len++] = '.';
//        bcd8_to_str(&buf[len], trip_time->month);
//        len += 2;
//        buf[len++] = '.';
//    }
    bcd8_to_str(&buf[len], trip_time->year);
    len += 2;
    buf[len++] = ' ';
//    {
//        buf[len++] = ' ';
//        buf[len++] = ' ';
//    }
    bcd8_to_str(&buf[len], trip_time->hour);
    len += 2;
    buf[len++] = ':';
    bcd8_to_str(&buf[len], trip_time->minute);
    len += 2;
    return len;
}

void screen_journal_viewer() {
    char _buf[4];  // 4 symbols for index (iii.)

    if (journal_support == 0) {
        item_skip = 1;
    } else {

        LCD_CMD(0x80);
        _print16(strcpy2(buf, (char *) &journal_viewer_string, 0), ALIGN_LEFT);

        LCD_CMD(0xC0);
        _print16(0, ALIGN_NONE);

        if (key2_press != 0) {
            key2_press = 0;

            uint8_t journal_type = 0;

            timeout_timer1 = 5;
            while (timeout_timer1 != 0) {
                screen_refresh = 0;

                handle_keys_next_prev(&journal_type, 0, 3);

                LCD_CMD(0x80);
                _print16(strcpy2(buf, (char *) &journal_viewer_string, 0), ALIGN_LEFT);

                LCD_CMD(0xC0);
                buf[0] = '1' + journal_type;
                buf[1] = '.';
                _print16(strcpy2(&buf[2], (char *) journal_viewer_items_array, journal_type + 1) + 2, ALIGN_LEFT);

                if (key2_press != 0) {
                    key2_press = 0;

                    uint8_t item_current = journal_header.journal_type_pos[journal_type].current;
                    uint8_t item_max = journal_header.journal_type_pos[journal_type].max;

                    uint8_t item_num = 0;
                    uint8_t item_prev = ~item_num;

                    // item buffer
                    unsigned char item[sizeof(journal_trip_item_t) >= sizeof(journal_accel_item_t) ? sizeof(journal_trip_item_t) : sizeof(journal_accel_item_t)];
                    journal_accel_item_t *accel_item;
                    journal_trip_item_t *trip_item;
                    trip_time_t *trip_time;

                    uint8_t item_page = 0;
                    uint8_t item_page_max = journal_type == 3 ? 0 : 2;

                    timeout_timer1 = 5;
                    while (timeout_timer1 != 0) {
                        screen_refresh = 0;

                        if (item_current != 0xFF) {
                            handle_keys_next_prev(&item_num, 0, item_max - 1);

                            if (item_prev != item_num) {
                                uint8_t item_index = item_current + item_max - item_num;
                                if (item_num <= item_current) {
                                    item_index -= item_max;
                                }

                                while (1) {
                                    // read item from eeprom
                                    if (journal_type == 3) {
                                        // accel
                                        JOURNAL_read_eeprom_block((unsigned char *) &item, journal_find_eeaddr(journal_type, (int8_t) (item_num == 0 ? item_current : item_index)), sizeof(journal_accel_item_t));
                                        accel_item = (journal_accel_item_t *) &item;
                                        trip_time = &accel_item->start_time;
                                    } else {
                                        // trip
                                        JOURNAL_read_eeprom_block((unsigned char *) &item, journal_find_eeaddr(journal_type, (int8_t) (item_num == 0 ? item_current : item_index)), sizeof(journal_trip_item_t));
                                        trip_item = (journal_trip_item_t *) &item;
                                        trip_time = &trip_item->start_time;
                                    }
                                    // check, if item is valid. if not, read first item
                                    if (item[0] == JOURNAL_ITEM_OK || item_num == 0) {
                                        break;
                                    }
                                    item_num = 0;
                                };
                                item_page = 0;
                                
                                item_prev = item_num;

                                if (item[0] != JOURNAL_ITEM_OK && item_num == 0) {
                                    item_current = 0xFF;
                                }
                            }
                            
                            if (item_current != 0xFF) {

                                LCD_CMD(0xC0);
                                // show journal item data
                                if (journal_type == 3) {
                                    // accel_item

                                    // upper-lower
                                    len = ultoa2(buf, accel_item->lower, 10);
                                    buf[len++] = '-';
                                    len += ultoa2(&buf[len], accel_item->upper, 10);

                                    _print8(len, ALIGN_LEFT);

                                    // time
                                    len = print_fract(buf, accel_item->time, 2);
                                    _print8_suffix(len, POS_SEC, ALIGN_RIGHT);

                                } else {
                                    // trip_item

                                    switch(item_page) {
                                        case 0:
                                            print_trip_odometer(&trip_item->trip, ALIGN_LEFT);
                                            print_trip_average_fuel(&trip_item->trip, ALIGN_RIGHT);
                                            break;
                                        case 1:
                                            print_trip_average_speed(&trip_item->trip, ALIGN_LEFT);
                                            print_trip_time(&trip_item->trip, ALIGN_RIGHT);
                                            break;
                                        case 2:
                                            print_trip_total_fuel(&trip_item->trip, ALIGN_LEFT);
                                            _print8(0, ALIGN_RIGHT);
                                            break;
                                    }
                                }

                                _memset(buf, ' ', 16);
                                len = journal_print_item_time((char *) buf, trip_time);

                                uint8_t _len = ultoa2(_buf, item_num + 1, 10);
                                _buf[_len++] = '.';
                                
                                add_leading_symbols(buf, ' ', (16 - _len), 16);
                                
                                memcpy(buf, _buf, _len);
                                
                                len += _len;
                                if (len > 16) {
                                    len = 16;
                                }

                                LCD_CMD(0x80);
                                _print16(len, ALIGN_LEFT);

                                if (key2_press != 0) {
                                    timeout_timer1 = 5;
                                    if (++item_page > item_page_max) {
                                        item_page = 0;
                                    }
                                }
                            }
                        }

                        if (item_current == 0xFF) {
                            // no items for current journal
                            LCD_CMD(0x80);
                            _print16(strcpy2(buf, (char *) journal_viewer_items_array, journal_type + 1), ALIGN_LEFT);
                            LCD_CMD(0xC0);
                            _print16(strcpy2(buf, (char *) &journal_viewer_no_items_string, 0), ALIGN_LEFT);

                            if (key2_press != 0) {
                                timeout_timer1 = 0;
                                screen_refresh = 1;
                            }
                        }

                        wait_refresh_timeout();
                    }
                    
                    timeout_timer1 = 5;
                }

                wait_refresh_timeout();
            }
            
            screen_refresh = 1;
            c_item = 0;
        }
    }
}

#endif

void config_screen_fuel_constant() {
    config.fuel_const = edit_value_char(config.fuel_const, CHAREDIT_MODE_NONE, 1, 255);
}

void config_screen_vss_constant() {
    config.odo_const = (uint16_t) edit_value_long(config.odo_const, 29999L);
}

void config_screen_total_trip() {
    config.odo = edit_value_long(config.odo, 999999L);
}

void config_screen_settings_bits() {
    config.settings.word = edit_value_bits(config.settings.word, (char *) &settings_bits_array);
}

#ifdef MIN_SPEED_CONFIG
void config_screen_min_speed() {
    config.selected_param.min_speed = edit_value_char(config.selected_param.min_speed, CHAREDIT_MODE_KMH, 1, 10);
}
#endif

#if defined(DS18B20_CONFIG_EXT)

__bit config_temperature_conv_fl;

/**
 * extended version of temp sensors' configuration (use onewire search)
 * all sensors can be connected at once
 */
void config_screen_temp_sensors() {
    unsigned char tbuf[24];

    uint16_t _temps[3] = {DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE};
    uint8_t _t_num[3] = {0, 0, 0};
    uint8_t current_device = 0;

    _memset(buf, ' ', 16);
    strcpy2(buf, (char *) &temp_no_sensors, 0);

    uint8_t num_devices = onewire_search_devices((uint8_t *) tbuf, 3);
    
    ds18b20_start_conversion(); config_temperature_conv_fl = 0; timeout_timer2 = 100;

    timeout_timer1 = 5;
    while (timeout_timer1 != 0) {
        screen_refresh = 0;
        
        if (num_devices != 0) {

            if (config_temperature_conv_fl == 0 && timeout_timer2 == 0) {
                config_temperature_conv_fl = 1;
                for (uint8_t i = 0; i < 3; i++) {
                    if (ds18b20_read_temp_matchrom((unsigned char *) &tbuf[i * 8], &_t) != 0) {
                        _temps[i] = _t;
                    }
                }
            }

            handle_keys_next_prev(&current_device, 0, num_devices - 1);

            if (key2_press != 0) {
                if (_t_num[current_device]++ == 3) {
                    _t_num[current_device] = 0;
                }
                timeout_timer1 = 5;
            }

            // print temp for current device (if converted)
            if (config_temperature_conv_fl != 0) {
                temps[TEMP_CONFIG] = _temps[current_device];
                len = print_temp(TEMP_CONFIG, ALIGN_LEFT);
            } else {
                len = 0;
            }
            add_leading_symbols(buf, ' ', len, 16);

            // print t.sensor + number/total
            strcpy2(buf, (char *) &temp_sensor_string, 0);
            buf[9]= '1' + current_device;
            buf[10] = '/';
            buf[11] = '0' + num_devices;

            LCD_CMD(0x80);
            _print16(16, ALIGN_NONE);

            // convert id to hex string
            llptrtohex((unsigned char*) &tbuf[current_device * 8], (unsigned char*) buf);

            // print sensor string (in/out/eng)
            len = strcpy2(&buf[12], (char *) &temp_sensors_array, _t_num[current_device] + 1);
            add_leading_symbols(&buf[12], ' ', len, 4);

        }

        LCD_CMD(0xC0);
        _print16(16, ALIGN_NONE);
        
        if (request_screen((char *) &reset_string) != 0) {
            _t_num[0] = 1; _t_num[1] = 2; _t_num[2] = 3;
            temps[0] = DS18B20_TEMP_NONE; temps[1] = DS18B20_TEMP_NONE; temps[2] = DS18B20_TEMP_NONE;
            _memset(&tbuf, 0xFF, 8 * 3);
            timeout_timer1 = 0;
        }

        clear_keys_state();

        while (screen_refresh == 0 && timeout_timer1 != 0 && (timeout_timer2 != 0 || config_temperature_conv_fl != 0));
    }

    for (uint8_t i = 0; i < 3; i++) {
        if (_t_num[i] != 0) {
            // save ds18b20 serial number to eeprom
            HW_write_eeprom_block((unsigned char *) &tbuf[i * 8], EEPROM_DS18B20_ADDRESS + (_t_num[i] - 1) * 8, 8);
        }
    }
}
#elif defined(DS18B20_CONFIG)

// show temp of connected sensor
#if defined(DS18B20_CONFIG_SHOW_TEMP)
__bit config_temperature_conv_fl;
#endif

/**
 * simple temp sensors' configuration
 * sensors connected one by one
 */
void config_screen_temp_sensors() {
    char tbuf[8];
    
    _memset(tbuf, 0xFF, 8);
    ds18b20_read_rom((unsigned char*) tbuf);

#if defined(DS18B20_CONFIG_SHOW_TEMP)
    ds18b20_start_conversion(); config_temperature_conv_fl = 0; timeout_timer2 = 100;
#endif

    unsigned char t_num = 0;
    
    timeout_timer1 = 5;
    while (timeout_timer1 != 0) {
        screen_refresh = 0;

#if defined(DS18B20_CONFIG_SHOW_TEMP)
        if (config_temperature_conv_fl == 0 && timeout_timer2 == 0) {
            config_temperature_conv_fl = 1;
            if (ds18b20_read_temp_matchrom((unsigned char *) &tbuf, &_t) != 0) {
                temps[TEMP_CONFIG] = _t;
            }
        }
#endif

        if (key2_press != 0) {
            t_num++;
            if (t_num >= 4) {
                t_num = 0;
            }
            timeout_timer1 = 5;
        }

#if defined(DS18B20_CONFIG_SHOW_TEMP)        
        if (config_temperature_conv_fl != 0) {
            add_leading_symbols((char *) &buf, ' ', print_temp(TEMP_CONFIG, ALIGN_LEFT), 16);
            
            strcpy2(buf, (char *) config_menu_array, TEMP_SENSOR_INDEX);
            
            LCD_CMD(0x80);
            _print16(16, ALIGN_NONE);
        }
#endif
        llptrtohex((unsigned char*) tbuf, (unsigned char*) buf);
        len = strcpy2(&buf[12], (char *) &temp_sensors_array, t_num + 1);
        add_leading_symbols(&buf[12], ' ', len, 4);
        
        LCD_CMD(0xC0);
        _print16(16, ALIGN_NONE);
        
        wait_refresh_timeout();
    }
    
    if (t_num != 0) {
        // save ds18b20 serial number to eeprom
        HW_write_eeprom_block((unsigned char *) tbuf, EEPROM_DS18B20_ADDRESS + (t_num - 1) * 8, 8);
    }
}
#endif

#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT

#define MAX_SUB_ITEM 4

void config_screen_service_counters() {
    static uint8_t c_sub_item = 0;

    handle_keys_next_prev(&c_sub_item, 0, MAX_SUB_ITEM);
    
    LCD_CMD(0xC0);
    buf[0] = '1' + c_sub_item;
    buf[1] = '.';
    _print16(2 + strcpy2(&buf[2], (char *) &service_counters_array, c_sub_item + 1), ALIGN_LEFT);

    if (key2_press != 0) {
        clear_keys_state();

        LCD_CMD(0x80);
        _print16(strcpy2(buf, (char *) &service_counters_array, c_sub_item + 1), ALIGN_LEFT);
        LCD_CMD(0xC0);
        _print16(0, ALIGN_NONE);

        if (c_sub_item == 0) {
            services.mh.limit = (unsigned short) edit_value_long(services.mh.limit, 1999L);
        } else {
            services.srv[c_sub_item - 1].limit = edit_value_char(services.srv[c_sub_item - 1].limit, CHAREDIT_MODE_10000KM, 0, 60);
        }

        timeout_timer1 = 5;
    }
    
}
#endif

void config_screen_ua_const() {
    handle_keys_up_down(&config.vcc_const, VOLTAGE_ADJUST_CONST_MIN, VOLTAGE_ADJUST_CONST_MAX);

    LCD_CMD(0xC4);
    print_voltage((uint16_t *) &adc_voltage.current, POS_NONE, ALIGN_RIGHT);
}

void config_screen_version() {
    if (key1_press || key2_press != 0) {
        timeout_timer1 = 0;
    }
    LCD_CMD(0xC0);
    _print16(strcpy2(buf, (char*) &version_string, 0), ALIGN_LEFT);
}

void config_screen() {

    screen_config_item_t *item = (screen_config_item_t *) &items_service[c_item];
    
    LCD_CMD(0x80);
    _print16(strcpy2(buf, (char *) &config_menu_title_string, 0), ALIGN_LEFT);

    LCD_CMD(0xC0);
    buf[0] = '1' + c_item;
    buf[1] = '.';
    _print16(strcpy2(&buf[2], (char *) config_menu_array, item->title_string_index) + 2, ALIGN_LEFT);

    if (key2_press != 0) {
        key2_press = 0;
        LCD_Clear();
        timeout_timer1 = 5;
        while (timeout_timer1 != 0) {
            screen_refresh = 0;

            LCD_CMD(0x80);
            _print16(strcpy2(buf, (char *) config_menu_array, item->title_string_index), ALIGN_LEFT);

            item->screen();

            wait_refresh_timeout();
        }
        screen_refresh = 1;
    }
}


#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT

unsigned char check_service_counters() {
    unsigned char i;
    unsigned char warn = 0;
    for (i = 0; i < 5; i++) {
        if (i == 0) {
            if (services.mh.limit != 0 && get_mh() >= services.mh.limit) {
                warn |= (1 << i);
            }
        } else {
            srv_t* srv = &services.srv[i - 1];
            if (srv->limit != 0 && srv->counter >= (srv->limit * 1000U)) {
                warn |= (1 << i);
            }
        }
    }
    return warn;
}

void print_warning_service_counters(unsigned char warn) {
    unsigned char i;
    for (i = 0; i < 5; i++) {
        if ((warn & 0x01) != 0) {
#ifdef SOUND_SUPPORT
            buzzer_mode_index = BUZZER_WARN;
#endif
            LCD_CMD(0x80);
            _print16(strcpy2(buf, (char*) &warning_string, 0), ALIGN_CENTER);

            LCD_CMD(0xC0);
            _print16(strcpy2(buf, (char*) &service_counters_array, i + 1), ALIGN_CENTER);

            timeout_timer1 = 5;
            while (timeout_timer1 != 0 && NO_KEY_PRESSED)
                ;
            clear_keys_state();
        }
        warn >>= 1;
    }
}
#endif


void read_eeprom() {

#if defined(PROGMEM_EEPROM)
    // check eeprom special mark and save default eeprom content if mark not exists
    unsigned char tbuf[8];
    // checking key ok pressed for 1 sec for overwriting eeprom with defaults
    uint8_t c = 25;
    while (KEY_OK_PRESSED) {
        if (c > 0) {
            c--;
            delay_ms(40);
        }
    }
    if (c != 0) {
        HW_read_eeprom_block((unsigned char*) &tbuf, sizeof(eedata) - 8, 8);
    }       
    if (c == 0 || memcmp_P((unsigned char*) &tbuf, &eedata[sizeof(eedata) - 8], 8) != 0) {
        uint8_t c;
        for (c = 0; c < sizeof(eedata); c += 8) {
            memcpy_P(&tbuf, &eedata[c], 8);
            HW_write_eeprom_block((unsigned char*) &tbuf, c, 8);
        }
    }
#endif
    
    HW_read_eeprom_block((unsigned char*) &config, EEPROM_CONFIG_ADDRESS, sizeof(config_t));
    
    HW_read_eeprom_block((unsigned char*) &trips, EEPROM_TRIPS_ADDRESS, sizeof(trips_t));

#ifdef SERVICE_COUNTERS_SUPPORT
    HW_read_eeprom_block((unsigned char*) &services, EEPROM_SERVICES_ADDRESS, sizeof(services_t));
#endif
    
#ifdef CONTINUOUS_DATA_SUPPORT
    HW_read_eeprom_block((unsigned char*) &cd, EEPROM_CONTINUOUS_DATA_ADDRESS, sizeof (continuous_data_t));
#endif
}

void save_eeprom_config() {
    HW_write_eeprom_block((unsigned char*) &config, EEPROM_CONFIG_ADDRESS, sizeof (config_t));
}

void save_eeprom_trips() {
    HW_write_eeprom_block((unsigned char*) &trips, EEPROM_TRIPS_ADDRESS, sizeof (trips_t));
}

void save_eeprom() {
    save_eeprom_config();
    
    save_eeprom_trips();

#ifdef SERVICE_COUNTERS_SUPPORT
    HW_write_eeprom_block((unsigned char*) &services, EEPROM_SERVICES_ADDRESS, sizeof (services_t));
#endif

#ifdef CONTINUOUS_DATA_SUPPORT
    HW_write_eeprom_block((unsigned char*) &cd, EEPROM_CONTINUOUS_DATA_ADDRESS, sizeof (continuous_data_t));
#endif
}

void read_ds_time() {
    if (timeout_ds_read == 0) {
        timeout_ds_read = TIMEOUT_DS_READ;
        get_ds_time(&time);
    }
}

void fill_trip_time(trip_time_t *trip_time) {
    trip_time->minute = time.minute;
    trip_time->hour = time.hour;
    trip_time->day = time.day;
    trip_time->month = time.month;
    trip_time->year = time.year;
}

uint16_t get_yday(uint8_t month, uint8_t day) {
    const uint16_t ydayArray[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
    return ydayArray[bcd8_to_bin(month) - 1] + bcd8_to_bin(day);
}

uint8_t check_tripC_time() {
    // clear trip C if diff between dates is more than TRIPC_PAUSE_MINUTES minutes
    int8_t diff;

#ifndef SIMPLE_TRIPC_TIME_CHECK    
    diff = bcd_subtract(time.year, trips.tripC_time.year);
    if (diff < 0) return 0; else if (diff > 1) return 1;

    uint16_t yday = get_yday(time.month, time.day);
    uint16_t yday_c = get_yday(trips.tripC_time.month, trips.tripC_time.day);
    
    int16_t diff_day = (int16_t) ((diff == 0 ? 0 : 365) + yday - yday_c);
    if (diff_day < 0) return 0; else if (diff_day > 1) return 1;

    diff = (int8_t) diff_day;
#else    
    diff = bcd_subtract(time.day,trips.tripC_time.day);
    if (diff < 0) return 0; else if (diff > 1) return 1;
#endif
    
    if (config.settings.daily_tripc == 0) {
        diff = (diff == 0 ? 0 : 24) + bcd_subtract(time.hour, trips.tripC_time.hour);
        if (diff < 0) return 0;

        if ((int16_t) (60 * diff) + bcd_subtract(time.minute, trips.tripC_time.minute) > TRIPC_PAUSE_MINUTES) return 1;
    } else {
        return (uint8_t) diff;
    }
    
    return 0;
}

uint8_t check_tripB_month() {
    if (config.settings.monthly_tripb != 0) {
        if (trips.tripB_month != 0 && trips.tripB_month != time.month) {
            return 1;
        }
    }
    return 0;
}

void set_consts() {
    // default const
    fuel1_const = 65;
    fuel2_const = 28 * 2;                   // immediate fuel consumption const (l/h)
    odo_con4 = config.odo_const * 2 / 13;   // immediate fuel consumption const (l/100km)
    // const for dual injection
    if (config.settings.par_injection != 0) {
        fuel1_const <<= 1; // * 2
        fuel2_const >>= 1; // / 2
        odo_con4 >>= 1;    // / 2
    }

    main_param = config.selected_param.main_param;
    misc_param = config.selected_param.misc_param;
#ifdef SERVICE_COUNTERS_SUPPORT
    service_param = config.selected_param.service_param;
#endif

#ifndef MIN_SPEED_CONFIG
    config.selected_param.min_speed = MIN_SPEED_DEFAULT;
#endif

#ifndef EXTENDED_ACCELERATION_MEASUREMENT
    accel_meas_upper_const = (unsigned short) (speed_const(100) / config.odo_const);
#endif

}

#if defined(TEMPERATURE_SUPPORT)

//#define TEMP_CRC_ERROR

#if defined(DS18B20_TEMP)
#if defined(TEMP_CRC_ERROR)
uint8_t t_error[3] = {0, 0, 0};
#endif
#endif

void handle_temp() {
    if (temperature_conv_fl != 0) {
        temperature_conv_fl = 0;
        // read temperature for ds18b20/ds3231
        timeout_temperature = TIMEOUT_TEMPERATURE;
#if defined(DS18B20_TEMP)
        unsigned char _temps_ee_addr = EEPROM_DS18B20_ADDRESS;
        for (unsigned char i = 0; i < 3; i++) {
            HW_read_eeprom_block((unsigned char *) &buf, _temps_ee_addr, 8);
            if (ds18b20_read_temp_matchrom((unsigned char *) &buf, &_t) == 0) {
#if defined(TEMP_CRC_ERROR)
                if (t_error[i] >= 5) { // max sequential crc errors
                    temps[i] = DS18B20_TEMP_NONE;
                } else {
                    t_error[i]++;
                }
#else
                temps[i] = DS18B20_TEMP_NONE;
#endif                
            } else {
                temps[i] = _t;
#if defined(TEMP_CRC_ERROR)
                t_error[i] = 0;
#endif
            }
            _temps_ee_addr += 8;
        }
#endif
#if defined(DS3231_TEMP)
#if defined(DS18B20_TEMP)
        if (config.settings.ds3231_temp)
#endif
        {
          get_ds_temp(&temps[TEMP_IN]);
        }
#endif        
    } else {
        // start conversion for ds18b20/ds3231
        temperature_conv_fl = 1;
        timeout_temperature = 1;
#if defined(DS18B20_TEMP)
        ds18b20_start_conversion();
#endif
#if defined(DS3231_TEMP)
#if defined(DS18B20_TEMP)
        if (config.settings.ds3231_temp)
#endif
        {
          start_ds_temp();
        }
#endif        
    }
}

#endif

#ifdef CONTINUOUS_DATA_SUPPORT

void cd_init() {
    if (cd.filter < CD_FILTER_VALUE_MIN || cd.filter > CD_FILTER_VALUE_MAX) {
        continuous_data_fl = 0;
        cd.time = 0;
        cd.time_threshold = CD_TIME_THRESHOLD_INIT;
        cd.filter = cd.f_kmh.filter = cd.f_fuel.filter = CD_FILTER_VALUE_MIN;
        cd.f_kmh.tmp = cd.f_fuel.tmp = 0;
        cd_fuel = 0;
        cd_kmh = 0;
    } else if (cd.filter > CD_FILTER_VALUE_MIN) {
        cd_fuel = (uint16_t) (cd.f_fuel.tmp >> cd.f_fuel.filter);
        cd_kmh = (uint16_t) (cd.f_kmh.tmp >> cd.f_kmh.filter);
        continuous_data_fl = 1;
    }
}

void cd_increment_filter() {
    if (cd.filter >= CD_FILTER_VALUE_MIN && cd.filter < CD_FILTER_VALUE_MAX) {
        continuous_data_fl = 1;
        cd.filter++;
        cd.f_kmh.filter = cd.f_fuel.filter = cd.filter;
        cd.f_kmh.tmp <<= 1;
        cd.f_fuel.tmp <<= 1;
        cd.time = 0;
        cd.time_threshold <<= 1; 
    }
}
#endif

uint16_t get_speed(uint16_t kmh) {
    return (unsigned short) ((36000UL * (unsigned long) kmh) / (unsigned long) config.odo_const);
}

void handle_misc_values() {
    speed = get_speed((uint16_t) kmh);
    drive_min_speed_fl = speed >= config.selected_param.min_speed * 10;

    if (trips.tripC_max_speed < speed) {
        trips.tripC_max_speed = speed;
    }

    if (drive_fl != 0 && speed == 0) {
        drive_fl = 0;
    }

    if (trips.tripA.odo > MAX_ODO_TRIPA) {
        clear_trip(true, &trips.tripA);
    }

    if (trips.tripB.odo > MAX_ODO_TRIPB) {
        clear_trip(true, &trips.tripB);
    }

#ifdef CONTINUOUS_DATA_SUPPORT
    cd_speed = get_speed((uint16_t) cd_kmh);
    drive_min_cd_speed_fl = cd_speed >= config.selected_param.min_speed * 10;
#endif    
}

void power_on() {
    HW_Init();

    read_eeprom();

    LCD_Init();

    set_consts();

    read_ds_time();

#if defined(JOURNAL_SUPPORT)
    journal_check_eeprom();
#endif

    if (time.flags.is_valid) {
        if (check_tripC_time() != 0) {
            // clear tripC
            clear_trip(true, &trips.tripC);
            trips.tripC_max_speed = 0;
        }

        if (check_tripB_month() != 0) {
            clear_trip(true, &trips.tripB);
        }
    }

#if defined(CONTINUOUS_DATA_SUPPORT)
    cd_init();
#endif
    
}

void power_off() {
    LCD_Clear();
    // save and shutdown;
    disable_interrupts();

    if (save_tripc_time_fl != 0) {
        // save current time for tripC
        read_ds_time();
        fill_trip_time(&trips.tripC_time);
    }

    // save tripB month
    if (config.settings.monthly_tripb != 0) {
        if (save_tripc_time_fl != 0 || trips.tripB_month == 0) {
            trips.tripB_month = time.month;
        }
    } else {
        trips.tripB_month = 0;
    }


    config.selected_param.main_param = main_param;
    config.selected_param.misc_param = misc_param;
#ifdef SERVICE_COUNTERS_SUPPORT
    config.selected_param.service_param = service_param;
#endif

    save_eeprom();

    PWR_OFF;
    while (1);
}

void main() {
    static __bit config_mode;
    uint8_t prev_main_item = 0, prev_config_item = 0;
    uint8_t max_item = 0;

    power_on();
    
    start_main_timer();

    enable_interrupts();
    
#ifdef TEMPERATURE_SUPPORT
    timeout_temperature = 0;
#endif        

#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
    if (config.settings.service_alarm) {
        unsigned char warn = check_service_counters();
        print_warning_service_counters(warn);
    }
#endif
    
    clear_keys_state();
    
    // wait first adc conversion
    while (adc_voltage.current == 0 && screen_refresh == 0)
        ;
    
    
    while (1) {
        screen_refresh = 0;

        // check power
        if (shutdown_fl != 0) {
            power_off();
        }

        if (key2_longpress != 0) {
            // long keypress for service key - switch service mode and main mode
            if (config_mode == 0 && motor_fl == 0 && drive_fl == 0 && c_item == 0) {
                prev_main_item  = c_item;
                c_item = prev_config_item;
                config_mode = 1;
                //LCD_Clear();
                clear_keys_state();
            } else if (config_mode != 0) {
                prev_config_item = c_item;
                c_item = prev_main_item;
                config_mode = 0;
                // save config
                save_eeprom_config();
                // set consts
                set_consts();
                //LCD_Clear();
                clear_keys_state();
            }
        }
        
        if (config_mode == 0) {
            max_item = sizeof (items_main) / sizeof (screen_item_t) - 1;
#ifdef TEMPERATURE_SUPPORT
            if (timeout_temperature == 0) {
                handle_temp();
            }
#endif            
            handle_misc_values();
        } else {
            max_item = sizeof (items_service) / sizeof (screen_item_t) - 1;
        }
        
        // show next/prev screen
        c_item_prev = c_item;

        handle_keys_next_prev(&c_item, 0, max_item);

        if (c_item_prev != c_item) {
            tmp_param = 0;
            clear_keys_state();
        }
        
        if (config_mode != 0) {
            config_screen();
        } else {
            do {
                if (drive_min_speed_fl != 0 && c_item > (drive_mode_screen_max - 1)) {
                    c_item = 0;
                    //LCD_Clear();
                }
                item_skip = 0;
                items_main[c_item].screen();
                if (item_skip != 0) {
#ifdef KEY3_SUPPORT
                    if (c_item < c_item_prev) {
                        if (c_item == 0) {
                            c_item = (sizeof(items_main) / sizeof(items_main[0])) - 1;
                        } else {
                            c_item--;
                        }
                    } else
#endif
                    {
                        if (c_item == (sizeof(items_main) / sizeof(items_main[0])) - 1) {
                            c_item = 0;
                        } else {
                            c_item++;
                        }
                    }
                }
            } while (item_skip != 0);
        }
        
        while (screen_refresh == 0);
    }
}
