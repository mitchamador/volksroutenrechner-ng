#include "core.h"
#include "ds3231.h"
#include "ds18b20.h"
#include "eeprom.h"
#include "lcd.h"
#include "journal.h"
#include "ui_16x2_legacy.h"
#include <string.h>

// __near forces xc8 to use bitbssCOMMON section
__near volatile flag_t screen_refresh;

// misc flags
volatile flag_t odom_fl, drive_fl, motor_fl, fuel_fl, taho_fl, taho_measure_fl, acc_power_off_fl, shutdown_fl;

volatile flag_t save_tripc_time_fl = 0;

config_t config;
trips_t trips;
services_t services;

__bank2 print_trip_t ptrip;

__bank2 live_data_t data;

volatile uint24_t taho_tmp, taho, taho_timer_ticks;
volatile uint8_t taho_timer_ofl;

// uint16_t - max ~104 ms for pic16f targets (* 2.5 for atmega)
volatile uint16_t fuel_duration_tmp, fuel_duration;

// acceleration measurement flags and variables
volatile flag_t accel_meas_fl, accel_meas_ok_fl, accel_meas_process_fl, accel_meas_timer_fl, accel_meas_drive_fl;
#ifdef EXTENDED_ACCELERATION_MEASUREMENT
volatile uint16_t accel_meas_lower_const;
#endif
volatile uint16_t accel_meas_upper_const, accel_meas_timer, accel_meas_speed;
volatile uint16_t speed_timer_prev, speed_timer_ticks;
volatile uint8_t speed_timer_ofl;

volatile uint16_t kmh_tmp, fuel_tmp;
volatile uint16_t kmh, fuel;

#if defined(LCD_1602) && defined(LCD_1602_I2C)
volatile flag_t use_lcd_1602_i2c_fl;
#endif

#ifdef ADC_BUTTONS_SUPPORT

#define ADC_KEY_OK 2
#define ADC_KEY_NEXT 1
#define ADC_KEY_PREV 3

volatile uint8_t adc_key;
volatile flag_t use_adc_buttons_fl;

#define _key1_pressed() ((use_adc_buttons_fl == 0 && HW_key1_pressed()) || (use_adc_buttons_fl != 0 && adc_key == ADC_KEY_NEXT))
#define _key2_pressed() ((use_adc_buttons_fl == 0 && HW_key2_pressed()) || (use_adc_buttons_fl != 0 && adc_key == ADC_KEY_OK))
#define _key3_pressed() ((use_adc_buttons_fl == 0 && HW_key3_pressed()) || (use_adc_buttons_fl != 0 && adc_key == ADC_KEY_PREV))

#else
#define _key1_pressed() HW_key1_pressed()
#define _key2_pressed() HW_key2_pressed()
#define _key3_pressed() HW_key3_pressed()
#endif

// key variables and flags
volatile uint8_t key_repeat_counter;
volatile flag_t key1_press, key2_press, key1_longpress, key2_longpress, key_pressed, key_longpressed;

#if defined(KEY3_SUPPORT)
volatile flag_t key3_press, key3_longpress;
#endif

#if defined(ENCODER_SUPPORT)
volatile flag_t key2_doubleclick;
volatile flag_t use_encoder_fl;
#endif

volatile uint8_t acc_power_off_counter;

// timeout1 (resolution - 1 s)
volatile uint8_t timeout_timer1;
// timeout2 (resolution - 0.01 s)
volatile uint8_t timeout_timer2;

volatile uint16_t main_timer;
#ifndef taho_timer
volatile uint16_t taho_timer;
#endif
#ifndef speed_timer
volatile uint16_t speed_timer;
#endif

volatile uint8_t timeout_ds_read = 0;

#ifdef SOUND_SUPPORT
volatile int8_t buzzer_mode_index = BUZZER_NONE;
volatile flag_t buzzer_fl;
#endif

#ifdef TEMPERATURE_SUPPORT
volatile uint8_t timeout_temperature;
#endif

#ifdef MIN_MAX_VOLTAGES_SUPPORT
volatile adc_voltage_t adc_voltage = {0, HW_ADC_MAX, HW_ADC_MIN};
#else
volatile adc_voltage_t adc_voltage = {0};
#endif

uint8_t fuel1_const;
//uint8_t fuel2_const;
//uint16_t odo_con4;

uint8_t drive_min_speed;

uint16_t calc_filtered_value(filtered_value_t *, uint16_t);

#ifdef CONTINUOUS_DATA_SUPPORT
continuous_data_t cd;
flag_t continuous_data_fl;
volatile uint16_t cd_kmh, cd_fuel;
void cd_init(void);
void cd_increment_filter(void);
#endif

