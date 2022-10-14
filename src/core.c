#include "core.h"
#include "ds3231.h"

// __near forces xc8 to use bitbssCOMMON section
__near volatile flag_t screen_refresh;

// misc flags
volatile flag_t odom_fl, drive_fl, motor_fl, fuel_fl, taho_fl, taho_measure_fl, acc_power_off_fl, shutdown_fl;

volatile flag_t save_tripc_time_fl = 0;

config_t config;
trips_t trips;
services_t services;

volatile uint24_t taho, taho_timer_ticks;
volatile uint16_t taho_timer_prev;
volatile uint8_t taho_timer_ofl;

// uint16_t - max ~104 ms for pic16f targets (* 2.5 for atmega)
volatile uint16_t fuel_duration;

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

#ifdef ADC_BUTTONS

#define ADC_KEY_OK 2
#define ADC_KEY_NEXT 1
#define ADC_KEY_PREV 3

volatile uint8_t adc_key;

#define HW_key1_pressed() (adc_key == ADC_KEY_NEXT)
#define HW_key2_pressed() (adc_key == ADC_KEY_OK)
#define HW_key3_pressed() (adc_key == ADC_KEY_PREV)

#endif

// key variables and flags
volatile uint8_t key_repeat_counter;
volatile flag_t key1_press, key2_press, key1_longpress, key2_longpress, key_pressed;

#if defined(KEY3_SUPPORT)
volatile flag_t key3_press, key3_longpress;
#endif

#if defined(ENCODER_SUPPORT)
volatile flag_t key2_doubleclick;
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

uint16_t calc_filtered_value(filtered_value_t *, uint16_t);

#ifdef CONTINUOUS_DATA_SUPPORT
continuous_data_t cd;
flag_t continuous_data_fl, drive_min_cd_speed_fl;
volatile uint16_t cd_kmh, cd_fuel;
uint16_t cd_speed;
void cd_init(void);
void cd_increment_filter(void);
#endif

#if !defined(SIMPLE_ADC)

uint16_t adc_value;

filtered_value_t f_voltage = {0, 1 | FILTERED_VALUE_FIRST_SAMPLE};

void adc_handler_voltage(filtered_value_t *f);
void adc_handler_buttons(filtered_value_t *f);
void adc_handler_fuel_tank(filtered_value_t *f);

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
    {adc_handler_voltage, &f_voltage, HW_ADC_CHANNEL_POWER_SUPPLY},
#ifdef ADC_BUTTONS
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
    // fuel injector
    if (HW_fuel_active()) {
        if (fuel_fl == 0) {
            // start fuel timer
            HW_start_fuel_timer();
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
            HW_stop_fuel_timer();
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
            HW_stop_fuel_timer();
            taho_fl = 0;
            fuel_duration = 0;
            motor_fl = 0;
        } else {
            taho_timer_ticks += HW_TAHO_TIMER_TICKS_PER_PERIOD;
        }
    }
}

void int_capture_speed_level_change() {
    // speed sensor
    if (HW_tx_active()) {
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
    static flag_t key_longpressed;
#if defined(ENCODER_SUPPORT)
    static uint8_t key2_multiclick_counter, key2_clicks;
    static flag_t key_multiclicked;
#endif    
    static uint8_t key1_counter = 0, key2_counter = 0;
#ifdef KEY3_SUPPORT
    static uint8_t key3_counter = 0;
#endif
    static uint8_t main_interval_counter = MAIN_INTERVAL;

    if (key_repeat_counter == 0) {
#ifdef ENCODER_SUPPORT
        if (config.settings.encoder == 0 && HW_key1_pressed())
#else
        if (HW_key1_pressed()) // key pressed
#endif
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

        if (HW_key2_pressed()) // key pressed
        {
            if (key2_counter <= LONGKEY) {
                key2_counter++;
            }
            if (key2_counter == LONGKEY) {
                // long keypress
                key2_longpress = 1;
                key_longpressed = 1;
            }
#if defined(ENCODER_SUPPORT)
            if (config.settings.encoder != 0 && key2_multiclick_counter == 0) {
                key2_multiclick_counter = MULTICLICK;
            }
#endif
        } else // key released
        {
#if defined(ENCODER_SUPPORT)
            if (config.settings.encoder != 0) {
                if (key2_counter >= LONGKEY) {
                    key2_multiclick_counter = 0;
                    key2_clicks = 0;
                } else {
                    if (key2_counter > DEBOUNCE && key2_counter <= SHORTKEY) {
                        key2_clicks++;
#if defined(SOUND_SUPPORT)
                        buzzer_mode_index = BUZZER_KEY;
#endif                    
                    }
                    if (key2_multiclick_counter == 0) {
                        if (key2_clicks == 1) {
                            key2_press = 1;
                            key_multiclicked = 1;
                        } else if (key2_clicks == 2) {
                            key2_doubleclick = 1;
                            key_multiclicked = 1;
                        }
                        key2_clicks = 0;
                    } else {
                        key2_multiclick_counter--;
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
#ifdef ENCODER_SUPPORT
        if (config.settings.encoder == 0 && HW_key3_pressed())
#else
        if (HW_key3_pressed()) // key pressed
#endif
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
#if defined(ADC_BUTTONS) || defined(FUEL_TANK_SUPPORT)        
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
    if (config.settings.encoder == 0) return;

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

void handle_keys_up_down(uint8_t *v, uint8_t min_value, uint8_t max_value) {
    uint8_t _v = *v;
    if (key1_press != 0) {
        if (_v++ == max_value) {
            _v = min_value;
        }
        timeout_timer1 = 5;
    }
#if defined(ENCODER_SUPPORT)
    if (config.settings.encoder != 0 && key2_press != 0) {
        timeout_timer1 = 0;
    }
    if ((config.settings.encoder == 0 && (key2_press != 0 || key3_press != 0)) || (config.settings.encoder != 0 && key3_press != 0)) {
#elif !defined(KEY3_SUPPORT)
    if (key2_press != 0) {
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
