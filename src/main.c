#include "main.h"
#include "locale.h"
#include "eeprom.h"
#include "lcd.h"
#include "ds1307.h"
#include "ds18b20.h"
#include "utils.h"
#include "i2c.h"
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

#define NO_KEY_PRESSED (key1_press == 0 && key2_press == 0 && key3_press == 0)
#define CLEAR_KEYS_STATE() key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0; key3_press = 0; key3_longpress = 0;

#else

#ifdef KEY3_SUPPORT
#define NO_KEY_PRESSED (key1_press == 0 && key2_press == 0 && key3_press == 0)
#define CLEAR_KEYS_STATE() key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0; key3_press = 0; key3_longpress = 0
#else
#define NO_KEY_PRESSED (key1_press == 0 && key2_press == 0)
#define CLEAR_KEYS_STATE() key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0
#endif

#endif

#ifdef ENCODER_SUPPORT

// todo config.settings.alt_buttons
#define ALT_BUTTONS 1
#define KEY_NEXT ((key1_press != 0 && ALT_BUTTONS == 0) || (key2_press != 0 && ALT_BUTTONS != 0))
#define KEY_OK   ((key2_press != 0 && ALT_BUTTONS == 0) || (key1_press != 0 && ALT_BUTTONS != 0) || (key3_press != 0 && ALT_BUTTONS != 0))
#define KEY_PREV (key3_press != 0 && ALT_BUTTONS == 0)
#define KEY_UP   ((key2_press != 0 && ALT_BUTTONS == 0) || (key1_press != 0 && ALT_BUTTONS != 0))
#define KEY_DOWN (key3_press != 0 && ALT_BUTTONS != 0)

#else

#define KEY_NEXT (key1_press != 0)
#define KEY_OK (key2_press != 0)
#ifdef KEY3_SUPPORT
#define KEY_PREV (key3_press != 0)
#endif

#endif

// key variables and flags
volatile uint8_t key_repeat_counter;
volatile __bit key1_press, key2_press, key1_longpress, key2_longpress, key_pressed;

#if defined(KEY3_SUPPORT)
volatile __bit key3_press, key3_longpress;
#endif

volatile uint8_t shutdown_counter;

// main interval
volatile uint8_t main_interval;
volatile __bit screen_refresh;

// timeout1
volatile uint16_t timeout_timer1;
// timeout2
volatile uint8_t timeout_timer2;

// misc flags
volatile __bit odom_fl, drive_fl, motor_fl, fuel_fl, taho_fl, taho_measure_fl, drive_min_speed_fl;

// acceleration measurement flags and variables
volatile __bit accel_meas_fl, accel_meas_ok_fl, accel_meas_process_fl, accel_meas_timer_fl, accel_meas_drive_fl, accel_meas_final_fl, _accel_meas_exit;
#ifndef SIMPLE_ACCELERATION_MEASUREMENT
volatile uint16_t accel_meas_lower_const;
#endif
volatile uint16_t accel_meas_upper_const, accel_meas_timer, accel_meas_speed;
volatile uint16_t accel_meas_timer1_prev, accel_meas_timer1_ticks;
volatile uint8_t accel_meas_timer1_ofl;

volatile uint16_t kmh_tmp, fuel_tmp;
volatile uint16_t kmh, fuel, adc_voltage;

volatile uint16_t taho_timer1_prev, timer1;
volatile uint8_t taho_timer1_ofl;
volatile uint24_t taho, taho_timer1_ticks;

// uint16_t - max ~26 ms for pic16f targets (* 2.5 for atmega)
volatile uint16_t fuel_duration;

volatile __bit save_tripc_time_fl = 0;
uint16_t speed;

#ifdef TEMPERATURE_SUPPORT
volatile uint8_t timeout_temperature;
__bit temperature_conv_fl;
// ds18b20 temperatures and eeprom pointer
#define TEMP_OUT 0
#define TEMP_IN 1
#define TEMP_ENGINE 2
#define TEMP_CONFIG 3

#define PRINT_TEMP_PARAM_HEADER      0x80
#define PRINT_TEMP_PARAM_MASK 0x0F

uint16_t _t;
uint16_t temps[4] = {DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE};
#endif

#ifdef SOUND_SUPPORT

#define BUZZER_KEY 0
#define BUZZER_LONGKEY 1
#define BUZZER_WARN 2
#define BUZZER_NONE -1

volatile int8_t buzzer_mode_value = BUZZER_NONE;
volatile __bit buzzer_fl, buzzer_init_fl;

typedef struct {
    uint8_t counter; // number of repeats
    uint8_t sound;   // sound on duration  (*0.1ms)
    uint8_t pause;   // sound off duration (*0.1ms)
} buzzer_t;

buzzer_t *buzzer_mode;

buzzer_t buzzer[3] = {
    {1,1,1}, // BUZZER_KEY
    {1,4,1}, // BUZZER_LONG_KEY
    {3,3,2}  // BUZZER_WARN
};

#endif

uint8_t tmp_param = 0, main_param = 0;
#ifdef SERVICE_COUNTERS_SUPPORT
uint8_t service_param = 0;
#endif
        
// buffer for strings
char buf[16];
//char buf2[16];
uint8_t len;

#if defined(_16F876A)
__bank2 ds_time time;
#else
ds_time time;
#endif

uint8_t fuel1_const, fuel2_const;
uint16_t odo_con4;

void screen_main(void);
void screen_tripC(void);
void screen_temp(void);
void screen_max(void);
void screen_tripA(void);
void screen_tripB(void);
void screen_time(void);
#ifdef SERVICE_COUNTERS_SUPPORT
void screen_service_counters(void);
#endif

// max screen in drive mode
#define DRIVE_MODE_MAX 2

const screen_item_t items_main[] = {
    {screen_main},
    {screen_tripC},
#if defined(TEMPERATURE_SUPPORT)
    {screen_temp},
#endif
    {screen_tripA},
    {screen_tripB},
    {screen_time},
#ifdef SERVICE_COUNTERS_SUPPORT   
    {screen_service_counters},
#endif
};

void config_screen_fuel_constant(void);
void config_screen_vss_constant(void);
void config_screen_total_trip(void);
void config_screen_settings_bits(void);
#if defined(DS18B20_CONFIG)
void config_screen_temp_sensors(void);
#endif
#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
void config_screen_service_counters(void);
#endif
void config_screen_ua_const(void);
#ifdef MIN_SPEED_CONFIG
void config_screen_min_speed(void);
#endif
void config_screen_version(void);

const config_screen_item_t items_service[] = {
    {FUEL_CONSTANT_INDEX, config_screen_fuel_constant},
    {VSS_CONSTANT_INDEX, config_screen_vss_constant},
    {TOTAL_TRIP_INDEX, config_screen_total_trip},
    {VOLTAGE_ADJUST_INDEX, config_screen_ua_const},
    {SETTINGS_BITS_INDEX, config_screen_settings_bits},
#ifdef MIN_SPEED_CONFIG
    {MIN_SPEED_INDEX, config_screen_min_speed},
#endif
#if defined(DS18B20_TEMP) && defined(DS18B20_CONFIG)
    {TEMP_SENSOR_INDEX, config_screen_temp_sensors},
#endif
#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
    {SERVICE_COUNTERS_INDEX, config_screen_service_counters},
#endif
    {VERSION_INFO_INDEX, config_screen_version},
};

__bit item_skip;
#define CONFIG_SCREEN_FORCE_ITEM 0x80
#define CONFIG_SCREEN_MASK_ITEM  0x0F

uint8_t request_screen(char *);
void config_screen(uint8_t c_item);

#ifdef ENCODER_SUPPORT
// A valid CW or CCW move returns 1, invalid returns 0.
void read_rotary() {
  static int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
  static uint8_t prevNextCode = 0;
  static uint8_t store = 0;

  prevNextCode <<= 2;
  if (ENCODER_DATA) prevNextCode |= 0x02;
  if (ENCODER_CLK) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

  // If valid then store as 16 bit data.
  if  (rot_enc_table[prevNextCode] ) {
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
            buzzer_mode_value = BUZZER_KEY;
#endif
        }
    }
  }
}
#endif