#if defined(INSTANT_FUEL_AVERAGE_SUPPORT)
filtered_value_t f_fuel = {0, 1};
#endif

#if !defined(SIMPLE_ADC)

uint16_t adc_value;

filtered_value_t f_voltage = {0, 1 | FILTERED_VALUE_FIRST_SAMPLE};

void adc_handler_voltage(filtered_value_t *f);
void adc_handler_buttons(filtered_value_t *f);
void adc_handler_fuel_tank(filtered_value_t *f);

#if defined(ADC_BUTTONS_SUPPORT) || defined(FUEL_TANK_SUPPORT)
uint8_t _adc_ch;

#ifdef ADC_BUTTONS_SUPPORT
filtered_value_t f_buttons = {0, 0};
#endif

#if defined(FUEL_TANK_SUPPORT)
uint8_t adc_fuel_tank_counter;
uint16_t adc_fuel_tank;
filtered_value_t f_fuel_tank = {0, 3 | FILTERED_VALUE_FIRST_SAMPLE};
#endif

adc_item_t adc_items[] = {
    {adc_handler_voltage, &f_voltage, HW_ADC_CHANNEL_POWER_SUPPLY},
#ifdef ADC_BUTTONS_SUPPORT
    {adc_handler_buttons, &f_buttons, HW_ADC_CHANNEL_BUTTONS},
#endif
#ifdef FUEL_TANK_SUPPORT
    {adc_handler_fuel_tank, &f_fuel_tank, HW_ADC_CHANNEL_FUEL_TANK},
#endif
};
#else
adc_item_t adc_item = {adc_handler_voltage, &f_voltage, HW_ADC_CHANNEL_POWER_SUPPLY};
#endif

#endif

// interrupt routines starts

void int_capture_injector_level_change() {

    static uint16_t taho_timer_prev;

    // fuel injector
    if (HW_fuel_active()) {
        if (fuel_fl == 0) {
            // start fuel timer
            HW_start_fuel_timer();
            fuel_fl = motor_fl = save_tripc_time_fl = 1;

            // new taho calculation based on captured value of 0.01s timer
            if (taho_measure_fl == 0) {
                taho_measure_fl = 1;

                taho_timer_ticks = 0;
                taho_timer_prev = taho_timer;
                taho_timer_ofl = 0;
            } else {
                taho_tmp = taho_timer_ticks + taho_timer - taho_timer_prev;
                taho_fl = 1;

                taho_timer_ticks = 0;
                taho_timer_prev = taho_timer;
                taho_timer_ofl = 0;
            }
        }
    } else {
        if (fuel_fl != 0) {
            // stop fuel timer
            HW_stop_fuel_timer();
            fuel_fl = 0;
            // measure fuel duration                
            if (taho_measure_fl != 0) {
                fuel_duration_tmp = (uint16_t) (taho_timer_ticks + taho_timer - taho_timer_prev);
            }
        }
    }
}

__section("text999") void int_taho_timer_overflow() {
    if (taho_measure_fl != 0) {
        if (++taho_timer_ofl == TAHO_OVERFLOW) {
            HW_stop_fuel_timer();
            taho_measure_fl = taho_fl = motor_fl = 0;
            fuel_duration_tmp = taho_tmp = 0;
        } else {
            taho_timer_ticks += HW_TAHO_TIMER_TICKS_PER_PERIOD;
        }
    }
}

