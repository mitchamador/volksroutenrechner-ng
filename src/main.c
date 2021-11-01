#include "main.h"
#include "locale.h"
#include "eeprom.h"
#include "lcd.h"
#include "ds1307.h"
#include "ds18b20.h"
#include "utils.h"
#include "i2c.h"
#include "version.h"
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>

// disable ds18b20 support with external defines
#ifndef NO_DS18B20
#define DS18B20_SUPPORT
#ifdef DS18B20_SUPPORT
#ifndef NO_DS18B20_CONFIG
#define DS18B20_CONFIG
#endif
#endif
#endif

// adc voltage filtering (power of 2)
//#define ADC_VOLTAGE_FILTER_VALUE 4

#ifdef HW_LEGACY
// simple checking time difference (decrease memory usage)
//#define SIMPLE_TRIPC_TIME_CHECK
#endif

// alt tripc screen
#define ALT_TRIPC_SCREEN

// auto calculate day of week
#define AUTO_DAY_OF_WEEK

// power supply threshold 
// with default divider resistor's (8,2k (to Vcc) + 3,6k (to GND)) values
// THRESHOLD_VOLTAGE * (3,6 / (3,6 + 8,2)) * (1024 / 5) = THRESHOLD_VOLTAGE_ADC_VALUE
// 2,048V ~ 128
#define THRESHOLD_VOLTAGE_ADC_VALUE 128
// threshold for buttons +-0,1v
#define ADC_BUTTONS_THRESHOLD 100
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

// fix taho weird values (maybe hw fix needed)
// possible fuel injector oscillogram
//  |-------|  |-|                           |-------|
//  |       |  | |                           |       |
// _|       |__| |___________________________|       |__
//  injector  strange
//    pulse    pulse
// seems to be fixed with timer overflow checking
//#define TAHO_RPM_FIX_VALUE 7500UL
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

// temperature timeout
#define TIMEOUT_TEMPERATURE (15 - 1)

config_t config;
trips_t trips;
services_t services;

//========================================================================================

#ifdef HW_LEGACY

#define KEY_SERVICE_PRESSED KEY1_PRESSED
#define KEY_SERVICE_LONGPRESSED (key1_longpress != 0)
#define NO_KEY_PRESSED (key1_press == 0 && key2_press == 0)
#define CLEAR_KEYS_STATE() key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0

#define KEY_NEXT (key1_press != 0)
#define KEY_CHANGEVALUE (key2_press != 0)
#define KEY_PREV (0)

#else

uint8_t _adc_ch;
uint16_t adc_key;
volatile uint8_t key_pressed;

uint8_t key3_counter;
volatile __bit key3_press;
signed char c_item_dir;

#define KEY1_PRESSED (key_pressed == 1)
#define KEY2_PRESSED (key_pressed == 2)
#define KEY3_PRESSED (key_pressed == 3)

#define KEY_SERVICE_PRESSED KEY2_PRESSED
#define KEY_SERVICE_LONGPRESSED (key2_longpress != 0)
#define NO_KEY_PRESSED (key1_press == 0 && key2_press == 0 && key3_press == 0)
#define CLEAR_KEYS_STATE() key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0; key3_press = 0

#define KEY_NEXT ((key1_press != 0 && config.settings.alt_buttons == 0) || (key2_press != 0 && config.settings.alt_buttons != 0))
#define KEY_CHANGEVALUE ((key2_press != 0 && config.settings.alt_buttons == 0) || (key1_press != 0 && config.settings.alt_buttons != 0) || (key3_press != 0 && config.settings.alt_buttons != 0))
#define KEY_PREV (key3_press != 0 && config.settings.alt_buttons == 0)

#endif

// key variables and flags
uint8_t key1_counter, key2_counter, shutdown_counter;
volatile __bit key1_press, key2_press, key1_longpress, key2_longpress;

// 0.1s flag and counter
uint8_t counter_01sec;
volatile __bit counter_01sec_fl;

// main interval
uint8_t main_interval_counter;
volatile uint8_t main_interval;
volatile __bit screen_refresh;
__bit time_increase_fl;

// timeout
volatile uint16_t timeout_timer;
volatile __bit timeout;

#define start_timeout(timer) timeout = 0; timeout_timer = timer;

#ifdef DS18B20_SUPPORT
volatile uint8_t timeout_temperature;
volatile __bit temperature_fl, temperature_conv_fl;
#endif

// misc flags
volatile __bit odom_fl, drive_fl, motor_fl, fuel_fl, taho_fl, taho_measure_fl, shutdown_fl, _timer1_overflow, drive_min_speed_fl;

// speed100 flags and variables
volatile __bit speed100_measure_fl, speed100_ok_fl, speed100_timer_fl, speed100_fl, speed100_final_fl, _speed100_exit;
volatile uint16_t speed100_const, speed100_timer, speed100;
uint16_t speed100_tmr1_prev, speed100_tmr1;
uint8_t speed100_tmr1_ofl;

uint16_t kmh_tmp, fuel_tmp;
volatile uint16_t taho_tmr1_prev;
uint16_t _timer1;
volatile uint8_t taho_tmr1_ofl;
volatile uint16_t kmh, fuel, adc_voltage;

#ifdef TAHO_RPM_FIX_VALUE
uint16_t taho_rpm_fix_const;
#endif
volatile uint24_t taho, _taho;
volatile uint24_t taho_tmr1;

uint16_t adc_tmp0, adc_tmp1;

volatile __bit save_tripc_time_fl = 0;
uint16_t speed;

uint8_t mh_rpm_const;

#ifdef DS18B20_SUPPORT
// ds18b20 temperatures and eeprom pointer
#define TEMP_OUT 0
#define TEMP_IN 1
#define TEMP_ENGINE 2
uint16_t _t;
uint16_t temps[3] = {0, 0, 0};
#endif

__bit buzzer_fl, buzzer_init_fl, buzzer_snd_fl, buzzer_repeat_fl;
uint8_t buzzer_counter_r;
uint8_t buzzer_counter;

typedef struct {
    uint8_t counter; // number of repeats
    uint8_t sound;   // sound on duration  (*0.1ms)
    uint8_t pause;   // sound off duration (*0.1ms)
} buzzer_t;

buzzer_t *buzzer_mode;

#define BUZZER_KEY 0
#define BUZZER_LONGKEY 1
#define BUZZER_WARN 2
#define BUZZER_NONE -1

buzzer_t buzzer[3] = {
    {1,1,1}, // BUZZER_KEY
    {1,4,1}, // BUZZER_LONG_KEY
    {3,3,2}  // BUZZER_WARN
};

uint8_t tmp_param = 0, service_param = 0;

// buffer for strings
char buf[16];
//char buf2[16];
uint8_t len;

__bank2 ds_time time;