#if !defined(SIMPLE_ADC)

uint8_t _adc_ch;

typedef void (*handle)(uint16_t adc_value);

void adc_handler_voltage(uint16_t adc_value);
#ifdef ADC_BUTTONS
void adc_handler_buttons(uint16_t adc_value);
#endif
void adc_handler_fuel_tank(uint16_t adc_value);

typedef struct {
    void (*handle)(uint16_t adc_value);
    uint16_t tmp;
    uint8_t filter; // filter value (power of 2)
    uint8_t channel; // adc channel
    uint16_t dummy; // align to 8 byte
} adc_item_t;

adc_item_t adc_items[] = {
    {adc_handler_voltage, 0xFFFF, 2, ADC_CHANNEL_POWER_SUPPLY},
#ifdef ADC_BUTTONS
    {adc_handler_buttons, 0xFFFF, 0, ADC_CHANNEL_BUTTONS},
#endif
    //{adc_handler_fuel_tank, 0xFFFF, 4, ADC_CHANNEL_FUEL_TANK},
};

void adc_handler_voltage(uint16_t adc_value) {
    adc_voltage = adc_value;
    // read power supply status
    if (adc_voltage > THRESHOLD_VOLTAGE_ADC_VALUE) {
        shutdown_counter = 0;
    } else {
        if (shutdown_counter == SHUTDOWN_COUNTER) {
            screen_refresh = 1;
        } else {
            shutdown_counter++;
        }
    }
};

#ifdef ADC_BUTTONS
void adc_handler_buttons(uint16_t adc_value) {
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

void adc_handler_fuel_tank(uint16_t adc_value) {
}

#endif

#if defined(__AVR)
void inc_fuel(trip_t* trip) {
    if (--trip->fuel_tmp1 == 0) {
        trip->fuel_tmp1 = fuel1_const;
        if (++trip->fuel_tmp2 >= config.fuel_const) {
            trip->fuel_tmp2 = 0;
            trip->fuel++;
        }
    }
}

void inc_odo(trip_t* trip) {
    if (++trip->odo_temp > config.odo_const) {
        trip->odo_temp = 0;
        trip->odo++;
    }
}
#endif

int_handler_GLOBAL_begin

#if defined(ENCODER_SUPPORT) && defined(int_handler_encoder_begin) && defined(int_handler_encoder_end)
    int_handler_encoder_begin
        // handle encoder
        read_rotary();
    int_handler_encoder_end
#endif

#if defined(int_handler_fuel_speed_begin)
    int_handler_fuel_speed_begin

        // capture 0.01s timer value
        get_timer1(timer1);
#elif defined(int_handler_fuel_begin)
    int_handler_fuel_begin

        // capture fuel level change timer value
        capture_fuel(timer1);
#endif

        // fuel injector
        if (FUEL_ACTIVE) {
            if (fuel_fl == 0) {
                // start timer0
                start_timer_fuel();
                fuel_fl = 1;
                motor_fl = 1;
                save_tripc_time_fl = 1;

#ifdef SERVICE_COUNTERS_SUPPORT
                services.mh.counter_rpm ++;
                if (config.settings.par_injection == 0) {
                    services.mh.counter_rpm++;
                }
#endif

// new taho calculation based on captured value of 0.01s timer
                if (taho_measure_fl == 0) {
                    taho_measure_fl = 1;

                    taho_timer1_ticks = 0;
                    taho_timer1_prev = timer1;
                    taho_timer1_ofl = 0;
                } else {
                    taho = taho_timer1_ticks + timer1 - taho_timer1_prev;
                    taho_fl = 1;

                    taho_timer1_ticks = 0;
                    taho_timer1_prev = timer1;
                    taho_timer1_ofl = 0;
                }
            }
        } else {
            if (fuel_fl != 0) {
                // stop timer0
                stop_timer_fuel();
                fuel_fl = 0;
                // measure fuel duration                
                if (taho_measure_fl != 0) {
                    fuel_duration = (uint16_t) (taho_timer1_ticks + timer1 - taho_timer1_prev);
                }
            }
        }

#if defined(int_handler_fuel_end) && defined(int_handler_speed_begin)
    int_handler_fuel_end
    
    int_handler_speed_begin

        // capture speed level change timer value
        capture_speed(timer1);
#endif    
        // speed sensor
        if (TX_ACTIVE) {
            if (odom_fl == 0) {
                odom_fl = 1;
                drive_fl = 1;
                
                // new speed 100 calculation based on captured value of 0.01s timer
                if (accel_meas_process_fl != 0) {
                    if (accel_meas_fl == 0) {
                        accel_meas_fl = 1;
                        accel_meas_timer1_prev = timer1;
                        accel_meas_timer1_ticks = 0;
                        accel_meas_timer1_ofl = 0;
                    } else {
                        accel_meas_speed = accel_meas_timer1_ticks + timer1 - accel_meas_timer1_prev;
                        accel_meas_drive_fl = 1;
#ifndef SIMPLE_ACCELERATION_MEASUREMENT
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
                            accel_meas_timer1_prev = timer1;
                            accel_meas_timer1_ticks = 0;
                            accel_meas_timer1_ofl = 0;
                        }
                    }
                } else {
                    accel_meas_fl = 0;
                }

                kmh_tmp++;

                // main odometer
                if (++config.odo_temp >= config.odo_const) {
                    config.odo_temp = 0;
                    // increment odometer counters
                    config.odo++;
                    services.srv[0].counter++;
                    services.srv[1].counter++;
                    services.srv[2].counter++;
                    services.srv[3].counter++;
                }

#if defined(__AVR)
                inc_odo(&trips.tripA);
                inc_odo(&trips.tripB);
                inc_odo(&trips.tripC);
#else
                // eliminate possible stack overflow with pic16f876a

                // trip A
                if (++trips.tripA.odo_temp >= config.odo_const) {
                    trips.tripA.odo_temp = 0;
                    trips.tripA.odo++;
                }

                // trip B
                if (++trips.tripB.odo_temp >= config.odo_const) {
                    trips.tripB.odo_temp = 0;
                    trips.tripB.odo++;
                }

                // trip C
                if (++trips.tripC.odo_temp >= config.odo_const) {
                    trips.tripC.odo_temp = 0;
                    trips.tripC.odo++;
                }
#endif
            }
        } else {
            odom_fl = 0;
}

#if defined(int_handler_speed_end)
    int_handler_speed_end
#elif defined(int_handler_fuel_speed_end)
    int_handler_fuel_speed_end
#endif

    int_handler_timer0_begin
            
        fuel_tmp++;

#if defined(__AVR)
        inc_fuel(&trips.tripA);
        inc_fuel(&trips.tripB);
        inc_fuel(&trips.tripC);
#else
        // eliminate possible stack overflow with pic16f876a
        
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
#endif
        
    int_handler_timer0_end

    int_handler_timer1_begin

        static uint8_t key1_counter = 0, key2_counter = 0;
#ifdef KEY3_SUPPORT
        static uint8_t key3_counter = 0;
#endif
        static uint8_t main_interval_counter;

        if (taho_measure_fl != 0) {
            if (++taho_timer1_ofl == TAHO_OVERFLOW) {
                taho_measure_fl = 0;
                stop_timer_fuel();
                taho_fl = 0;
                fuel_duration = 0;
                motor_fl = 0;
            } else {
                taho_timer1_ticks += TIMER1_VALUE;
            }
        }

        if (accel_meas_fl != 0) {
            if (++accel_meas_timer1_ofl == ACCEL_MEAS_OVERFLOW) {
                accel_meas_fl = 0;
                accel_meas_drive_fl = 0;
            } else {
                accel_meas_timer1_ticks += TIMER1_VALUE;
            }
        }

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
                    key_pressed = 1;
#ifdef SOUND_SUPPORT
                    buzzer_mode_value = BUZZER_LONGKEY;