void int_capture_speed_level_change() {
    // speed sensor
    if (HW_tx_active()) {
        if (odom_fl == 0) {
            odom_fl = drive_fl = 1;

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
                        accel_meas_timer_fl = accel_meas_process_fl = 0;
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
            accel_meas_fl = accel_meas_drive_fl = 0;
        } else {
            speed_timer_ticks += HW_SPEED_TIMER_TICKS_PER_PERIOD;
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
    static uint8_t refresh_interval_counter = REFRESH_INTERVAL;
    static uint8_t main_interval_counter = MAIN_INTERVAL;

#if defined(ENCODER_SUPPORT)
    static uint8_t key2_click_counter, key2_clicks;
    static flag_t key_multiclicked;
#endif    
    static uint8_t key1_counter = 0, key2_counter = 0;
#ifdef KEY3_SUPPORT
    static uint8_t key3_counter = 0;
#endif

    if (key_repeat_counter == 0) {
        if (use_encoder() == 0 && _key1_pressed()) // key pressed
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

        if (_key2_pressed()) // key pressed
        {
#if defined(ENCODER_SUPPORT)
            if (use_encoder() != 0 && key2_counter == 0) {
                key2_click_counter = MULTICLICK;
            }
            if (key2_click_counter != 0) {
                key2_click_counter--;
            }
#endif
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
#if defined(ENCODER_SUPPORT)
            if (use_encoder() != 0) {
                if (key2_counter >= LONGKEY) {
                    key2_click_counter = 0;
                    key2_clicks = 0;
                } else {
                    if (key2_counter > DEBOUNCE && key2_counter <= SHORTKEY) {
                        key2_clicks++;
#if defined(SOUND_SUPPORT)
                        buzzer_mode_index = BUZZER_KEY;
#endif                    
                    }
                    if (key2_click_counter == 0 || key2_clicks == 2) {
                        if (key2_clicks == 1) {
                            key2_press = 1;
                            key_multiclicked = 1;
                        } else if (key2_clicks == 2) {
                            key2_doubleclick = 1;
                            key_multiclicked = 1;
                        }
                        key2_click_counter = 0;
                        key2_clicks = 0;
                    } else {
                        key2_click_counter--;
                    }
                }
            }
            else
#endif
            {
                if (key2_counter > DEBOUNCE && key2_counter <= SHORTKEY) {
                    // key press
                    key2_press = 1;
                    key_pressed = 1;
                }
            }
            key2_counter = 0;
        }

#if defined(KEY3_SUPPORT)
        if (use_encoder() == 0 && _key3_pressed()) // key pressed
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

#if defined(ENCODER_SUPPORT)
    if (key_pressed != 0 || key_longpressed != 0 || key_multiclicked != 0) {
#else
    if (key_pressed != 0 || key_longpressed != 0) {
#endif
#ifdef SOUND_SUPPORT
        if (key_pressed != 0) {
            buzzer_mode_index = BUZZER_KEY;
        } else if (key_longpressed != 0) {
            buzzer_mode_index = BUZZER_LONGKEY;
        }
#endif
        key_pressed = 0;
        key_longpressed = 0;
#if defined(ENCODER_SUPPORT)
        if (key_multiclicked != 0) {
            key_multiclicked = 0;
        } else {
            key_repeat_counter = KEY_REPEAT_PAUSE;
        }
#else
        key_repeat_counter = KEY_REPEAT_PAUSE;
#endif
        screen_refresh = 1;
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

        // increment time counters
        if (motor_fl != 0 || drive_fl != 0) {
            services.mh.time++;
            trips.tripA.time++;
            trips.tripB.time++;
            trips.tripC.time++;
        }

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
    
    if (--refresh_interval_counter == 0) {
        refresh_interval_counter = REFRESH_INTERVAL;

        // screen refresh_flag
        screen_refresh = 1;

#ifdef CONTINUOUS_DATA_SUPPORT
        if (motor_fl != 0 || drive_fl != 0) {
            if (cd.filter < CD_FILTER_VALUE_MAX) {
                if (cd.time < cd.time_threshold) {
                    cd.time++;
                } else {
                    cd_increment_filter();
                }
            }
            cd_fuel = calc_filtered_value(&cd.f_fuel, fuel_tmp);
            cd_kmh = calc_filtered_value(&cd.f_kmh, kmh_tmp);
        }
#endif

        // copy temp interval variables to main (with filtering if enabled and supported)
#if defined(INSTANT_FUEL_AVERAGE_SUPPORT)
        if (config.settings.instant_fuel_avg != 0) {
            fuel = calc_filtered_value(&f_fuel, fuel_tmp);
        } else
#endif
        {
            fuel = fuel_tmp;
        }
        if (motor_fl == 0) {
            fuel = 0;
        }
        fuel_tmp = 0;

        fuel_duration = fuel_duration_tmp;
        taho = taho_tmp;

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
        
    }

    if (timeout_timer2 > 0) {
        timeout_timer2--;
    }

    if (accel_meas_timer_fl != 0) {
        accel_meas_timer++;
    }

#ifdef SOUND_SUPPORT

    static flag_t buzzer_init_fl;
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
        }
        buzzer_mode_index = BUZZER_NONE;
    }

    static flag_t buzzer_snd_fl, buzzer_repeat_fl;
    static uint8_t buzzer_counter_r;
    static uint8_t buzzer_counter;

    if (buzzer_fl != 0) {
        if (buzzer_init_fl == 0) {
            buzzer_init_fl = 1;
            buzzer_repeat_fl = 1;
            buzzer_counter_r = buzzer_mode_counter;
        }

        if (buzzer_repeat_fl != 0) {
            buzzer_repeat_fl = 0;
            if (buzzer_counter_r == 0) {
                buzzer_fl = 0;
                buzzer_init_fl = 0;
            } else {
                buzzer_counter_r--;
                buzzer_snd_fl = 1;
                buzzer_counter = buzzer_mode_sound;
            }
        }

        if (buzzer_snd_fl != 0) {
            if (buzzer_counter == 0) {
                buzzer_snd_fl = 0;
                buzzer_counter = buzzer_mode_pause;
            }
            HW_snd_on();
        }
        if (buzzer_snd_fl == 0) {
            if (buzzer_counter == 0) {
                buzzer_repeat_fl = 1;
            }
            HW_snd_off();
        }
        buzzer_counter--;
    }
#endif
}

void int_adc_finish() {
#if defined(SIMPLE_ADC)

    adc_voltage.current = HW_adc_read();

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
    adc_value = HW_adc_read();
#if defined(ADC_BUTTONS_SUPPORT) || defined(FUEL_TANK_SUPPORT)        
    adc_item_t* h = &adc_items[_adc_ch];

    if (++_adc_ch >= sizeof (adc_items) / sizeof (adc_item_t)) {
        _adc_ch = 0;
    }

    // set next channel
    HW_adc_set_channel(adc_items[_adc_ch].channel);

    h->handle(h->f);

    if (_adc_ch != 0) {
        // start next adc channel (first channel is started from auto trigger)
        HW_adc_start();
    }
#else
    adc_item.handle(adc_item.f);
#endif

#endif
}

#ifdef ENCODER_SUPPORT
// A valid CW or CCW move returns 1, invalid returns 0.

void int_change_encoder_level() {
    if (use_encoder() == 0) return;

    static int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    static uint8_t prevNextCode = 0;
    static uint8_t store = 0;

    prevNextCode <<= 2;
    if (HW_encoder_get_data()) prevNextCode |= 0x02;
    if (HW_encoder_get_clk()) prevNextCode |= 0x01;
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

#ifdef ADC_BUTTONS_SUPPORT

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

void handle_keys_up_down(uint8_t *v, uint8_t min_value, uint8_t max_value, uint8_t timeout) {
    uint8_t _v = *v;
    if (key1_press != 0) {
        if (_v++ == max_value) {
            _v = min_value;
        }
        timeout_timer1 = timeout;
    }
#if defined(ENCODER_SUPPORT)
    if (use_encoder() != 0 && key2_press != 0) {
        timeout_timer1 = 0;
    }
    if ((use_encoder() == 0 && (key2_press != 0 || key3_press != 0)) || (use_encoder() != 0 && key3_press != 0)) {
#elif !defined(KEY3_SUPPORT)
    if (key2_press != 0) {
#else
    if (key2_press != 0 || key3_press != 0) {
#endif
        if (_v-- == min_value) {
            _v = max_value;
        }
        timeout_timer1 = timeout;
    }
    *v = _v;
}

void handle_keys_next_prev(uint8_t *v, uint8_t min_value, uint8_t max_value, uint8_t timeout) {
    uint8_t _v = *v;
    // change cursor to next position
    if (key1_press != 0) {
        if (_v++ == max_value) {
            _v = min_value;
        }
        timeout_timer1 = timeout;
    }

#if defined(KEY3_SUPPORT)
    // change cursor to prev position
    if (key3_press != 0) {
        if (_v-- == min_value) {
            _v = max_value;
        }
        timeout_timer1 = timeout;
    }
#endif
    *v = _v;
}

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

void read_ds_time() {
    if (timeout_ds_read == 0) {
        timeout_ds_read = TIMEOUT_DS_READ;
        DS3231_time_read(&time);
    }
}

void fill_trip_time(trip_time_t *trip_time) {
    trip_time->minute = time.minute;
    trip_time->hour = time.hour;
    trip_time->day = time.day;
    trip_time->month = time.month;
    trip_time->year = time.year;
}

uint16_t round_div(uint16_t dividend, uint16_t divisor) {
    return (dividend + (divisor / 2)) / divisor;
}

void fill_print_trip(print_trip_t* pt, trip_t* t) {
    uint16_t int_part = t->odo * 10;
    uint16_t frac_part = (uint16_t) (t->odo_temp * 10UL / config.odo_const);
    pt->odo = int_part + frac_part;

    pt->fuel = round_div(t->fuel, 10U);
    pt->time = (uint16_t) (t->time / 60U);

    if (t->time > 0) {
        pt->average_speed = round_div((uint16_t) ((uint32_t) ((t->odo * 360000UL) + (t->odo_temp * 360000UL / config.odo_const)) / t->time), 10);
    } else {
        pt->average_speed = 0;
    }

    if (pt->fuel < AVERAGE_MIN_FUEL || pt->odo < AVERAGE_MIN_DIST) {
        pt->average_fuel = 0;
    } else {
        pt->average_fuel = round_div((uint16_t) (t->fuel * 1000UL / pt->odo), 10);
    }
}
uint16_t get_voltage_value(uint16_t *adc_voltage) {
    return (uint16_t) (*adc_voltage << 5) / (uint8_t) (VOLTAGE_ADJUST_CONST_MAX - (config.vcc_const - VOLTAGE_ADJUST_CONST_MIN));
}

uint16_t get_speed(uint16_t kmh) {
    // instant speed (0,1*km/h)
    return (uint16_t) (((uint32_t) (36000 / REFRESH_PERIOD) * (uint32_t) kmh) / (uint32_t) config.odo_const);
}

uint16_t get_instant_fuel(uint16_t fuel, uint16_t kmh, uint8_t drive_fl) {
    // fuel / (fuel_const * fuel1_const) (fixed point 16.16)
    uint32_t t = (uint32_t) (((uint32_t) fuel << 16) / (uint16_t) (fuel1_const * config.fuel_const));
    
    if (drive_fl == 0) {
        // instant fuel (0,01*l/h) = ((3600 / main_interval_period) * fuel) / (fuel_const * fuel1_const) = (3600 / main_interval_period) * t
        t = round_div((uint16_t) ((uint32_t) ((uint32_t) (3600 / REFRESH_PERIOD) * t) >> 16), 10);
    } else {
        // instant fuel (0,01*l/100km) = (100 * odo_const * fuel) / (fuel_const * fuel1_const * kmh) = (100 * odo_const / kmh) * t
        t = round_div((uint16_t) ((uint32_t) (((100UL * config.odo_const) / kmh) * t) >> 16), 10);
    }
    return (uint16_t) t;
}

void fill_live_data() {
    
    data.speed = get_speed((uint16_t) kmh);
    if (data.speed >= drive_min_speed && trips.tripC_max_speed < data.speed) {
        trips.tripC_max_speed = data.speed;
    }

    data.fuel_instant = get_instant_fuel(fuel, kmh, data.speed >= drive_min_speed);

    if (taho_fl == 0) {
        data.taho_rpm = 0;
    } else {
        data.taho_rpm = (unsigned short) (((config.settings.par_injection != 0 ? (TAHO_CONST) : (TAHO_CONST*2)) / taho));
#ifdef TAHO_ROUND
        data.taho_rpm = round_div(data.taho_rpm, TAHO_ROUND) * TAHO_ROUND;        
#endif
    }

    data.fuel_duration_ms = (uint16_t) (fuel_duration * (1000 / 250) / (HW_TAHO_TIMER_TICKS_PER_PERIOD / 250));

    if (drive_fl != 0 && data.speed == 0) {
        drive_fl = 0;
    }

#ifdef CONTINUOUS_DATA_SUPPORT
    data.cd_speed = get_speed((uint16_t) cd_kmh);
    data.cd_fuel_instant = get_instant_fuel(cd_fuel, cd_kmh, data.cd_speed >= drive_min_speed);
#endif

}

void set_consts() {
    // default const (sequentional injection)
    fuel1_const = 65;
    if (config.settings.par_injection != 0) {
        // bank-to-bank injection
        fuel1_const <<= 1; // * 2
    }
#ifndef MIN_SPEED_CONFIG
    config.min_speed = MIN_SPEED_DEFAULT;
#endif
#ifndef EXTENDED_ACCELERATION_MEASUREMENT
    accel_meas_upper_const = (unsigned short) (speed_const(100) / config.odo_const);
#endif
    drive_min_speed = config.min_speed * 10;
}

//========================================================================================
// Live/derived state moved from main.c (set by fill_misc_values/handle_temp
// below, read by the UI layer for display -- see extern decls in core.h)
//========================================================================================

flag_t drive_min_speed_fl;
#ifdef CONTINUOUS_DATA_SUPPORT
flag_t cd_drive_min_speed_fl;
#endif

#ifdef TEMPERATURE_SUPPORT
flag_t temperature_conv_fl;
uint8_t main_temp_index;

uint16_t _t;
uint16_t temps[4] = {DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE};
#endif

//========================================================================================
// Functions moved from main.c during the main.c -> core.c / ui_16x2_legacy.c
// split. See include/core.h for the reasoning. clear_trip() is NOT here --
// it stays in ui_16x2_legacy.c because it calls request_screen() (an
// interactive confirmation prompt) for the non-forced case; core.c calls it
// via the extern declared in ui_16x2_legacy.h.
//========================================================================================


// ---- moved from main.c: clear_trip stays in ui_16x2_legacy.c (see note there); ----
// ---- moved from main.c: get_mh (guarded, matches original SERVICE_COUNTERS_SUPPORT scope) ----
#ifdef SERVICE_COUNTERS_SUPPORT
uint16_t get_mh() {
    // time based motor hours
    return (uint16_t) (services.mh.time / 3600UL);
}
#endif

// ---- moved from main.c: check_service_counters (guarded, matches original scope) ----
#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
uint8_t check_service_counters() {
    uint8_t i;
    uint8_t warn = 0;
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
#endif

// ---- moved from main.c: read_eeprom/save_eeprom* ----
void read_eeprom() {

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

// ---- moved from main.c: beep/_beep/check_eeprom/preinit_settings, one self-contained #if unit ----
#if defined(PROGMEM_EEPROM) || defined (ENCODER_SUPPORT) || defined(ADC_BUTTONS_SUPPORT) || (defined(LCD_1602) && defined(LCD_1602_I2C))

typedef enum {
    BEEP_OK=1,
    BEEP_UP,
    BEEP_DOWN
} beep_t;

void _beep(uint8_t tone, uint16_t length)
{
  uint16_t i;
  uint8_t r;
  for (i = 0; i < length; i++) {
    if ((i & 0x01) == 0) {
      HW_snd_on();
    } else {
      HW_snd_off();
    }
    for (r = 0; r < tone; r++) {
      HW_delay_us(15);
    }
  }
  HW_snd_off();
}

void beep(uint8_t beep)
{
    switch (beep) {
        case BEEP_OK:
            _beep(15, 255);
            break;
        case BEEP_UP:
            _beep(25, 250);
            _beep(15, 400);
            break;
        case BEEP_DOWN:
            _beep(15, 400);
            _beep(25, 250);
            break;
    }
}

#define SETTINGS_DELAY 150

uint8_t stage_setting = 0;

typedef enum {
    FORCE_SETTING_START=0,
#if defined(ENCODER_SUPPORT)
    FORCE_SETTING_ENCODER_OFF,        // force encoder off
    FORCE_SETTING_ENCODER_ON,         // force encoder on
#endif
#if defined(ADC_BUTTONS_SUPPORT)
    FORCE_SETTING_ADC_BUTTONS_OFF,    // force adc buttons off
    FORCE_SETTING_ADC_BUTTONS_ON,     // force adc buttons on
#endif
#if defined(LCD_1602) && defined(LCD_1602_I2C)
    FORCE_SETTING_LCD_1602_I2C_OFF,    // force lcd 1602 i2c off
    FORCE_SETTING_LCD_1602_I2C_ON,     // force lcd 1602 i2c on
#endif
#if defined(PROGMEM_EEPROM)  
    FORCE_SETTING_EEPROM_REWRITE,     // eeprom rewrite for arduino target
#endif
    FORCE_SETTING_MAX    
} pre_settings_t;

#if defined(PROGMEM_EEPROM)
void check_eeprom(uint8_t c) {
    unsigned char tbuf[8];
    HW_read_eeprom_block((unsigned char*) &tbuf, sizeof(eedata) - 8, 8);
    if (c == FORCE_SETTING_EEPROM_REWRITE || memcmp_P((unsigned char*) &tbuf, &eedata[sizeof(eedata) - 8], 8) != 0) {
        uint8_t c;
        for (c = 0; c < sizeof(eedata); c += 8) {
            memcpy_P(&tbuf, &eedata[c], 8);
            HW_write_eeprom_block((unsigned char*) &tbuf, c, 8);
        }
    }
}
#endif

// force settings overwrite
// press ok button before start
// release after pre_setting_t beeps for change setting
void preinit_settings() {
    while (HW_key2_pressed()) {
        uint8_t keytime = SETTINGS_DELAY;
        while (HW_key2_pressed()) {
            HW_delay_ms(10);
            if (--keytime == 0) {
                keytime = SETTINGS_DELAY;
                if (stage_setting < (FORCE_SETTING_MAX - 1)) {
                    stage_setting++;
#if 0
                    for (uint8_t i = 0; i < stage_setting; i++) {
                        beep(BEEP_OK);
                        HW_delay_ms(150);
                    }
#else
                    beep(BEEP_OK);
#endif
                } else {
                    while (HW_key2_pressed()) {};
                    stage_setting = 0;
                }
            }
        }
        if (!HW_key2_pressed() && stage_setting != 0) {
            beep(BEEP_UP);
            beep(BEEP_UP);
            beep(BEEP_UP);
        }
    }

#if defined(PROGMEM_EEPROM)
    // check eeprom special mark and save default eeprom content if mark not exists
    check_eeprom(stage_setting);
#endif

    read_eeprom();
    
#if defined(ENCODER_SUPPORT)
    if (stage_setting == FORCE_SETTING_ENCODER_OFF) {
        config.settings.encoder = 0;
    } else if (stage_setting == FORCE_SETTING_ENCODER_ON) {
        config.settings.encoder = 1;
    }
    use_encoder_fl = config.settings.encoder;
#endif
#if defined(ADC_BUTTONS_SUPPORT)
    if (stage_setting == FORCE_SETTING_ADC_BUTTONS_OFF) {
        config.settings.adc_buttons = 0;
    } else if (stage_setting == FORCE_SETTING_ADC_BUTTONS_ON) {
        config.settings.adc_buttons = 1;
    }
    use_adc_buttons_fl = config.settings.adc_buttons;
#endif
#if defined(LCD_1602) && defined(LCD_1602_I2C)
    if (stage_setting == FORCE_SETTING_LCD_1602_I2C_OFF) {
        config.settings.lcd_1602_i2c = 0;
    } else if (stage_setting == FORCE_SETTING_LCD_1602_I2C_ON) {
        config.settings.lcd_1602_i2c = 1;
    }
    use_lcd_1602_i2c_fl = config.settings.lcd_1602_i2c;
#endif

}

#endif

// ---- moved from main.c: get_yday/check_tripC_time ----
uint16_t get_yday(uint8_t month, uint8_t day) {
    const uint16_t ydayArray[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
    return ydayArray[bcd8_to_bin(month) - 1] + bcd8_to_bin(day);
}

uint8_t check_tripC_time() {
#ifdef SIMPLE_TRIPC_TIME_CHECK    
    // clear trip C for different day
    return (time.day != trips.tripC_time.day || time.month != trips.tripC_time.month);
#else
    // clear trip C if diff between dates is more than TRIPC_PAUSE_MINUTES minutes
    int8_t diff = 0;
    diff = bcd_subtract(time.year, trips.tripC_time.year);
    if (diff < 0) return 0; else if (diff > 1) return 1;

    uint16_t yday = get_yday(time.month, time.day);
    uint16_t yday_c = get_yday(trips.tripC_time.month, trips.tripC_time.day);
    
    int16_t diff_day = (int16_t) ((diff == 0 ? 0 : 365) + yday - yday_c);
    if (diff_day < 0) return 0; else if (diff_day > 1) return 1;

    diff = (int8_t) diff_day;
    
    if (config.settings.daily_tripc == 0) {
        diff = (diff == 0 ? 0 : 24) + bcd_subtract(time.hour, trips.tripC_time.hour);
        if (diff < 0) return 0;

        if ((int16_t) (60 * diff) + bcd_subtract(time.minute, trips.tripC_time.minute) > TRIPC_PAUSE_MINUTES) return 1;
    } else {
        return (uint8_t) diff;
    }
    
    return 0;
#endif
}

// ---- moved from main.c: check_tripB_month (guarded, matches original scope) ----
#if defined(JOURNAL_SUPPORT)
uint8_t check_tripB_month() {
    if (config.settings.monthly_tripb != 0) {
        if (trips.tripB_month != 0 && trips.tripB_month != time.month) {
            return 1;
        }
    }
    return 0;
}
#endif

// ---- moved from main.c: handle_temp (guarded, matches original scope) ----
#if defined(TEMPERATURE_SUPPORT)

// max sequential crc errors before set temp to DS18B20_TEMP_NONE
#define MAX_CRC_ERROR   5

void handle_temp() {
    unsigned char buf[8];

#if defined(DS18B20_TEMP) && defined(MAX_CRC_ERROR)
    static uint8_t t_error[3] = {0, 0, 0};
#endif

    if (temperature_conv_fl == 0) {
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
          DS3231_temp_start();
        }
#endif        
    } else {
        // read temperature for ds18b20/ds3231
        temperature_conv_fl = 0;
        timeout_temperature = TIMEOUT_TEMPERATURE;
#if defined(DS18B20_TEMP)
        unsigned char _temps_ee_addr = EEPROM_DS18B20_ADDRESS;
        for (uint8_t i = 0; i < 3; i++) {
            HW_read_eeprom_block((unsigned char *) &buf, _temps_ee_addr, 8);
            if (ds18b20_read_temp_matchrom((unsigned char *) &buf, &_t) == 0) {
#if defined(MAX_CRC_ERROR)
                if (t_error[i] >= MAX_CRC_ERROR) {
                    temps[i] = DS18B20_TEMP_NONE;
                } else {
                    t_error[i]++;
                }
#else
                temps[i] = DS18B20_TEMP_NONE;
#endif
            } else {
                temps[i] = _t;
#if defined(MAX_CRC_ERROR)
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
          DS3231_temp_read(&temps[TEMP_IN]);
        }
#endif        
    }
}
#endif

// ---- moved from main.c: set_params/fill_misc_values/power_on/power_off ----

// Restores UI navigation state (params/service_param) from the persisted
// config after an EEPROM read. This is the one place core.c writes directly
// into ui_16x2_legacy.c's exported globals rather than calling a function --
// see the comment on params_t in ui_16x2_legacy.h.
void set_params() {
    // set core constants
    set_consts();

    params.main = config.main_param;
    params.main_add = config.main_add_param;
#ifdef SERVICE_COUNTERS_SUPPORT
    service_param = config.service_param;
#endif
}

void fill_misc_values() {

    if (data.speed >= drive_min_speed) {
        drive_min_speed_fl = 1;
    } else {
        drive_min_speed_fl = 0;
    }

#ifdef CONTINUOUS_DATA_SUPPORT
    if (data.cd_speed >= drive_min_speed) {
        cd_drive_min_speed_fl = 1;
    } else {
        cd_drive_min_speed_fl = 0;
    }
#endif

#ifdef TEMPERATURE_SUPPORT
    main_temp_index = (config.settings.show_inner_temp == 0 ? TEMP_OUT : TEMP_IN) | PRINT_TEMP_PARAM_FRACT | PRINT_TEMP_PARAM_DEG_SIGN;
#endif    

    if (trips.tripA.odo > MAX_ODO_TRIPA) {
        clear_trip(&trips.tripA);
    }

    if (trips.tripB.odo > MAX_ODO_TRIPB) {
        clear_trip(&trips.tripB);
    }

}

void clear_trip(trip_t* trip) {
#ifdef JOURNAL_SUPPORT
    journal_save_trip(trip);
#endif
    _memset(trip, 0, sizeof (trip_t));

    save_eeprom_trips();
}

void power_on() {
    HW_Init();

#if defined(PROGMEM_EEPROM) || defined(ENCODER_SUPPORT) || defined(ADC_BUTTONS_SUPPORT) || (defined(LCD_1602) && defined(LCD_1602_I2C))
    preinit_settings();
#else
    read_eeprom();
#endif

    LCD_init();
#if defined LCD_CONTRAST_CONFIG
    LCD_set_contrast(config.lcd_contrast);
#endif
    set_params();

    read_ds_time();

#if defined(JOURNAL_SUPPORT)
    journal_check_eeprom();
#endif

    if (time.flags.is_valid) {
        if (check_tripC_time() != 0) {
            // clear tripC
            clear_trip(&trips.tripC);
            trips.tripC_max_speed = 0;
        }

#if defined(JOURNAL_SUPPORT)
        if (check_tripB_month() != 0) {
            clear_trip(&trips.tripB);
        }
#endif
    }

#if defined(CONTINUOUS_DATA_SUPPORT)
    cd_init();
#endif
    
}

void power_off() {
    LCD_off();
    // save and shutdown;
    HW_disable_interrupts();

    if (save_tripc_time_fl != 0) {
        // save current time for tripC
        read_ds_time();
        fill_trip_time(&trips.tripC_time);
        trips.tripC_time_dow = time.day_of_week;
    }

    // save tripB month
#if defined(JOURNAL_SUPPORT)
    if (config.settings.monthly_tripb != 0) {
        if (save_tripc_time_fl != 0 || trips.tripB_month == 0) {
            trips.tripB_month = time.month;
        }
    } else
#endif
    {
        trips.tripB_month = 0;
    }

    config.main_param = params.main;
    config.main_add_param = params.main_add;
#ifdef SERVICE_COUNTERS_SUPPORT
    config.service_param = service_param;
#endif

    save_eeprom();

    HW_pwr_off();

    while (1);
}

//========================================================================================
// Called once from main() at startup. Former top half of main() before its
// while(1) loop: power-on/HW/interrupt init.
//========================================================================================
void core_init(void) {
    power_on();

    HW_start_main_timer();

    HW_enable_interrupts();

#ifdef TEMPERATURE_SUPPORT
    // wait 1 sec before first conversion's request
    timeout_temperature = 1;
#endif
}

//========================================================================================
// Called once per main-loop iteration from main(), before ui_16x2_legacy_update().
// Former top of main()'s while(1) body: refresh live/misc data on the timer
// tick, handle shutdown, handle temperature conversion.
//========================================================================================

void core_tick(void) {
    if (screen_refresh != 0) {
        screen_refresh = 0;
        fill_live_data();
        fill_misc_values();
    }

    // check power
    if (shutdown_fl != 0) {
        power_off();
    }

#ifdef TEMPERATURE_SUPPORT
    if (timeout_temperature == 0) {
        handle_temp();
    }
#endif
}