uint8_t fuel1_const;
uint8_t fuel2_const;
uint16_t odo_con4;

void screen_main(void);
void screen_tripC(void);
void screen_temp(void);
void screen_max(void);
void screen_tripA(void);
void screen_tripB(void);
void screen_time(void);
void screen_service_counters(void);

// max screen in drive mode
#define DRIVE_MODE_MAX 2

const screen_item_t items_main[] = {
    {screen_main},
    {screen_tripC},
    {screen_temp},
    {screen_tripA},
    {screen_tripB},
    {screen_time},
    {screen_service_counters},
};

void service_screen_fuel_constant(void);
void service_screen_vss_constant(void);
void service_screen_total_trip(void);
void service_screen_settings_bits(void);
void service_screen_temp_sensors(void);
void service_screen_service_counters(void);
void service_screen_ua_const(void);
void service_screen_min_speed(void);
void service_screen_version(void);

const service_screen_item_t items_service[] = {
    {FUEL_CONSTANT_INDEX, service_screen_fuel_constant},
    {VSS_CONSTANT_INDEX, service_screen_vss_constant},
    {TOTAL_TRIP_INDEX, service_screen_total_trip},
    {VOLTAGE_ADJUST_INDEX, service_screen_ua_const},
    {SETTINGS_BITS_INDEX, service_screen_settings_bits},
    {MIN_SPEED_INDEX, service_screen_min_speed},
#if defined(DS18B20_SUPPORT) && defined(DS18B20_CONFIG)
    {TEMP_SENSOR_INDEX, service_screen_temp_sensors},
#endif
    {SERVICE_COUNTERS_INDEX, service_screen_service_counters},
    {VERSION_INFO_INDEX, service_screen_version},
};

uint8_t c_item = 0, c_sub_item = 0;
uint8_t prev_main_item = 0, prev_service_item = 0;
uint8_t service_mode = 0;
uint8_t max_item = 0;

uint8_t request_screen(char *);

int8_t key_sound = BUZZER_NONE;

#ifndef HW_LEGACY
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

// A valid CW or CCW move returns 1, invalid returns 0.
void read_rotary() {
  static int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
  static uint8_t prevNextCode = 0;
  static uint8_t store = 0;

  prevNextCode <<= 2;
  if (ENCODER_DATA) prevNextCode |= 0x02;
  if (ENCODER_CLK) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

  int8_t encoder_key = 0;
  // If valid then store as 16 bit data.
  if  (rot_enc_table[prevNextCode] ) {
    store <<= 4;
    store |= prevNextCode;
    if (store == 0x2b) {
        encoder_key = -1;
    }
    if (store == 0x17) {
        encoder_key = 1;
    }
  }

  if (encoder_key != 0) {
    if (encoder_key == 1) {
        key1_press = 1;
    } else if (encoder_key == -1) {
        key3_press = 1;
    }
    screen_refresh = 1;
    key_sound = BUZZER_KEY;
  }
}

#endif

int_handler_GLOBAL_begin

#if !defined(HW_LEGACY) && defined(int_handler_encoder_begin)
    int_handler_encoder_begin
        // handle encoder
        read_rotary();
    int_handler_encoder_end
#endif

    int_handler_fuel_speed_begin

        // capture 0.01s timer value
        /*uint16_t*/ _timer1 = get_timer1();
        if (timer1_overflow()) {
            _timer1_overflow = 1;
        }

#if !defined(HW_LEGACY) && !defined(int_handler_encoder_begin)
        // handle encoder
        read_rotary();