#endif
                }
            } else // key released
            {
                if (key1_counter > DEBOUNCE && key1_counter <= SHORTKEY) {
                    // key press
                    key1_press = 1;
                    key_pressed = 1;
#ifdef SOUND_SUPPORT
                    buzzer_mode_value = BUZZER_KEY;
#endif
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
                    key_pressed = 1;
#ifdef SOUND_SUPPORT
                    buzzer_mode_value = BUZZER_LONGKEY;
#endif
                }
            } else // key released
            {
                if (key2_counter > DEBOUNCE && key2_counter <= SHORTKEY) {
                    // key press
                    key2_press = 1;
                    key_pressed = 1;
#ifdef SOUND_SUPPORT
                    buzzer_mode_value = BUZZER_KEY;
#endif
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
                    key_pressed = 1;
#ifdef SOUND_SUPPORT
                    buzzer_mode_value = BUZZER_LONGKEY;
#endif
                }
            } else // key released
            {
                if (key3_counter > DEBOUNCE && key3_counter <= SHORTKEY) {
                    // key press
                    key3_press = 1;
                    key_pressed = 1;
#ifdef SOUND_SUPPORT
                    buzzer_mode_value = BUZZER_KEY;
#endif
                }
                key3_counter = 0;
            }
#endif
        } else {
            key_repeat_counter--;
        }

        if (key_pressed == 1) {
            key_pressed = 0; screen_refresh = 1;
            key_repeat_counter = KEY_REPEAT_PAUSE; 
        }

#ifdef SOUND_SUPPORT
        static __bit buzzer_snd_fl, buzzer_repeat_fl;
        static uint8_t buzzer_counter_r;
        static uint8_t buzzer_counter, buzzer_counter_01sec;

        if (buzzer_mode_value != BUZZER_NONE) {
            if (config.settings.key_sound != 0 || buzzer_mode_value == BUZZER_WARN) {
                buzzer_fl = 1; buzzer_init_fl = 0; buzzer_mode = &buzzer[buzzer_mode_value];
                buzzer_counter_01sec = 1;
            }
            buzzer_mode_value = BUZZER_NONE;
        }
#endif    
    
        if (++main_interval_counter >= main_interval) {
            main_interval_counter = 0;

            // screen refresh_flag
            screen_refresh = 1;
             
            // copy temp interval variables to main
            fuel = fuel_tmp;
            fuel_tmp = 0;
            kmh = kmh_tmp;
            kmh_tmp = 0;
             
            static __bit time_increment_fl = 0;
 
            // if fast refresh enabled increment time counters every second time
            if (time_increment_fl != 0) {
                time_increment_fl = 0;
                // increment time counters
                if (motor_fl != 0 || drive_fl != 0) {
                     services.mh.counter++;
                     trips.tripA.time++;
                     trips.tripB.time++;
                     trips.tripC.time++;
                }

#ifdef TEMPERATURE_SUPPORT
                if (timeout_temperature > 0) {
                    timeout_temperature--;
                }
#endif                
            } else {
                time_increment_fl = 1;
            }
             
        }
        
        if (timeout_timer1 > 0) {
            timeout_timer1--;
        }

        if (timeout_timer2 > 0) {
            timeout_timer2--;
        }

        if (accel_meas_timer_fl != 0) {
            accel_meas_timer++;
        }