#endif
        // fuel injector
        if (FUEL_ACTIVE) {
            if (fuel_fl == 0) {
                // start timer0
                start_timer_fuel();
                fuel_fl = 1;
                motor_fl = 1;
                save_tripc_time_fl = 1;
                
                services.mh.counter_rpm += mh_rpm_const;

// new taho calculation based on captured value of 0.01s timer
                if (taho_measure_fl == 0) {
                    taho_measure_fl = 1;

                    taho_tmr1 = 0;
                    taho_tmr1_prev = _timer1;
                    taho_tmr1_ofl = 0;
                } else {
                    _taho = taho_tmr1 + _timer1 - taho_tmr1_prev;
                    if (_timer1_overflow != 0) {
                        _taho += TIMER1_VALUE;
                    }
#ifdef TAHO_RPM_FIX_VALUE
                    if (_taho >= taho_rpm_fix_const)
#endif
                    {
                        taho = _taho;
                        taho_fl = 1;

                        taho_tmr1 = 0;
                        taho_tmr1_prev = _timer1;
                        taho_tmr1_ofl = 0;
                    }
                }
            }
        } else {
            if (fuel_fl != 0) {
                // stop timer0
                stop_timer_fuel();
                fuel_fl = 0;
            }
        }
        
        // speed sensor
        if (TX_ACTIVE) {
            if (odom_fl == 0) {
                odom_fl = 1;
                drive_fl = 1;
                
                // new speed 100 calculation based on captured value of 0.01s timer
                if (speed100_timer_fl != 0) {
                    if (speed100_measure_fl == 0) {
                        speed100_measure_fl = 1;
                        speed100_tmr1_prev = _timer1;
                        speed100_tmr1 = 0;
                        speed100_tmr1_ofl = 0;
                    } else {
                        speed100 = speed100_tmr1 + _timer1 - speed100_tmr1_prev;
                        if (_timer1_overflow != 0) {
                            speed100 += TIMER1_VALUE;
                        }
                        speed100_fl = 1;
                        if (speed100 <= speed100_const) {
                            speed100_ok_fl = 1;
                            speed100_timer_fl = 0;
                        } else {
                            speed100_tmr1_prev = _timer1;
                            speed100_tmr1 = 0;
                            speed100_tmr1_ofl = 0;
                        }
                    }
                } else {
                    speed100_measure_fl = 0;
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

#ifdef HW_LEGACY                
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
#else
                inc_odo(&trips.tripA);
                inc_odo(&trips.tripB);
                inc_odo(&trips.tripC);
#endif
            }
        } else {
            odom_fl = 0;
        }
    int_handler_fuel_speed_end
        
    int_handler_timer0_begin
            
        fuel_tmp++;

#ifdef HW_LEGACY
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
#else
        inc_fuel(&trips.tripA);
        inc_fuel(&trips.tripB);
        inc_fuel(&trips.tripC);
#endif
        
    int_handler_timer0_end

    int_handler_timer1_begin

        if (_timer1_overflow == 0) {
            if (taho_measure_fl != 0) {
                if (++taho_tmr1_ofl == TAHO_OVERFLOW) {
                    taho_measure_fl = 0;
                    stop_timer_fuel();
                    taho_fl = 0;
                    motor_fl = 0;
                } else {
                    taho_tmr1 += TIMER1_VALUE;
                }
            }

            if (speed100_measure_fl != 0) {
                if (++speed100_tmr1_ofl == SPEED100_OVERFLOW) {
                    speed100_measure_fl = 0;
                    speed100_fl = 0;
                } else {
                    speed100_tmr1 += TIMER1_VALUE;
                }
            }
        } else {
            _timer1_overflow = 0;
        }

        if (KEY1_PRESSED) // key pressed
        {
            if (key1_counter <= LONGKEY) {
                key1_counter++;
            }
            if (key1_counter == LONGKEY) {
                // long keypress
                key1_longpress = 1;
                screen_refresh = 1;
                key_sound = BUZZER_LONGKEY;
            }
        } else // key released
        {
            if (key1_counter > DEBOUNCE && key1_counter <= SHORTKEY) {
                // key press
                key1_press = 1;
                screen_refresh = 1;
                key_sound = BUZZER_KEY;
            }
            key1_counter = 0;
        }
          
        if (KEY2_PRESSED) // key pressed
        {
            if (key2_counter <= LONGKEY) {
                key2_counter++;
            }
            if (key2_counter == LONGKEY) {
                // long keypress
                key2_longpress = 1;
                screen_refresh = 1;
                key_sound = BUZZER_LONGKEY;
            }
        } else // key released
        {
            if (key2_counter > DEBOUNCE && key2_counter <= SHORTKEY) {
                // key press
                key2_press = 1;
                screen_refresh = 1;
                key_sound = BUZZER_KEY;
            }
            key2_counter = 0;
        }
    
#ifndef HW_LEGACY
        if (KEY3_PRESSED) // key pressed
        {
            if (key3_counter <= SHORTKEY) {
                key3_counter++;
            }
        } else // key released
        {
            if (key3_counter > DEBOUNCE && key3_counter <= SHORTKEY) {
                // key press
                key3_press = 1;
                screen_refresh = 1;
                key_sound = BUZZER_KEY;
            }
            key3_counter = 0;
        }
#endif    

        if (key_sound != BUZZER_NONE) {
            if (config.settings.key_sound) {
                buzzer_fl = 1; buzzer_init_fl = 0; buzzer_mode = &buzzer[key_sound];
            }
            key_sound = BUZZER_NONE;
        }
    
        if (++main_interval_counter >= main_interval) {
            main_interval_counter = 0;

            // screen refresh_flag
            screen_refresh = 1;
             
            // copy temp interval variables to main
            fuel = fuel_tmp;
            fuel_tmp = 0;
            kmh = kmh_tmp;
            kmh_tmp = 0;
             
            // if fast refresh enabled increase time counter every second times
            if (config.settings.fast_refresh == 0 || time_increase_fl != 0) {
                time_increase_fl = 0;
                // increase time counters
                if (motor_fl != 0 || drive_fl != 0) {
                     services.mh.counter++;
                     trips.tripA.time++;
                     trips.tripB.time++;
                     trips.tripC.time++;
                }

#ifdef DS18B20_SUPPORT
                if (timeout_temperature > 0) {
                   if (--timeout_temperature == 0) {
                       temperature_fl = 1;
                   }
                }
#endif                
            } else {
                time_increase_fl = 1; 
            }
             
        }
        
        if (timeout_timer > 0) {
            if (--timeout_timer == 0) {
                timeout = 1;
            }
        }

        if (speed100_timer_fl != 0) {
            speed100_timer++;
        }

        if (counter_01sec == 0) {
            counter_01sec_fl = 1;
            counter_01sec = 10;
            if (buzzer_fl != 0) {
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
        } else {
            counter_01sec--;
        }
    int_handler_timer1_end

    int_handler_adc_begin

#ifndef HW_LEGACY
        if (_adc_ch == 0) {
#endif

#ifdef ADC_VOLTAGE_FILTER_VALUE            
            adc_tmp0 = adc_tmp0 - ((adc_tmp0 + ADC_VOLTAGE_FILTER_VALUE/2) / ADC_VOLTAGE_FILTER_VALUE) + adc_read_value();
            adc_voltage = ((adc_tmp0 + ADC_VOLTAGE_FILTER_VALUE/2) / ADC_VOLTAGE_FILTER_VALUE);
#else
            adc_voltage = adc_read_value();
#endif

#ifdef HW_LEGACY
            PWR_ON;
#endif    
            // read power supply status
            if (adc_voltage > THRESHOLD_VOLTAGE_ADC_VALUE) {
                shutdown_counter = 0;
            } else {
                if (shutdown_counter == 8) {
                    shutdown_fl = 1; screen_refresh = 1;
                } else {
                    shutdown_counter++;
                }
            }
#ifndef HW_LEGACY
            _adc_ch = 1;
            set_adc_channel(ADC_CHANNEL_BUTTONS);
        } else {
            adc_key = adc_read_value();
            if (/*   adc_key >= (ADC_BUTTONS_1V * 0 - ADC_BUTTONS_THRESHOLD) && */adc_key <= (ADC_BUTTONS_1V * 0 + ADC_BUTTONS_THRESHOLD)) {
                key_pressed = 2;
            } else if (adc_key >= (ADC_BUTTONS_1V * 1 - ADC_BUTTONS_THRESHOLD) && adc_key <= (ADC_BUTTONS_1V * 1 + ADC_BUTTONS_THRESHOLD)) {
                key_pressed = 1;
            } else if (adc_key >= (ADC_BUTTONS_1V * 2 - ADC_BUTTONS_THRESHOLD) && adc_key <= (ADC_BUTTONS_1V * 2 + ADC_BUTTONS_THRESHOLD)) {
                key_pressed = 3;
            } else {
                key_pressed = 0;
            }
            _adc_ch = 0;
            set_adc_channel(ADC_CHANNEL_POWER_SUPPLY);
        }
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
    LCD_Write_String8(buf, 8, LCD_ALIGN_LEFT);

}

void print_time_dow(unsigned char day_of_week) {
    LCD_Write_String16(buf, strcpy2((char *)buf, (char*) day_of_week_str, day_of_week), LCD_ALIGN_LEFT);
}

void print_time(ds_time* time) {
    LCD_CMD(0x80);
    print_time_hm(time->hour, time->minute, LCD_ALIGN_LEFT);

    LCD_CMD(0x88);
    print_time_dmy(time->day, time->month, time->year);

    LCD_CMD(0xc0);
    print_time_dow(time->day_of_week);
}

void screen_time(void) {

    get_ds_time(&time);
    if (request_screen((char *) &time_correction) != 0) {

        unsigned char c = 0;
        const char cursor_position[] = {0x81, 0x84, 0x89, 0x8c, 0x8f, 0xc0};

        LCD_Clear();
        start_timeout(500);
        while (timeout == 0) {
            screen_refresh = 0;
            if (KEY_NEXT) {
                // edit next element
                c++;
                if (c == 6) {
                    c = 0;
                }
                start_timeout(500);
            }
#ifndef HW_LEGACY
            if (KEY_PREV) {
                // edit previous element
                if (c == 0) {
                    c = 5;
                } else {
                    c--;
                }
                start_timeout(500);
            }
#endif            
            if (KEY_CHANGEVALUE) {
#ifdef HW_LEGACY
#define BCD8_INCDEC(value, dummy, min, max) bcd8_inc(value, min, max)
#else
                dir_t dir = key3_press != 0 ? FORWARD : BACKWARD;
#define BCD8_INCDEC(value, incdec, min, max) bcd8_incdec(value, incdec, min, max)
#endif                
                // increment/decrement current element
                switch (c) {
                    case 0:
                        time.hour = BCD8_INCDEC(time.hour, dir, 0, 23);
                        break;
                    case 1:
                        time.minute = BCD8_INCDEC(time.minute, dir, 0, 59);
                        break;
                    case 2:
                        time.day = BCD8_INCDEC(time.day, dir, 1, 31);
                        break;
                    case 3:
                        time.month = BCD8_INCDEC(time.month, dir, 1, 12);
                        break;
                    case 4:
                        time.year = BCD8_INCDEC(time.year, dir, VERSION_YEAR, VERSION_YEAR + 5);
                        break;
                    case 5:
                        time.day_of_week = BCD8_INCDEC(time.day_of_week, dir, 1, 7);
                        break;
                }
                
#ifdef AUTO_DAY_OF_WEEK
                if (c == 2 || c == 3 || c == 4) {
                    set_day_of_week(&time);
                }
#endif                
                start_timeout(500);
            }
            
            CLEAR_KEYS_STATE();

            LCD_CMD(LCD_CURSOR_OFF);
            print_time(&time);
            LCD_CMD(cursor_position[c]);
            LCD_CMD(LCD_BLINK_CURSOR_ON);

            while (screen_refresh == 0 && timeout == 0);
        }
        LCD_CMD(LCD_CURSOR_OFF);
        // save time
        set_ds_time(&time);
        key2_longpress = 0;

    } else {
        print_time(&time);
    }
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
 * print fractional number [num/10].[num%10]
 * @param buf
 * @param num
 * @return 
 */
unsigned char print_fract(char * buf, uint16_t num) {
    unsigned char len = ultoa2(buf, num, 10);

    if (num < 10) {
        buf[1] = buf[0];
        buf[0] = '0';
        len++;
    }
    
    //buf[len + 1] = 0;
    buf[len] = buf[len - 1];
    buf[len - 1] = '.';
    return len + 1;
}
typedef enum {
    CHAREDIT_MODE_NONE = POS_NONE,
    CHAREDIT_MODE_KMH = POS_KMH,
    CHAREDIT_MODE_10000KM = POS_KM
} edit_value_char_t;

unsigned char edit_value_char(unsigned char v, edit_value_char_t mode, unsigned char min_value, unsigned char max_value) {
    start_timeout(300);
    while (timeout == 0) {
        screen_refresh = 0;

        if (key1_press != 0) {
            if (v++ == max_value) {
                v = min_value;
            }
            start_timeout(300);
        }
        if (key2_press != 0) {
#ifndef HW_LEGACY
            timeout = 1;
        }
        if (key3_press != 0) {
#endif            
            if (v-- == min_value) {
                v = max_value;
            }
            start_timeout(300);
        }

        len = ultoa2(buf, (unsigned long) (mode == CHAREDIT_MODE_10000KM ? (v * 1000L) : v), 10);

        len += print_symbols_str(len, (unsigned char) mode);

        LCD_CMD(0xC4);
        LCD_Write_String8(buf, len, LCD_ALIGN_RIGHT);

        CLEAR_KEYS_STATE();

        while (screen_refresh == 0 && timeout == 0);
    }

    screen_refresh = 1;

    return v;
}

unsigned long edit_value_long(unsigned long v, unsigned long max_value) {
    // number of symbols to edit
    unsigned char max_len = ultoa2(buf, max_value, 10);
#ifndef HW_LEGACY        
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

    start_timeout(300);
    while (timeout == 0) {
        screen_refresh = 0;

        // change cursor to next position
        if (KEY_NEXT) {
            pos++;
            if (pos == max_len) {
                pos = 0;
            }
            start_timeout(300);
        }

#ifndef HW_LEGACY        
        // change cursor to prev position
        if (KEY_PREV) {
            if (pos == 0) {
                pos = max_len - 1;
            } else {
                pos--;
            }
            start_timeout(300);
        }
#endif
        // edit number in cursor position
        if (KEY_CHANGEVALUE) {
#ifndef HW_LEGACY
            if (key3_press != 0) {
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

            start_timeout(300);
        }

        LCD_CMD(LCD_CURSOR_OFF);

        LCD_CMD(cursor_pos);
        LCD_Write_String8(buf, max_len, LCD_ALIGN_LEFT);
        LCD_CMD(cursor_pos + pos);

        LCD_CMD(LCD_BLINK_CURSOR_ON);

        CLEAR_KEYS_STATE();

        while (screen_refresh == 0 && timeout == 0);
    }

    LCD_CMD(LCD_CURSOR_OFF);
    screen_refresh = 1;

    return strtoul2(buf);
}

unsigned char edit_value_bits(unsigned char v, char* str) {

#define cursor_pos 0xC4

    unsigned char pos = 0;

    start_timeout(300);
    while (timeout == 0) {
        screen_refresh = 0;

        // change cursor to next position
        if (key1_press != 0) {
            if (++pos == 8) {
                pos = 0;
            }
            start_timeout(300);
        }

#ifndef HW_LEGACY
        // change cursor to prev position
        if (key3_press != 0) {
            if (pos == 0) {
                pos = 7;
            } else {
                pos--;
            }
            start_timeout(300);
        }
#endif        
        // edit number in cursor position
        if (key2_press != 0) {
            v ^= (1 << (7 - pos));
            start_timeout(300);
        }

        LCD_CMD(LCD_CURSOR_OFF);

        add_leading_symbols(buf, '0', ultoa2(buf, v, 2), 8);

        unsigned char _onoff = (buf[pos] - '0') + 1;

        LCD_CMD(cursor_pos);
        LCD_Write_String8(buf, 8, LCD_ALIGN_LEFT);

#if 0
        LCD_CMD(0x80);
        LCD_Write_String16(buf, strcpy2((char*) buf, (char *) str, pos + 1), LCD_ALIGN_LEFT);
        // print on/off
        LCD_CMD(0x8C);
        LCD_Write_String8(buf, strcpy2((char*) buf, (char *) &onoff_string, _onoff), LCD_ALIGN_LEFT);
#else
        memset(buf, ' ', 16);
        len = strcpy2((char*) buf, (char *) str, pos + 1);
        if (len != 0) {
            // print on/off
            strcpy2((char*) &buf[12], (char *) &onoff_string, _onoff);
        }
        LCD_CMD(0x80);
        LCD_Write_String16(buf, 16, LCD_ALIGN_LEFT);
#endif

        LCD_CMD(cursor_pos + pos);
        LCD_CMD(LCD_BLINK_CURSOR_ON);

        CLEAR_KEYS_STATE();

        while (screen_refresh == 0 && timeout == 0);
    }

    LCD_CMD(LCD_CURSOR_OFF);
    screen_refresh = 1;

    return v;
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

#if 0
    uint8_t _min = time % 60;
    if (_min < 10) {
        buf[len++] = '0';
    }
    len += ultoa2(&buf[len], (unsigned short) _min, 10);
#else
    bcd8_to_str(&buf[len], bin8_to_bcd(time % 60));
    len += 2;
#endif
   
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
        len = print_fract(buf, average_speed);
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
    len = print_fract(buf, odo);

    _print(len, POS_KM, align);
}

/**
 * show trip total fuel consumption (l)
 * @param t
 * @param align
 */
void print_trip_total_fuel(trip_t* t, align_t align) {
    len = print_fract(buf, t->fuel / 10);

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
        len = print_fract(buf, (uint16_t) (t->fuel * 100UL / odo));
    }

    _print(len, POS_LKM, align);
}

void print_speed(unsigned short speed, unsigned short i, align_t align) {
    // use fractional by default
    len = print_fract(&buf[i], speed);

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
        unsigned short res = (unsigned short) (((config.settings.dual_injection != 0 ? (TAHO_CONST) : (TAHO_CONST*2)) / taho));
#ifdef TAHO_ROUND
        res = (res + TAHO_ROUND / 2) / TAHO_ROUND * TAHO_ROUND;        
#endif
        len = ultoa2(buf, res, 10);
    }

    _print(len, POS_OMIN, align);
}

void print_current_fuel_km(align_t align) {
    unsigned short t = (unsigned short) ((((unsigned long) fuel * (unsigned long) odo_con4) / (unsigned long) kmh) / (unsigned char) config.fuel_const);
    len = print_fract(buf, t);

    _print(len, POS_LKM, align);
}

void print_current_fuel_lh(align_t align) {
    unsigned short t = (unsigned short) (((unsigned long) fuel * (unsigned long) fuel2_const / (unsigned long) config.fuel_const) / 10UL);
    len = print_fract(buf, t);

    _print(len, POS_LH, align);
}

void print_main_odo(align_t align) {
    len = ultoa2(buf, (unsigned long) config.odo, 10);

    _print(len, POS_KM, align);
}

#ifdef DS18B20_SUPPORT

void print_temp(unsigned char index, bool header, align_t align) {
    _t = temps[index];
    if (_t & 0x8000) // if the temperature is negative
    {
        buf[0] = '-'; // put minus sign (-)
        _t = (~_t) + 1; // change temperature value to positive form
    } else {
        buf[0] = '+';
    }
    _t = (unsigned short) ((_t >> 4) * 10 + (((_t & 0x000F) * 10) >> 4));

    if (index == 2) {
        len = ultoa2(buf, _t / 10, 10);
        len += print_symbols_str(len, POS_CELS);
    } else {
        len = print_fract(&buf[1], _t) + 1;
        if (header) {
            add_leading_symbols(buf, ' ', len, 8);
            strcpy2(buf, (char *) &temp_sensors, (index + 1) + 1);
            len = 8;
        }
    }
    LCD_Write_String8(buf, len, align);
}
#else
#define print_temp(index, header, align) LCD_Write_String8(buf, strcpy2(buf, (char *) &empty_string, 0), align)
#endif

void print_voltage(align_t align) {
    len = print_fract(buf, (unsigned short) (adc_voltage << 5) / config.vcc_const);

    _print(len, POS_VOLT, align);
}

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

unsigned char request_screen(char* request_str) {
    unsigned char res = 0;
    if (key2_longpress != 0) {
        CLEAR_KEYS_STATE();

        LCD_Clear();
        LCD_CMD(0x80);
        LCD_Write_String16(buf, strcpy2(buf, request_str, 0), LCD_ALIGN_CENTER);

        start_timeout(512);
        while (timeout == 0 && NO_KEY_PRESSED);

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

void print_selected_param1(align_t align) {
    switch (select_param(&config.selected_param1, 6)) {
        case 0:
            print_temp(TEMP_OUT, false, align);
            break;
        case 1:
            print_voltage(align);
            break;
        case 2:
            print_trip_odometer(&trips.tripC, align);
            break;
        case 3:
            print_trip_average_fuel(&trips.tripC, align);
            break;
        case 4:
            print_trip_average_speed(&trips.tripC, align);
            break;
        case 5:
            print_speed(trips.tripC_max_speed, print_symbols_str(0, POS_MAXS), align);
            break;
    }
}

#ifndef ALT_TRIPC_SCREEN
void print_selected_param2(align_t align) {
    switch (select_param(&config.selected_param2, 5)) {
        case 0:
            print_trip_average_fuel(&trips.tripC, align);
            break;
        case 1:
            print_trip_total_fuel(&trips.tripC, align);
            break;
        case 2:
            print_trip_average_speed(&trips.tripC, align);
            break;
        case 3:
            print_trip_time(&trips.tripC, align);
            break;
        case 4:
            print_speed(trips.tripC_max_speed, print_symbols_str(0, POS_MAXS), align);
            break;
    }
}
#endif

void speed100_measurement(void) {
    LCD_CMD(0x80);
    LCD_Write_String16(buf, strcpy2(buf, (char *) &speed100_wait_string, 0), LCD_ALIGN_CENTER);

    speed100_measure_fl = 0; speed100_ok_fl = 0; speed100_timer = 0; speed100_final_fl = 0; _speed100_exit = 0;

    memset(buf, '=', 16);

    // 15 sec waiting for start
    start_timeout(1536);

    counter_01sec_fl = 0;
    while (_speed100_exit == 0 && NO_KEY_PRESSED) {
        if (timeout == 0 && drive_fl == 0) {
            if (counter_01sec_fl != 0) {
                counter_01sec_fl = 0;
                LCD_CMD(0xC0);
                LCD_Write_String16(buf, (unsigned char) (timeout_timer / 91), LCD_ALIGN_LEFT);
            }
        } else {
            if (timeout == 0) {
                if (speed100_ok_fl == 0 && speed100_timer_fl == 0) {
                    // 30 sec for acceleration measurement
                    timeout_timer = 3072;
                    speed100_timer_fl = 1;
                }

                len = print_fract((char*) buf, speed100_timer / 10U);
                len += print_symbols_str(len, POS_SEC);

#ifdef PRINT_SPEED100                
                add_leading_symbols(buf, ' ', len, 16);
                if (speed100_fl != 0) {
                    // print current speed 
                    //len = print_fract((char*) buf, (uint16_t) ((360000UL * TIMER1_VALUE / config.odo_const) * 10UL / speed100)); // with fraction
                    len = ultoa2((char*) buf, (uint16_t) ((360000UL * TIMER1_VALUE / config.odo_const) / speed100), 10); // integer
                } else {
                    // no speed
                    len = strcpy2(buf, (char *) &empty_string, 0);
                }
                print_symbols_str(len, POS_KMH);
                len = 16;
#endif
                LCD_CMD(0xC0);
                LCD_Write_String16(buf, len, LCD_ALIGN_CENTER);
            }

            if (timeout == 0 && speed100_ok_fl == 0) {
                counter_01sec_fl = 0; while (counter_01sec_fl == 0);
            } else {
                if (speed100_ok_fl != 0 && speed100_final_fl == 0) {
                    // print final result
                    speed100_final_fl = 1;
                } else {
                    // timeout or 100 km/h
                    speed100_timer_fl = 0;
                    if (timeout == 1) {
                        // timeout
                        LCD_CMD(0xC0);
                        LCD_Write_String16(buf, strcpy2(buf, (char *) &timeout_string, 0), LCD_ALIGN_CENTER);
                    }
                    buzzer_fl = 1; buzzer_init_fl = 0; buzzer_mode = &buzzer[BUZZER_WARN];
                    start_timeout(512); while (timeout == 0 && NO_KEY_PRESSED);
                    _speed100_exit = 1;
                }
            }
        }
    }
    speed100_timer_fl = 0;
    CLEAR_KEYS_STATE();
}

void screen_main(void) {
//; первый экран

    LCD_CMD(0x80);
    if (drive_min_speed_fl == 0) {
//; 1) на месте с заглушенным двигателем
//; время текущее       общий пробег (км)
//; нар.темп./пробег C  вольтметр
        get_ds_time(&time);
        print_time_hm(time.hour, time.minute, LCD_ALIGN_LEFT);

        if (motor_fl == 0) {
            print_main_odo(LCD_ALIGN_RIGHT);

            LCD_CMD(0xC0);
            print_temp(TEMP_OUT, false, LCD_ALIGN_LEFT);
            print_voltage(LCD_ALIGN_RIGHT);
        } else {
//; 2) на месте с работающим двигателем
//; время текущее       тахометр (об/мин)
//; selected_param1 	мгновенный расход (л/ч)
            print_taho(LCD_ALIGN_RIGHT);
            LCD_CMD(0xC0);
            print_selected_param1(LCD_ALIGN_LEFT);
            print_current_fuel_lh(LCD_ALIGN_RIGHT);
        }
    } else {
//; 3) в движении
//; скорость (км/ч)     тахометр (об/мин)
//; selected_param1 	мгновенный расход (л/100км)
        print_speed(speed, 0, LCD_ALIGN_LEFT);
        print_taho(LCD_ALIGN_RIGHT);

        LCD_CMD(0xC0);
        print_selected_param1(LCD_ALIGN_LEFT);
        print_current_fuel_km(LCD_ALIGN_RIGHT);
    }

    if (drive_fl == 0 && motor_fl != 0 && request_screen((char *) &speed100_string) != 0) {
        speed100_measurement();
    }
}


void clear_trip(trip_t* trip) {
    if (request_screen((char *) &reset_string) != 0) {
        memset(trip, 0, sizeof(trip_t));
    }
}

void screen_trip(trip_t* trip, unsigned char trips_pos) {
    LCD_CMD(0x80);

    len = strcpy2(buf, (char *) &trip_string, 0);
    len += strcpy2(&buf[len], (char *) trips_str, trips_pos);
    LCD_Write_String8(buf, len, LCD_ALIGN_LEFT);

    print_trip_odometer(trip, LCD_ALIGN_RIGHT);
    
    LCD_CMD(0xC0);
    switch (select_param(&tmp_param, 2)) {
        case 0:
            print_trip_average_fuel(trip, LCD_ALIGN_LEFT);
            print_trip_average_speed(trip, LCD_ALIGN_RIGHT);
            break;
        case 1:
            print_trip_time(trip, LCD_ALIGN_LEFT);
            print_trip_total_fuel(trip, LCD_ALIGN_RIGHT);
            break;
    }
    clear_trip(trip);
}

#ifdef ALT_TRIPC_SCREEN

void screen_tripC() {
    // второй экран - экран счетчика C
    screen_trip(&trips.tripC, config.settings.daily_tripc ? TRIPS_POS_DAY : TRIPS_POS_CURR);
}

#else

void screen_tripC(void) {
    //; второй экран
    LCD_CMD(0x80);
    if (drive_min_speed_fl == 0) {
        print_trip_time(&trips.tripC, LCD_ALIGN_LEFT);
        if (motor_fl == 0) {
            //; 1) на месте с заглушенным двигателем
            //; время поездки C             общий пробег
            //; пробег C (км)               selected_param2
            print_main_odo(LCD_ALIGN_RIGHT);
        } else {
            //; 2) на месте с работающим двигателем
            //; время поездки C             тахометр (об/мин)	
            //; пробег C (км)               selected_param2
            print_taho(LCD_ALIGN_RIGHT);
        }
    } else {
        //; 3) в движении
        //; скорость (км/ч)             тахометр (об/мин)
        //; пробег C (км)               selected_param2
        print_speed(speed, 0, LCD_ALIGN_LEFT);
        print_taho(LCD_ALIGN_RIGHT);
    }

    LCD_CMD(0xC0);
    print_trip_odometer(&trips.tripC, LCD_ALIGN_LEFT);
    print_selected_param2(LCD_ALIGN_RIGHT);

    clear_trip(&trips.tripC);
}
#endif

void screen_tripA() {
    // экран счетчика A
    screen_trip(&trips.tripA, TRIPS_POS_A);
}

void screen_tripB() {
    // экран счетчика B
    screen_trip(&trips.tripB, TRIPS_POS_B);
}

void screen_temp() {
    if (config.settings.skip_temp_screen) {
#ifdef HW_LEGACY
        c_item++;
#else
        c_item += c_item_dir;
#endif        
        screen_refresh = 1;
    } else {
        LCD_CMD(0x80);
        print_temp(TEMP_OUT, true, LCD_ALIGN_RIGHT);
        LCD_CMD(0xC0);
        print_temp(TEMP_IN, true, LCD_ALIGN_RIGHT);
        LCD_CMD(0xC8);
        print_temp(TEMP_ENGINE, false, LCD_ALIGN_RIGHT);
#ifdef DS18B20_SUPPORT
        // force temperature update
        temperature_fl = 1;
#endif        
    }
}

void screen_service_counters() {
    
    srv_t* srv;
    service_time_t s_time;
    unsigned short v;
    
    select_param(&service_param, 5);

    LCD_CMD(0x80);
    LCD_Write_String16(buf, strcpy2((char*)buf, (char *) &service_counters, service_param + 1), LCD_ALIGN_LEFT);

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
    LCD_Write_String8(buf, len, LCD_ALIGN_RIGHT);
    
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

void service_screen_fuel_constant() {
    config.fuel_const = edit_value_char(config.fuel_const, CHAREDIT_MODE_NONE, 1, 255);
}

void service_screen_vss_constant() {
    config.odo_const = (uint16_t) edit_value_long(config.odo_const, 29999L);
}

void service_screen_total_trip() {
    config.odo = edit_value_long(config.odo, 999999L);
}

void service_screen_settings_bits() {
    config.settings.byte = edit_value_bits(config.settings.byte, (char *) &settings_bits);
}

void service_screen_min_speed() {
    config.min_speed = edit_value_char(config.min_speed, CHAREDIT_MODE_KMH, 1, 10);
}

void service_screen_temp_sensors() {
    
    char tbuf[8];
    
    memset(tbuf, 0xFF, 8);
    ds18b20_read_rom((unsigned char*) tbuf);
    ds18b20_serial_to_string((unsigned char*) tbuf, (unsigned char*) buf);
    
    unsigned char t_num = 0;
    
    start_timeout(300);
    while (timeout == 0) {
        screen_refresh = 0;
#ifndef HW_LEGACY
        if (key1_press != 0 || key2_press != 0 || key3_press != 0) {
#else        
        if (key1_press != 0 || key2_press != 0) {
#endif
            CLEAR_KEYS_STATE();
            t_num++;
            if (t_num >= 4) {
                t_num = 0;
            }
            start_timeout(300);
        }
        len = strcpy2(&buf[12], (char *) &temp_sensors, t_num + 1);
        add_leading_symbols(&buf[12], ' ', len, 4);
        
        LCD_CMD(0xC0);
        LCD_Write_String16(buf, 16, LCD_ALIGN_LEFT);
        
        while (screen_refresh == 0 && timeout == 0);
    }
    
    if (t_num != 0) {
        // save ds18b20 serial number to eeprom
        HW_write_eeprom_block((unsigned char *) tbuf, EEPROM_DS18B20_ADDRESS + (t_num - 1) * 8, 8);
    }
}

void service_screen_service_counters() {
    // next service counter
    if (key1_press != 0) {
        c_sub_item++;

        if (c_sub_item == 5) {
            c_sub_item = 0;
        }

        start_timeout(300);
    }
    
#ifndef HW_LEGACY
    // prev service counter
    if (key3_press != 0) {

        if (c_sub_item == 0) {
            c_sub_item = 4;
        } else {
            c_sub_item--;
        }

        start_timeout(300);
    }
#endif  
    
    LCD_CMD(0xC0);
    buf[0] = '1' + c_sub_item;
    buf[1] = '.';
    LCD_Write_String16(buf, 2 + strcpy2(&buf[2], (char *) &service_counters, c_sub_item + 1), LCD_ALIGN_LEFT);

    if (key2_press != 0) {
        CLEAR_KEYS_STATE();

        LCD_CMD(0x80);
        LCD_Write_String16(buf, strcpy2(buf, (char *) &service_counters, c_sub_item + 1), LCD_ALIGN_LEFT);
        LCD_CMD(0xC0);
        memset(buf, ' ', 16);
        LCD_Write_String16(buf, 16, LCD_ALIGN_LEFT);

        if (c_sub_item == 0) {
            services.mh.limit = (unsigned short) edit_value_long(services.mh.limit, 1999L);
        } else {
            services.srv[c_sub_item - 1].limit = edit_value_char(services.srv[c_sub_item - 1].limit, CHAREDIT_MODE_10000KM, 0, 60);
        }

        start_timeout(300);
    }
    
}

void service_screen_ua_const() {

    if (key1_press != 0) {
        if (config.vcc_const >= 140) {
            config.vcc_const--;
        }
        start_timeout(300);
    }
    if (key2_press != 0) {
#ifndef HW_LEGACY
        timeout = 1;
    }
    if (key3_press != 0) {
#endif    
        if (config.vcc_const < 230) {
            config.vcc_const++;
        }
        start_timeout(300);
    }

    LCD_CMD(0xC4);
    print_voltage(LCD_ALIGN_RIGHT);
    
}

void service_screen_version() {
    LCD_CMD(0xC0);
    LCD_Write_String16(buf, strcpy2(buf, (char*) &version_str, 0), LCD_ALIGN_LEFT);
}

void service_screen(unsigned char c_item) {
    service_screen_item_t item = items_service[c_item];
    
    LCD_CMD(0x80);
    LCD_Write_String16(buf, strcpy2(buf, (char *) &service_menu_title, 0), LCD_ALIGN_LEFT);

    LCD_CMD(0xC0);
    buf[0] = '1' + c_item;
    buf[1] = '.';
    LCD_Write_String16(buf, strcpy2(&buf[2], (char *) service_menu_str, item.str_index) + 2, LCD_ALIGN_LEFT);

    if (key2_press != 0) {
        key2_press = 0;
        LCD_Clear();
        c_sub_item = 0;
        start_timeout(300);
        while (timeout == 0) {
            screen_refresh = 0;

            LCD_CMD(0x80);
            LCD_Write_String16(buf, strcpy2(buf, (char *) service_menu_str, item.str_index), LCD_ALIGN_LEFT);

            item.screen();

            CLEAR_KEYS_STATE();

            while (screen_refresh == 0 && timeout == 0);
        }
        screen_refresh = 1;
    }
}

void read_eeprom() {
    unsigned char ee_addr = 0;
    
    HW_read_eeprom_block((unsigned char*) &config, ee_addr, sizeof(config_t));
    ee_addr += ((sizeof(config_t) - 1) / 8 + 1) * 8;
    
    HW_read_eeprom_block((unsigned char*) &trips, ee_addr, sizeof(trips_t));
    ee_addr += ((sizeof(trips_t) - 1) / 8 + 1) * 8;
    
    HW_read_eeprom_block((unsigned char*) &services, ee_addr, sizeof(services_t));

}

void save_eeprom() {
    unsigned char ee_addr = 0;
    HW_write_eeprom_block((unsigned char*) &config, ee_addr, sizeof(config_t));
    ee_addr += ((sizeof(config_t) - 1) / 8 + 1) * 8;
    
    HW_write_eeprom_block((unsigned char*) &trips, ee_addr, sizeof(trips_t));
    ee_addr += ((sizeof(trips_t) - 1) / 8 + 1) * 8;
    
    HW_write_eeprom_block((unsigned char*) &services, ee_addr, sizeof(services_t));
}

void power_off() {
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

 void power_on() {
    HW_Init();
    
    read_eeprom();
    
    LCD_Init();
    
    // default const
    fuel1_const = 65;
    fuel2_const = 28;
    odo_con4 = config.odo_const * 2 / 13;
#ifdef TAHO_RPM_FIX_VALUE
    taho_rpm_fix_const = (uint16_t) (TAHO_CONST*2 / TAHO_RPM_FIX_VALUE);
#endif    
    // const for dual injection
    if (config.settings.dual_injection != 0) {
#ifdef TAHO_RPM_FIX_VALUE
        taho_rpm_fix_const >>= 1;
#endif
        fuel1_const <<= 1; // 130
        fuel2_const >>= 1; // 14
        odo_con4 >>= 1;    // (config.odo_const * 2 / 13) / 2
    }
    
    main_interval = MAIN_INTERVAL;
    
    // change const for fast refresh
    if (config.settings.fast_refresh != 0) {
        main_interval >>= 1;
        fuel2_const <<= 1;
    }
    
    if (check_tripC_time() != 0) {
        // clear tripC
        memset(&trips.tripC, 0, sizeof(trip_t));
        trips.tripC_max_speed = 0;
    }
}

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
            buzzer_fl = 1; buzzer_init_fl = 0; buzzer_mode = &buzzer[BUZZER_WARN];
            LCD_CMD(0x80);
            LCD_Write_String16(buf, strcpy2(buf, (char*) &warning_str, 0), LCD_ALIGN_CENTER);

            LCD_CMD(0xC0);
            LCD_Write_String16(buf, strcpy2(buf, (char*) &service_counters, i + 1), LCD_ALIGN_CENTER);

            start_timeout(300);
            while (timeout == 0 && NO_KEY_PRESSED)
                ;
            CLEAR_KEYS_STATE();
        }
        warn >>= 1;
    }
}

#ifdef DS18B20_SUPPORT
void handle_temp() {
    temperature_fl = 0;
    if (temperature_conv_fl != 0) {
        // read temperature for ds18b20
        timeout_temperature = TIMEOUT_TEMPERATURE;
        temperature_conv_fl = 0;
        unsigned char _temps_ee_addr = EEPROM_DS18B20_ADDRESS;
        for (unsigned char i = 0; i < 3; i++) {
            HW_read_eeprom_block((unsigned char *) &buf, _temps_ee_addr, 8);
            ds18b20_read_temp_matchrom((unsigned char *) &buf, &temps[i]);
            _temps_ee_addr += 8;
        }
    } else {
        // start conversion for ds18b20
        temperature_conv_fl = 1;
        timeout_temperature = 1;
        ds18b20_start_conversion();
    }
}
#endif

void handle_misc_values() {
    mh_rpm_const = config.settings.dual_injection != 0 ? 1 : 2;

    speed100_const = (unsigned short) (SPEED100_CONST / config.odo_const);

    speed = (unsigned short) ((36000UL * (unsigned long) kmh) / (unsigned long) config.odo_const);

    drive_min_speed_fl = speed >= config.min_speed * 10;

    if (config.settings.fast_refresh == 0) {
        speed >>= 1;
    }

    if (trips.tripC_max_speed < speed) {
        trips.tripC_max_speed = speed;
    }

    if (drive_fl != 0 && speed == 0) {
        drive_fl = 0;
    }

    if (trips.tripA.odo > MAX_ODO_TRIPA) {
        memset(&trips.tripA, 0, sizeof (trip_t));
    }

    if (trips.tripB.odo > MAX_ODO_TRIPB) {
        memset(&trips.tripB, 0, sizeof (trip_t));
    }
}

void main() {

    power_on();

#ifndef HW_LEGACY
    start_timer1();
    enable_interrupts();
    delay_ms(100);
    if (KEY_SERVICE_PRESSED) {
        delay_ms(40);
        if (KEY_SERVICE_PRESSED) {
            service_mode = 1;
        }
    }

    if (service_mode != 0) {
        LCD_CMD(0x80);
        LCD_Write_String8(buf, strcpy2(buf, (char *) &service_menu_title, 0), LCD_ALIGN_LEFT);
        while (KEY_SERVICE_PRESSED);
    }
#else
    start_timer1();
    enable_interrupts();
#endif
    
#ifdef DS18B20_SUPPORT
    temperature_fl = 1;
#endif        

    if (service_mode == 0 && config.settings.service_alarm) {
        unsigned char warn = check_service_counters();
        print_warning_service_counters(warn);
    }
      
    CLEAR_KEYS_STATE();
    
    // wait first adc conversion
    while (adc_voltage == 0 && screen_refresh == 0)
        ;
    
    while (1) {
        screen_refresh = 0;

        // check power
        if (shutdown_fl) {
            power_off();
        }

        if (KEY_SERVICE_LONGPRESSED) {
            // long keypress for service key - switch service mode and main mode
            if (service_mode == 0 && motor_fl == 0 && drive_fl == 0) {
                prev_main_item  = c_item;
                c_item = prev_service_item;
                service_mode = 1;
                CLEAR_KEYS_STATE();
            } else if (service_mode == 1) {
                prev_service_item = c_item;
                c_item = prev_main_item;
                service_mode = 0;
                CLEAR_KEYS_STATE();
            }
        }
        
        if (service_mode == 0) {
            max_item = sizeof (items_main) / sizeof (screen_item_t);
#ifdef DS18B20_SUPPORT
            if (temperature_fl != 0) {
                handle_temp();
            }
#endif            
            handle_misc_values();
        } else {
            max_item = sizeof (items_service) / sizeof (service_screen_item_t);
        }
        
        // show next/prev screen
#ifdef HW_LEGACY
        if (key1_press != 0) {
            if (++c_item >= max_item) {
                c_item = 0;
            }
#else
        if (key1_press != 0 || key3_press != 0) {
               if (key1_press != 0) {
                    c_item_dir = 1;
                    if (++c_item >= max_item) {
                        c_item = 0;
                    }
                } else if (key3_press != 0) {
                    c_item_dir = -1;
                    if (c_item-- == 0) {
                        c_item = drive_min_speed_fl != 0 ? DRIVE_MODE_MAX : (max_item - 1);
                    }
                }
 #endif        
            tmp_param = 0;
            LCD_Clear();
            CLEAR_KEYS_STATE();
        }
        
        
        if (service_mode != 0) {
            service_screen(c_item);
        } else {
            if (drive_min_speed_fl != 0 && c_item > DRIVE_MODE_MAX) {
                c_item = 0;
                LCD_Clear();
            }
            items_main[c_item].screen();
        }
        
        while (screen_refresh == 0);
    }
}