#ifdef SOUND_SUPPORT
        if (buzzer_fl != 0) {
            if (--buzzer_counter_01sec == 0) {
                buzzer_counter_01sec = TIMER_01SEC_INTERVAL;
                if (buzzer_init_fl == 0) {
                    buzzer_init_fl = 1;
                    buzzer_repeat_fl = 1;
                    buzzer_counter_r = buzzer_mode->counter + 1;
                }
                
                if (buzzer_repeat_fl != 0) {
                    buzzer_repeat_fl = 0;
                    if (--buzzer_counter_r > 0) {
                        buzzer_snd_fl = 1;
                        buzzer_counter = buzzer_mode->sound;
                    } else {
                        buzzer_fl = 0;
                        buzzer_init_fl = 0;
                    }
                }
        
                if (buzzer_snd_fl != 0) {
                    if (buzzer_counter == 0) {
                        buzzer_snd_fl = 0;
                        buzzer_counter = buzzer_mode->pause - 1;
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
    int_handler_timer1_end

    int_handler_adc_begin

#if defined(SIMPLE_ADC)

        adc_voltage = adc_read_value();

#if defined(__DEBUG)
        // RA0 is configured as analog pin
        PWR_ON;
#endif

        // read power supply status
        if (adc_voltage > THRESHOLD_VOLTAGE_ADC_VALUE) {
            shutdown_counter = 0;
        } else {
            if (shutdown_counter == SHUTDOWN_COUNTER) {
                screen_refresh = 1;
            } else {
                shutdown_counter++;
            }
        }

#else
        adc_item_t* h = &adc_items[_adc_ch];
        if (h->filter == 0) {
            h->handle(adc_read_value());
        } else {
            if (h->tmp == 0xFFFF) {
                h->tmp = adc_read_value() << h->filter;
            } else {
                h->tmp = h->tmp - ((h->tmp + (1 << (h->filter - 1))) >> h->filter) + adc_read_value();
            }
            h->handle((h->tmp + (1 << (h->filter - 1))) >> h->filter);
        }
        _adc_ch++;
        if (_adc_ch >= sizeof (adc_items) / sizeof (adc_item_t)) {
            _adc_ch = 0;
        }
        set_adc_channel(adc_items[_adc_ch].channel);
#endif


#if defined(restart_adc_event)
        restart_adc_event();
#endif        
        
    int_handler_adc_end
    
int_handler_GLOBAL_end

void print_time_hm(unsigned char hour, unsigned char minute, align_t align) {
    bcd8_to_str(buf, hour);
    buf[2] = ':';
    bcd8_to_str(&buf[3], minute);
    LCD_Write_String8(buf, 5, align);
}

void print_time_dmy(unsigned char day, unsigned char month, unsigned char year) {
    if (day == 0x00 || day == 0xFF) {
        strcpy2(buf, (char*) &no_time_string, 0);
    } else {
        bcd8_to_str(buf, day);
        strcpy2(&buf[2], (char *) &month_str, bcd8_to_bin(month));
        buf[5] = '\'';
        bcd8_to_str(&buf[6], year);
    }
    LCD_Write_String8(buf, 8, ALIGN_LEFT);

}

void print_time_dow(unsigned char day_of_week) {
    LCD_Write_String16(buf, strcpy2((char *)buf, (char*) day_of_week_str, day_of_week), ALIGN_LEFT);
}

void print_time(ds_time* time) {
    LCD_CMD(0x80);
    print_time_hm(time->hour, time->minute, ALIGN_LEFT);

    LCD_CMD(0x88);
    print_time_dmy(time->day, time->month, time->year);

    LCD_CMD(0xc0);
    print_time_dow(time->day_of_week);
}

/**
 * print symbols from symbols_str with [index] in buf at [len] position
 * @param len
 * @param index
 * @return 
 */
unsigned char print_symbols_str(unsigned char len, unsigned char index) {
    return strcpy2(&buf[len], (char *)symbols_str, index);
}

void _print(unsigned char len, unsigned char pos, align_t align) {
    len += print_symbols_str(len, pos);
    LCD_Write_String8(buf, len, align);
}

/**
 * print fractional number [num/10^frac].[num%10^frac]
 * @param buf
 * @param num
 * @param frac numbers after '.'
 * @return 
 */
unsigned char print_fract(char * buf, uint16_t num, uint8_t frac) {
    unsigned char len = ultoa2(buf, num, 10);

    // add leading zeroes
    if (len < frac + 1) {
        add_leading_symbols(buf, '0', len, frac + 1);
        len = frac + 1;
    }

    // shift right and add '.'
    unsigned char tmp = len;
    while (frac--) {
        buf[tmp] = buf[tmp - 1];
        tmp--;
    }
    buf[tmp] = '.';

    return len + 1;
}

/**
 * show trip time
 * @param t
 * @param align
 */
void print_trip_time(trip_t* t, align_t align) {
    unsigned short time = (unsigned short) (t->time / 30);

    len = ultoa2(buf, (unsigned short) (time / 60), 10);
    buf[len++] = ':';

    bcd8_to_str(&buf[len], bin8_to_bcd(time % 60));
    len += 2;
   
    LCD_Write_String8(buf, len, align);
}

/**
 * show trip average speed
 * @param t
 * @param align
 */
void print_trip_average_speed(trip_t* t, align_t align) {
    
    unsigned short average_speed = 0;
    if (t->time > 0) {
        average_speed = (unsigned short) ((unsigned long) ((t->odo * 18000UL) + (t->odo_temp * 18000UL / config.odo_const)) / t->time);
    }
    
    if (average_speed == 0) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        len = print_fract(buf, average_speed, 1);
    }

    _print(len, POS_KMH, align);
}

uint16_t get_odometer(trip_t* t) {
    return (uint16_t) (t->odo * 10UL + (uint16_t) (t->odo_temp * 10UL / config.odo_const));
}

/**
 * show trip odometer (km)
 * @param t
 * @param align
 */
void print_trip_odometer(trip_t* t, align_t align) {
    uint16_t odo = get_odometer(t);
    len = print_fract(buf, odo, 1);

    _print(len, POS_KM, align);
}

/**
 * show trip total fuel consumption (l)
 * @param t
 * @param align
 */
void print_trip_total_fuel(trip_t* t, align_t align) {
    len = print_fract(buf, t->fuel / 10, 1);

    _print(len, POS_LITR, align);
}

/**
 * show trip average fuel consumption (l/100km)
 * @param t
 * @param align
 */
void print_trip_average_fuel(trip_t* t, align_t align) {
    uint16_t odo = get_odometer(t);
    if (t->fuel < AVERAGE_MIN_FUEL || odo < AVERAGE_MIN_DIST) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        len = print_fract(buf, (uint16_t) (t->fuel * 100UL / odo), 1);
    }

    _print(len, POS_LKM, align);
}

void print_speed(unsigned short speed, unsigned short i, align_t align) {
    // use fractional by default
    len = print_fract(&buf[i], speed, 1);

    if (speed > 1000 || i == 0) {
        // more than 100 km/h (or current speed), skip fractional
        len -= 2;
    }

    len += i;

    _print(len, POS_KMH, align);
}

void print_taho(align_t align) {
    if (taho_fl == 0) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        unsigned short res = (unsigned short) (((config.settings.par_injection != 0 ? (TAHO_CONST) : (TAHO_CONST*2)) / taho));
#ifdef TAHO_ROUND
        res = (res + TAHO_ROUND / 2) / TAHO_ROUND * TAHO_ROUND;        
#endif
        len = ultoa2(buf, res, 10);
    }

    _print(len, POS_OMIN, align);
}

void print_fuel_duration(align_t align) {
//    if (fuel_duration == 0) {
//        len = strcpy2(buf, (char *) &empty_string, 0);
//    } else
    {
        uint16_t res = (uint16_t) (fuel_duration * (1000 / 250) / (TIMER1_VALUE / 250));
        len = print_fract(buf, res, 2);
    }
    _print(len, POS_MS, align);
}

void print_current_fuel_km(align_t align) {
    unsigned short t = (unsigned short) ((((unsigned long) fuel * (unsigned long) odo_con4) / (unsigned long) kmh) / (unsigned char) config.fuel_const);
    len = print_fract(buf, t, 1);

    _print(len, POS_LKM, align);
}

void print_current_fuel_lh(align_t align) {
    unsigned short t = (unsigned short) (((unsigned long) fuel * (unsigned long) fuel2_const / (unsigned long) config.fuel_const) / 10UL);
    len = print_fract(buf, t, 1);

    _print(len, POS_LH, align);
}

void print_main_odo(align_t align) {
    len = ultoa2(buf, (unsigned long) config.odo, 10);

    _print(len, POS_KM, align);
}

#if defined(TEMPERATURE_SUPPORT)
uint8_t print_temp(uint8_t index, align_t align) {

    uint8_t _print_header = (index & PRINT_TEMP_PARAM_HEADER) != 0;
    index &= PRINT_TEMP_PARAM_MASK;

    _t = temps[index];

    if (_t == DS18B20_TEMP_NONE) {
        if (index == TEMP_ENGINE || index == TEMP_CONFIG) {
            buf[0] = '-'; buf[1] = '-';
            len = 2;
        } else {
            len = strcpy2(buf, (char *) &empty_string, 0);
        }
    } else {
        uint8_t _index = 0;
        if (_t & 0x8000) // if the temperature is negative
        {
            buf[0] = '-'; // put minus sign (-)
            _t = (~_t) + 1; // change temperature value to positive form
            _index++;
        } else {
            if (index != TEMP_ENGINE) {
              buf[0] = '+';
              _index++;
            }
        }
        _t = (unsigned short) ((_t >> 4) * 10 + (((_t & 0x000F) * 10) >> 4));

        if (index == TEMP_ENGINE || index == TEMP_CONFIG) {
            len = ultoa2(&buf[_index], _t / 10, 10) + _index;
        } else {
            len = print_fract(&buf[1], _t, 1) + 1;
        }
    }

    if (_print_header) {
        add_leading_symbols(buf, ' ', len, 8);
        strcpy2(buf, (char *) &temp_sensors, (index + 1) + 1);
        len = 8;
    } else if (index != TEMP_CONFIG) {
        len += print_symbols_str(len, POS_CELS);
    }
    LCD_Write_String8(buf, len, align);
    return len;
}

#endif

void print_voltage(align_t align) {
    len = print_fract(buf, (unsigned short) (adc_voltage << 5) / config.vcc_const, 1);

    _print(len, POS_VOLT, align);
}

void handle_keys_up_down(uint8_t *v, uint8_t min_value, uint8_t max_value) {
    uint8_t _v = *v;
    if (key1_press != 0) {
        if (_v++ == max_value) {
            _v = min_value;
        }
        timeout_timer1 = 512;
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
        timeout_timer1 = 512;
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
        timeout_timer1 = 512;
    }

#if defined(KEY3_SUPPORT)
    // change cursor to prev position
    if (key3_press != 0) {
        if (_v-- == min_value) {
            _v = max_value;
        }
        timeout_timer1 = 512;
    }
#endif
    *v = _v;
}

#if defined(ENCODER_SUPPORT)
void handle_keys_next_prev_enc(uint8_t *v, uint8_t min_value, uint8_t max_value) {
    uint8_t _v = *v;
    // change cursor to next position
    if (KEY_NEXT) {
        if (_v++ == max_value) {
            _v = min_value;
        }
        timeout_timer1 = 512;
    }

    // change cursor to prev position
    if (KEY_PREV) {
        if (_v-- == min_value) {
            _v = max_value;
        }
        timeout_timer1 = 512;
    }
    *v = _v;
}
#endif

void screen_time(void) {

    get_ds_time(&time);

    if (request_screen((char *) &time_correction) != 0) {

        uint8_t c = 0;
        uint8_t *p, min, max;

        const char cursor_position[] = {0x81, 0x84, 0x89, 0x8c, 0x8f, 0xc0};

        LCD_Clear();
        timeout_timer1 = 512;
        while (timeout_timer1 != 0) {
            screen_refresh = 0;
#if defined(ENCODER_SUPPORT)
            handle_keys_next_prev_enc(&c, 0, 6);            
#else
            handle_keys_next_prev(&c, 0, 6);            
#endif

            if (KEY_OK) {
                switch (c) {
                    case 0:
                        p = &time.hour; min = 0; max = 23;
                        break;
                    case 1:
                        p = &time.minute; min = 0; max = 59;
                        break;
                    case 2:
                        p = &time.day; min = 1; max = 31;
                        break;
                    case 3:
                        p = &time.month; min = 1; max = 12;
                        break;
                    case 4:
                        p = &time.year; min = VERSION_YEAR; max = VERSION_YEAR + 10;
                        break;
                    case 5:
                        p = &time.day_of_week; min = 1; max = 7;
                        break;
                }
#if !defined(ENCODER_SUPPORT)
                *p = bcd8_inc(*p, min, max);
#else
                if (KEY_DOWN) {
                    *p = bcd8_dec(*p, min, max);
                } else {
                    *p = bcd8_inc(*p, min, max);
                }
#endif

#ifdef AUTO_DAY_OF_WEEK
                if (c == 2 || c == 3 || c == 4) {
                    set_day_of_week(&time);
                }
#endif                
                timeout_timer1 = 512;
            }

            CLEAR_KEYS_STATE();

            LCD_CMD(LCD_CURSOR_OFF);
            print_time(&time);
            LCD_CMD(cursor_position[c]);
            LCD_CMD(LCD_BLINK_CURSOR_ON);

            while (screen_refresh == 0 && timeout_timer1 != 0);
        }
        LCD_CMD(LCD_CURSOR_OFF);
        // save time
        set_ds_time(&time);
        key2_longpress = 0;

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
    timeout_timer1 = 512;
    while (timeout_timer1 != 0) {
        screen_refresh = 0;

        handle_keys_up_down(&v, min_value, max_value);

        len = ultoa2(buf, (unsigned long) (mode == CHAREDIT_MODE_10000KM ? (v * 1000L) : v), 10);

        len += print_symbols_str(len, (unsigned char) mode);

        LCD_CMD(0xC4);
        LCD_Write_String8(buf, len, ALIGN_RIGHT);

        CLEAR_KEYS_STATE();

        while (screen_refresh == 0 && timeout_timer1 != 0);
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
    if (v > max_value) {
        v = max_value;
    }

    // convert value
    unsigned char v_len = ultoa2(buf, v, 10);

    add_leading_symbols(buf, '0', v_len, max_len);

    unsigned char cursor_pos = 0xC0 + (16 - max_len) / 2U;
    unsigned char pos = 0;

    timeout_timer1 = 512;
    while (timeout_timer1 != 0) {
        screen_refresh = 0;

#if defined(ENCODER_SUPPORT)
        handle_keys_next_prev_enc(&pos, 0, max_len - 1);
#else
        handle_keys_next_prev(&pos, 0, max_len - 1);
#endif

        // edit number in cursor position
        if (KEY_OK) {
#if defined(ENCODER_SUPPORT)
            if (KEY_DOWN) {
                if (--buf[pos] < '0') {
                    buf[pos] = pos == 0 ? _max_symbol_pos0 : '9';
                }
            } else
#endif            
                if (++buf[pos] > '9') {
                buf[pos] = '0';
            }

            unsigned long _t = strtoul2(buf);
            if (_t > max_value) {
                buf[pos] = '0';
            }

            timeout_timer1 = 512;
        }

        LCD_CMD(LCD_CURSOR_OFF);

        LCD_CMD(cursor_pos);
        LCD_Write_String8(buf, max_len, ALIGN_LEFT);

        LCD_CMD(cursor_pos + pos);
        LCD_CMD(LCD_BLINK_CURSOR_ON);

        CLEAR_KEYS_STATE();

        while (screen_refresh == 0 && timeout_timer1 != 0);
    }

    LCD_CMD(LCD_CURSOR_OFF);
    screen_refresh = 1;

    return strtoul2(buf);
}

uint16_t edit_value_bits(uint16_t v, char* str) {

#define cursor_pos 0xC0

    uint8_t pos = 0;

    timeout_timer1 = 512;
    while (timeout_timer1 != 0) {
        screen_refresh = 0;

        handle_keys_next_prev(&pos, 0, 16 - 1);

        // edit number in cursor position
        if (key2_press != 0) {
            v ^= (1 << (15 - pos));
            timeout_timer1 = 512;
        }

        LCD_CMD(LCD_CURSOR_OFF);

        add_leading_symbols(buf, '0', ultoa2(buf, v, 2), 16);

        unsigned char _onoff = (buf[pos] - '0') + 1;

        LCD_CMD(cursor_pos);
        LCD_Write_String16(buf, 16, ALIGN_LEFT);

        _memset(buf, ' ', 16);
        len = strcpy2((char*) buf, (char *) str, pos + 1);
        if (len != 0) {
            // print on/off
            strcpy2((char*) &buf[12], (char *) &onoff_string, _onoff);
        }
        LCD_CMD(0x80);
        LCD_Write_String16(buf, 16, ALIGN_LEFT);

        LCD_CMD(cursor_pos + pos);
        LCD_CMD(LCD_BLINK_CURSOR_ON);

        CLEAR_KEYS_STATE();

        while (screen_refresh == 0 && timeout_timer1 != 0);
    }

    LCD_CMD(LCD_CURSOR_OFF);
    screen_refresh = 1;

    return v;
}

unsigned char request_screen(char* request_str) {
    unsigned char res = 0;
    if (key2_longpress != 0) {
        CLEAR_KEYS_STATE();

        LCD_Clear();
        LCD_CMD(0x80);
        LCD_Write_String16(buf, strcpy2(buf, request_str, 0), ALIGN_CENTER);

        timeout_timer1 = 512;
        while (timeout_timer1 != 0 && NO_KEY_PRESSED);

        if (key2_press != 0) {
            res = 1;
        }

        CLEAR_KEYS_STATE();
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

#define NUM_PARAMS 9

void print_selected_param1(align_t align) {
    switch (select_param(&main_param, NUM_PARAMS)) {
        case 0:
#if defined(TEMPERATURE_SUPPORT)
            print_temp(config.settings.show_inner_temp ? TEMP_IN : TEMP_OUT, align);
            break;
#else
            main_param++;
#endif            
        case 1:
            print_voltage(align);
            break;
        case 2:
            print_trip_time(&trips.tripC, align);
            break;
        case 3:
            print_trip_odometer(&trips.tripC, align);
            break;
        case 4:
            print_trip_average_fuel(&trips.tripC, align);
            break;
        case 5:
            print_trip_average_speed(&trips.tripC, align);
            break;
        case 6:
            print_trip_total_fuel(&trips.tripC, align);
            break;
        case 7:
            print_speed(trips.tripC_max_speed, print_symbols_str(0, POS_MAXS), align);
            break;
        case 8:
            print_fuel_duration(align);
            break;
    }
}

#ifdef SIMPLE_ACCELERATION_MEASUREMENT
void acceleration_measurement(void) {
#else

typedef struct {
    uint32_t lower;
    uint32_t upper;
} accel_meas_limits_t;

accel_meas_limits_t accel_meas_limits[4] = {
    {0, speed_const(100)},
    {0, speed_const(60)},
    {speed_const(60), speed_const(100)},
    {speed_const(80), speed_const(120)},
};

void acceleration_measurement(uint8_t index) {
#endif
    LCD_CMD(0x80);
    LCD_Write_String16(buf, strcpy2(buf, (char *) &accel_meas_wait_string, 0), ALIGN_CENTER);

    accel_meas_fl = 0; accel_meas_ok_fl = 0; accel_meas_timer = 0; accel_meas_final_fl = 0; _accel_meas_exit = 0;

#ifndef SIMPLE_ACCELERATION_MEASUREMENT
    accel_meas_lower_const = (unsigned short) (accel_meas_limits[index].lower / config.odo_const);
    accel_meas_upper_const = (unsigned short) (accel_meas_limits[index].upper / config.odo_const);
#endif
    
    _memset(buf, '=', 16);

    // 15 sec waiting for start
    timeout_timer1 = 1536;

    timeout_timer2 = 0;
    while (_accel_meas_exit == 0 && NO_KEY_PRESSED) {
        if (timeout_timer1 != 0 && drive_fl == 0) {
            if (timeout_timer2 == 0) {
                timeout_timer2 = (uint8_t) (TIMER_01SEC_INTERVAL * 2.5f);
                LCD_CMD(0xC0);
                LCD_Write_String16(buf, (unsigned char) (timeout_timer1 / 91), ALIGN_LEFT);
            }
        } else {
            if (timeout_timer1 != 0) {
                if (accel_meas_ok_fl == 0 && accel_meas_process_fl == 0) {
                    // 30 sec for acceleration measurement
                    timeout_timer1 = 3072;
                    accel_meas_process_fl = 1;

                    LCD_Clear();
                }

#if 1
                len = print_fract((char*) buf, accel_meas_timer, 2);
                len += print_symbols_str(len, POS_SEC);
                add_leading_symbols(buf, ' ', len, 16);

                len = ultoa2((char*) buf, accel_meas_drive_fl != 0 ? (uint16_t) ((360000UL * TIMER1_VALUE / config.odo_const) / accel_meas_speed) : 0, 10); // integer
                /*len += */print_symbols_str(len, POS_KMH);

                LCD_CMD(0xC0);
                LCD_Write_String16(buf, 16, ALIGN_CENTER);
#else

                len = ultoa2((char*) buf, accel_meas_drive_fl != 0 ? (uint16_t) ((360000UL * TIMER1_VALUE / config.odo_const) / accel_meas_speed) : 0, 10); // integer
                len += print_symbols_str(len, POS_KMH);
                LCD_CMD(0xC0);
                LCD_Write_String8(buf, len, ALIGN_LEFT);

                len = print_fract((char*) buf, accel_meas_timer, 2);
                len += print_symbols_str(len, POS_SEC);
                LCD_CMD(0xC8);
                LCD_Write_String8(buf, len, ALIGN_RIGHT);
#endif
            }

            if (timeout_timer1 != 0 && accel_meas_ok_fl == 0) {
                timeout_timer2 = (uint8_t) (TIMER_01SEC_INTERVAL / 2.5); while (timeout_timer2 != 0);
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
                        LCD_Write_String16(buf, strcpy2(buf, (char *) &timeout_string, 0), ALIGN_CENTER);
                    }
#ifdef SOUND_SUPPORT
                    buzzer_mode_value = BUZZER_WARN;
#endif
                    timeout_timer1 = 1024; while (timeout_timer1 != 0 && NO_KEY_PRESSED);
                    _accel_meas_exit = 1;
                }
            }
        }
    }
    accel_meas_process_fl = 0;
    CLEAR_KEYS_STATE();
}

#ifndef SIMPLE_ACCELERATION_MEASUREMENT
void select_acceleration_measurement() {
    CLEAR_KEYS_STATE();

    LCD_Clear();

    uint8_t v = 0, max_value = sizeof (accel_meas_limits) / sizeof (accel_meas_limits[0]) - 1, index = 0xFF;

    timeout_timer1 = 512;
    while (timeout_timer1 != 0) {
        screen_refresh = 0;

        handle_keys_next_prev(&v, 0, max_value);

        if (key2_press != 0) {
            timeout_timer1 = 0;
            index = v;
        }

        CLEAR_KEYS_STATE();

        LCD_CMD(0x80);
        len = strcpy2(buf, (char *) &accel_meas_timing_string, 0);
        len += strcpy2(&buf[len], (char *) accel_meas_string, v + 1);

        LCD_Write_String16(buf, len, ALIGN_CENTER);

        while (screen_refresh == 0 && timeout_timer1 != 0);
    }


    if (index != 0xFF) {
        acceleration_measurement(index);
    }
}
#endif

void screen_main(void) {
//; первый экран

    LCD_CMD(0x80);
    if (drive_min_speed_fl == 0) {
//; 1) на месте с заглушенным двигателем
//; время текущее       общий пробег (км)
//; нар.темп./пробег C  вольтметр
        get_ds_time(&time);
        print_time_hm(time.hour, time.minute, ALIGN_LEFT);

        if (motor_fl == 0) {
            print_main_odo(ALIGN_RIGHT);

            LCD_CMD(0xC0);
#if defined(TEMPERATURE_SUPPORT)
            print_temp(config.settings.show_inner_temp ? TEMP_IN : TEMP_OUT, ALIGN_LEFT);
#else
            print_trip_odometer(&trips.tripC, ALIGN_LEFT);
#endif
            print_voltage(ALIGN_RIGHT);
        } else {
//; 2) на месте с работающим двигателем
//; время текущее       тахометр (об/мин)
//; selected_param1 	мгновенный расход (л/ч)
            print_taho(ALIGN_RIGHT);
            LCD_CMD(0xC0);
            print_selected_param1(ALIGN_LEFT);
            print_current_fuel_lh(ALIGN_RIGHT);
        }
    } else {
//; 3) в движении
//; скорость (км/ч)     тахометр (об/мин)
//; selected_param1 	мгновенный расход (л/100км)
        print_speed(speed, 0, ALIGN_LEFT);
        print_taho(ALIGN_RIGHT);

        LCD_CMD(0xC0);
        print_selected_param1(ALIGN_LEFT);
        print_current_fuel_km(ALIGN_RIGHT);
    }

#ifdef SIMPLE_ACCELERATION_MEASUREMENT
    if (drive_fl == 0 && motor_fl != 0 && request_screen((char *) &accel_meas_string) != 0) {
        acceleration_measurement();
    }
#else
    if (drive_fl == 0 && motor_fl != 0 && key2_longpress != 0) {
        select_acceleration_measurement();
    }
#endif        
}

void clear_trip(bool force, trip_t* trip) {
    if (force || request_screen((char *) &reset_string) != 0) {
        _memset(trip, 0, sizeof(trip_t));
    }
}

void screen_trip(trip_t* trip, unsigned char trips_pos) {
    len = strcpy2(buf, (char *) &trip_string, 0);
    len += strcpy2(&buf[len], (char *) trips_str, trips_pos);

    LCD_CMD(0x80);
    LCD_Write_String8(buf, len, ALIGN_LEFT);

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

void screen_tripC() {
    // второй экран - экран счетчика C
    screen_trip(&trips.tripC, config.settings.daily_tripc ? TRIPS_POS_DAY : TRIPS_POS_CURR);
}

void screen_tripA() {
    // экран счетчика A
    screen_trip(&trips.tripA, TRIPS_POS_A);
}

void screen_tripB() {
    // экран счетчика B
    screen_trip(&trips.tripB, TRIPS_POS_B);
}

#ifdef TEMPERATURE_SUPPORT
void screen_temp() {
    if (config.settings.skip_temp_screen) {
        item_skip = 1;
    } else {
        LCD_CMD(0x80);
        print_temp(TEMP_OUT | PRINT_TEMP_PARAM_HEADER, ALIGN_RIGHT);
        LCD_CMD(0xC0);
        print_temp(TEMP_IN | PRINT_TEMP_PARAM_HEADER, ALIGN_RIGHT);

        LCD_CMD(0x88);
        LCD_Write_String8(buf, 0, ALIGN_RIGHT); //empty 
        LCD_CMD(0xC8);
        print_temp(TEMP_ENGINE, ALIGN_RIGHT);

        // force faster temperature update
        if (temperature_conv_fl == 0 && timeout_temperature > FORCED_TIMEOUT_TEMPERATURE) {
            timeout_temperature = FORCED_TIMEOUT_TEMPERATURE;
        }
// show temp sensors config on ok key longpress
#if defined(DS18B20_CONFIG)
        if (key2_longpress != 0) {
            key2_longpress = 0;

            LCD_CMD(0x80);
            LCD_Write_String16(buf, strcpy2(buf, (char *) service_menu_str, TEMP_SENSOR_INDEX), ALIGN_LEFT);
            config_screen_temp_sensors();
            CLEAR_KEYS_STATE();
            screen_refresh = 1;
        }
#endif
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
        return (uint16_t) (services.mh.counter_rpm / 96000UL);
    } else {
        // time based motor hours
        return (uint16_t) (services.mh.counter / 1800UL);
    }
}

void screen_service_counters() {
    
    srv_t* srv;
    service_time_t s_time;
    unsigned short v;
    
    select_param(&service_param, 5);

    LCD_CMD(0x80);
    LCD_Write_String16(buf, strcpy2((char*)buf, (char *) &service_counters, service_param + 1), ALIGN_LEFT);

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
    LCD_Write_String8(buf, len, ALIGN_RIGHT);
    
    s_time = srv->time;

    LCD_CMD(0xC8);
    print_time_dmy(s_time.day, s_time.month, s_time.year);
    
    if (request_screen((char *) &reset_string) != 0) {
        get_ds_time(&time);
        if (service_param == 0 || service_param == 1) {
            services.mh.counter = 0;
            services.mh.counter_rpm = 0;
        }
        srv->counter = 0;
        srv->time.day = time.day;
        srv->time.month = time.month;
        srv->time.year = time.year;
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
    config.settings.word = edit_value_bits(config.settings.word, (char *) &settings_bits);
}

#ifdef MIN_SPEED_CONFIG
void config_screen_min_speed() {
    config.min_speed = edit_value_char(config.min_speed, CHAREDIT_MODE_KMH, 1, 10);
}
#endif

#if defined(DS18B20_CONFIG_EXT)

__bit config_temperature_conv_fl;

#ifdef DS18B20_CONFIG_EXT_SHOW_DEV            
__bit config_temperature_show_temp;
#endif

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

#ifdef DS18B20_CONFIG_EXT_SHOW_DEV            
    config_temperature_show_temp = 0;
#endif
    timeout_timer1 = 512;
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
                timeout_timer1 = 512;
            }
            
#ifdef DS18B20_CONFIG_EXT_SHOW_DEV            
            if (key1_press != 0 || key2_press != 0 || key3_press != 0) {
                if (key2_press == 0) {
                    timeout_timer2 = 0;
                    config_temperature_show_temp = 1;
                }
                CLEAR_KEYS_STATE();
                timeout_timer1 = 512;
            }

            if (timeout_timer2 == 0) {
                if (config_temperature_conv_fl != 0) {
                    config_temperature_show_temp = !config_temperature_show_temp;
                } else {
                    config_temperature_show_temp = 0;
                }
                timeout_timer2 = 100;
            }

            if (config_temperature_show_temp != 0 || num_devices == 1)
            {
#else
            if (config_temperature_conv_fl != 0)
            {
#endif
                temps[TEMP_CONFIG] = _temps[current_device];
                add_leading_symbols(&buf[0], ' ', print_temp(TEMP_CONFIG, ALIGN_LEFT), 4);
#ifdef DS18B20_CONFIG_EXT_SHOW_DEV            
            }
            else
            {
                uint8_t *ptr = (uint8_t *) &buf[0];
                buf[0] = ' ';
                buf[1] = '1' + current_device;
                buf[2] = '/';
                buf[3] = '0' + num_devices;
            }
#endif
            {
                LCD_CMD(0x8C);
                LCD_Write_String(buf, 4);
            }
#ifndef DS18B20_CONFIG_EXT_SHOW_DEV
            }
#endif
            llptrtohex((unsigned char*) &tbuf[current_device * 8], (unsigned char*) buf);

            len = strcpy2(&buf[12], (char *) &temp_sensors, _t_num[current_device] + 1);
            add_leading_symbols(&buf[12], ' ', len, 4);

            CLEAR_KEYS_STATE();
        }

        LCD_CMD(0xC0);
        LCD_Write_String16(buf, 16, ALIGN_LEFT);

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
    
    timeout_timer1 = 512;
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
            timeout_timer1 = 512;
        }

#if defined(DS18B20_CONFIG_SHOW_TEMP)        
        if (config_temperature_conv_fl != 0) {
            add_leading_symbols((char *) &buf, ' ', print_temp(TEMP_CONFIG, ALIGN_LEFT), 4);
            LCD_CMD(0x8C);
            LCD_Write_String((char *) &buf, 4);
        }
#endif
        llptrtohex((unsigned char*) tbuf, (unsigned char*) buf);
        len = strcpy2(&buf[12], (char *) &temp_sensors, t_num + 1);
        add_leading_symbols(&buf[12], ' ', len, 4);
        
        LCD_CMD(0xC0);
        LCD_Write_String16(buf, 16, ALIGN_LEFT);
        
        CLEAR_KEYS_STATE();
        while (screen_refresh == 0 && timeout_timer1 != 0);
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
    LCD_Write_String16(buf, 2 + strcpy2(&buf[2], (char *) &service_counters, c_sub_item + 1), ALIGN_LEFT);

    if (key2_press != 0) {
        CLEAR_KEYS_STATE();

        LCD_CMD(0x80);
        LCD_Write_String16(buf, strcpy2(buf, (char *) &service_counters, c_sub_item + 1), ALIGN_LEFT);
        LCD_CMD(0xC0);
        _memset(buf, ' ', 16);
        LCD_Write_String16(buf, 16, ALIGN_LEFT);

        if (c_sub_item == 0) {
            services.mh.limit = (unsigned short) edit_value_long(services.mh.limit, 1999L);
        } else {
            services.srv[c_sub_item - 1].limit = edit_value_char(services.srv[c_sub_item - 1].limit, CHAREDIT_MODE_10000KM, 0, 60);
        }

        timeout_timer1 = 512;
    }
    
}
#endif

void config_screen_ua_const() {
    handle_keys_up_down(&config.vcc_const, 140, 230);

    LCD_CMD(0xC4);
    print_voltage(ALIGN_RIGHT);
}

void config_screen_version() {
    LCD_CMD(0xC0);
    LCD_Write_String16(buf, strcpy2(buf, (char*) &version_str, 0), ALIGN_LEFT);
}

void config_screen(unsigned char c_item) {
    uint8_t force_item = (c_item & CONFIG_SCREEN_FORCE_ITEM) != 0;

    c_item &= CONFIG_SCREEN_MASK_ITEM; 
    config_screen_item_t item = items_service[c_item];
    
    if (!force_item) {
        LCD_CMD(0x80);
        LCD_Write_String16(buf, strcpy2(buf, (char *) &service_menu_title, 0), ALIGN_LEFT);

        LCD_CMD(0xC0);
        buf[0] = '1' + c_item;
        buf[1] = '.';
        LCD_Write_String16(buf, strcpy2(&buf[2], (char *) service_menu_str, item.str_index) + 2, ALIGN_LEFT);
    }

    if (key2_press != 0 || force_item) {
        key2_press = 0;
        LCD_Clear();
        timeout_timer1 = 512;
        while (timeout_timer1 != 0) {
            screen_refresh = 0;

            LCD_CMD(0x80);
            LCD_Write_String16(buf, strcpy2(buf, (char *) service_menu_str, item.str_index), ALIGN_LEFT);

            item.screen();

            CLEAR_KEYS_STATE();

            while (screen_refresh == 0 && timeout_timer1 != 0);
        }
        screen_refresh = 1;
    }
}

void read_eeprom() {
    unsigned char ee_addr = 0;

#if defined(__AVR) && defined(PROGMEM_EEPROM)
    // check eeprom special mark and save default eeprom content if mark not exists
    unsigned char tbuf[8];
    // checking key ok pressed for 1 sec for overwriting eeprom with defaults
    uint8_t c;
    while (KEY_OK_PRESSED && ++c < 25) {
        delay_ms(40);
    }
    if (c == 25) {
        _memset(tbuf, 0xFF, sizeof(tbuf));
    } else {
        HW_read_eeprom_block((unsigned char*) &tbuf, sizeof(eedata) - 8, 8);
    }       
    if (memcmp_P((unsigned char*) &tbuf, &eedata[sizeof(eedata) - 8], 8) != 0) {
        uint8_t c;
        for (c = 0; c < sizeof(eedata); c += 8) {
            memcpy_P(&tbuf, &eedata[c], 8);
            HW_write_eeprom_block((unsigned char*) &tbuf, c, 8);
        }
    }
#endif
    
    HW_read_eeprom_block((unsigned char*) &config, ee_addr, sizeof(config_t));
    ee_addr += ((sizeof(config_t) - 1) / 8 + 1) * 8;
    
    HW_read_eeprom_block((unsigned char*) &trips, ee_addr, sizeof(trips_t));
    ee_addr += ((sizeof(trips_t) - 1) / 8 + 1) * 8;

#ifdef SERVICE_COUNTERS_SUPPORT
    HW_read_eeprom_block((unsigned char*) &services, ee_addr, sizeof(services_t));
#endif
}

void save_eeprom() {
    unsigned char ee_addr = 0;
    HW_write_eeprom_block((unsigned char*) &config, ee_addr, sizeof(config_t));
    ee_addr += ((sizeof(config_t) - 1) / 8 + 1) * 8;
    
    HW_write_eeprom_block((unsigned char*) &trips, ee_addr, sizeof(trips_t));
    ee_addr += ((sizeof(trips_t) - 1) / 8 + 1) * 8;

#ifdef SERVICE_COUNTERS_SUPPORT
    HW_write_eeprom_block((unsigned char*) &services, ee_addr, sizeof(services_t));
#endif
}

void power_off() {
    LCD_Clear();
    // save and shutdown;
    disable_interrupts();
    
    // save current time
    if (save_tripc_time_fl != 0) {
        get_ds_time(&time);
        trips.tripC_time.minute = time.minute;
        trips.tripC_time.hour = time.hour;
        trips.tripC_time.day = time.day;
        trips.tripC_time.month = time.month;
        trips.tripC_time.year = time.year;
    }
    
    config.selected_param.main_param = main_param;
#ifdef SERVICE_COUNTERS_SUPPORT
    config.selected_param.service_param = service_param;
#endif
    
    save_eeprom();
    
    PWR_OFF; while (1);
}

const uint8_t ydayArray[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273 - 256, 304 - 256, 334 - 256, 365 - 256};

uint16_t get_yday(uint8_t month, uint8_t day) {
    unsigned char m = bcd8_to_bin(month) - 1;
    unsigned short yday = ydayArray[m];
    if (m >= 9) {
        yday += 256;
    }
    return yday + bcd8_to_bin(day);
}

unsigned char check_tripC_time() {
    // clear trip C if diff between dates is more than TRIPC_PAUSE_MINUTES minutes
    short diff;

    get_ds_time(&time);

#ifndef SIMPLE_TRIPC_TIME_CHECK    
    diff = bcd_subtract(time.year, trips.tripC_time.year);
    if (diff < 0 || diff > 1) return 1;

    unsigned short yday = get_yday(time.month, time.day);
    unsigned short yday_c = get_yday(trips.tripC_time.month, trips.tripC_time.day);
    
    diff = (short) ((diff == 1 ? 365 : 0) + yday - yday_c);
#else    
    diff = bcd_subtract(time.day,trips.tripC_time.day);
#endif
    
    if (diff < 0 || diff > 1) return 1;

    if (config.settings.daily_tripc == 0) {
        diff = (diff == 1 ? 24 : 0) + bcd_subtract(time.hour, trips.tripC_time.hour);
        if (diff < 0) return 1;

        diff = 60 * diff + bcd_subtract(time.minute, trips.tripC_time.minute);
        if (diff > TRIPC_PAUSE_MINUTES) return 1;
    } else {
       return (unsigned char) diff; 
    }
    
    return 0;
}

void set_consts() {
    // default const
    fuel1_const = 65;
    fuel2_const = 28 * 2;
    odo_con4 = config.odo_const * 2 / 13;
    // const for dual injection
    if (config.settings.par_injection != 0) {
        fuel1_const <<= 1; // 130
        fuel2_const >>= 1; // 14
        odo_con4 >>= 1; // (config.odo_const * 2 / 13) / 2
    }

    main_interval = MAIN_INTERVAL;

    main_param = config.selected_param.main_param;
#ifdef SERVICE_COUNTERS_SUPPORT
    service_param = config.selected_param.service_param;
#endif

#ifdef SIMPLE_ACCELERATION_MEASUREMENT
    accel_meas_upper_const = (unsigned short) (speed_const(100) / config.odo_const);
#endif
}

void power_on() {
    HW_Init();

    read_eeprom();
    
    LCD_Init();
    
    set_consts();
    
    if (check_tripC_time() != 0) {
        // clear tripC
        clear_trip(true, &trips.tripC);
        trips.tripC_max_speed = 0;
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
            buzzer_mode_value = BUZZER_WARN;
#endif
            LCD_CMD(0x80);
            LCD_Write_String16(buf, strcpy2(buf, (char*) &warning_str, 0), ALIGN_CENTER);

            LCD_CMD(0xC0);
            LCD_Write_String16(buf, strcpy2(buf, (char*) &service_counters, i + 1), ALIGN_CENTER);

            timeout_timer1 = 512;
            while (timeout_timer1 != 0 && NO_KEY_PRESSED)
                ;
            CLEAR_KEYS_STATE();
        }
        warn >>= 1;
    }
}
#endif

#if defined(TEMPERATURE_SUPPORT)

#if defined(DS18B20_TEMP)
uint8_t t_error[3] = {0, 0, 0};
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
                if (t_error[i] >= 5) { // max sequential crc errors
                    temps[i] = DS18B20_TEMP_NONE;
                } else {
                    t_error[i]++;
                }
            } else {
                temps[i] = _t;
                t_error[i] = 0;
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

void handle_misc_values() {
    speed = (unsigned short) ((36000UL * (unsigned long) kmh) / (unsigned long) config.odo_const);

    drive_min_speed_fl = speed >= config.min_speed * 10;

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
}

uint8_t c_item = 0;
uint8_t c_item_prev = 0;

void main() {
    static __bit config_mode;
    uint8_t prev_main_item = 0, prev_config_item = 0;
    uint8_t max_item = 0;

    power_on();
    
    start_timer1();
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
    
    CLEAR_KEYS_STATE();
    
    // wait first adc conversion
    while (adc_voltage == 0 && screen_refresh == 0)
        ;
    
    
    while (1) {
        screen_refresh = 0;

        // check power
        if (shutdown_counter == SHUTDOWN_COUNTER) {
            power_off();
        }

        if (key2_longpress != 0) {
            // long keypress for service key - switch service mode and main mode
            if (config_mode == 0 && motor_fl == 0 && drive_fl == 0 && c_item == 0) {
                prev_main_item  = c_item;
                c_item = prev_config_item;
                config_mode = 1;
                //LCD_Clear();
                CLEAR_KEYS_STATE();
            } else if (config_mode != 0) {
                prev_config_item = c_item;
                c_item = prev_main_item;
                config_mode = 0;
                //LCD_Clear();
                CLEAR_KEYS_STATE();
                // save config
                HW_write_eeprom_block((unsigned char*) &config, 0, sizeof(config_t));
                // set consts
                set_consts();
            }
        }
        
        if (config_mode == 0) {
            if (drive_min_speed_fl != 0) {
                max_item = DRIVE_MODE_MAX;
            } else {
                max_item = sizeof (items_main) / sizeof (screen_item_t) - 1;
            }
#ifdef TEMPERATURE_SUPPORT
            if (timeout_temperature == 0) {
                handle_temp();
            }
#endif            
            handle_misc_values();
        } else {
            max_item = sizeof (items_service) / sizeof (config_screen_item_t) - 1;
        }
        
        // show next/prev screen
        c_item_prev = c_item;
        handle_keys_next_prev(&c_item, 0, max_item);
        if (c_item_prev != c_item) {
            tmp_param = 0;
            CLEAR_KEYS_STATE();
        }
        
        if (config_mode != 0) {
            config_screen(c_item);
        } else {
            do {
                if (drive_min_speed_fl != 0 && c_item > DRIVE_MODE_MAX) {
                    c_item = 0;
                    //LCD_Clear();
                }
                item_skip = 0;
                items_main[c_item].screen();
                if (item_skip != 0) {
#ifdef KEY3_SUPPORT
                    if (c_item < c_item_prev) {
                        c_item--;
                    } else
#endif
                    {
                        c_item++;
                    }
                }
            } while (item_skip != 0);
        }
        
        while (screen_refresh == 0);
    }
}
