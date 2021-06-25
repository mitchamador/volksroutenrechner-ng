
/********************************************************************************************
* PIC16F876A 
* LCD 16x2 with 4 bits
*********************************************************************************************/

#include "hw.h"
#include "locale.h"
#include <stdbool.h>
#include <string.h>
#include "lcd.h"
#include "ds1307.h"
#include "ds18b20.h"
#include "utils.h"

// use ds18b20 temperature sensors
#define USE_DS18B20

// use sound
#define USE_SOUND

// place custom chars data in eeprom
#define EEPROM_CUSTOM_CHARS

// simple checking time difference (decrease memory usage)
//#define SIMPLE_TRIPC_TIME_CHECK

// number of averaging ADC readings
//#define ADC_AVERAGE_SAMPLES 8

// * timer1 resolution * 0,01ms
#define MAIN_INTERVAL 200
#define DEBOUNCE 4
#define SHORTKEY 50
#define LONGKEY 100

// taho const for 80us timer2
#define TAHO_CONST 750000UL

// round taho
#define TAHO_ROUND 10

//show average speed (or fuel consumption) after distance AVERAGE_MIN_DIST * 0.1 km
#define AVERAGE_MIN_DIST 3

// show average fuel consumption after total consumption of AVERAGE_MIN_FUEL * 0,01 litres
#define AVERAGE_MIN_FUEL 5

// min speed (* 0.1 km/h))
#define MIN_SPEED 50

// max value of trip A odometer
#define MAX_ODO_TRIPA 2000

// max value of trip B odometer
#define MAX_ODO_TRIPB 10000

// max pause for continuing trip C
#define TRIPC_PAUSE_MINUTES 120

// min rpm constant (1/(400rpm/60sec)/80us) 80us - timer2 overflow
#define TAHO_TIMEOUT 1875
// timeout in timer1 resolution 0,01s
#define TAHO_TIMEOUT_TIMER (unsigned char) ((TAHO_TIMEOUT * 0.000080f) / 0.01f)

// temperature timeout
#define TIMEOUT_TEMPERATURE (15 - 1)

typedef struct {
    unsigned char day;
    unsigned char month;
    unsigned char year;
} service_time_t;

typedef struct {
    unsigned short counter;
    unsigned char limit;
    service_time_t time;
} srv_t;

typedef struct {
    uint32_t counter;
    unsigned short limit;
} srv_mh_t;

typedef union {
  // a structure with 8 single bit bit-field objects, overlapping the union member "byte"
  unsigned char byte;
  struct {
    unsigned b0:1;
    unsigned b1:1;
    unsigned b2:1;
    unsigned b3:1;
    unsigned service_alarm:1;
    unsigned key_sound:1;
    unsigned skip_temp_screen:1;
    unsigned dual_injection:1;
  };
} settings_u;

typedef struct {
    unsigned short odo;
    unsigned short odo_temp;
    unsigned char fuel_tmp1, fuel_tmp2;
    unsigned short fuel;
    uint32_t time;
} trip_t;

typedef struct {
    unsigned char minute, hour, day, month, year;
} trip_time_t;

// service counters limits
typedef struct {
    // main odometer
    uint32_t odo;
    unsigned short odo_temp;

    unsigned short odo_const;
    unsigned char fuel_const;
    unsigned char vcc_const;

    settings_u settings;

    // param counter for main screen
    unsigned char selected_param1;
    // param counter for tripC screen
    unsigned char selected_param2;
} config_t;

typedef struct {
    srv_mh_t mh;
    srv_t oil_engine;
    srv_t oil_gearbox;
    srv_t air_filter;
    srv_t sparks;
} services_t;

typedef struct {
    trip_t tripA, tripB, tripC;
    trip_time_t tripC_time;
    unsigned short tripC_max_speed;
} trips_t;

typedef struct {
    char sn_in[8];
    char sn_out[8];
    char sn_engine[8];  
} ds18b20_sn_t;

config_t config;
trips_t trips;
services_t services;

__EEPROM_DATA(0x7F,0x9F,0x04,0x00,0x16,0x13,0x80,0x3E); /*config*/
__EEPROM_DATA(0x6E,0xA6,0x80,0x01,0x00,0x00,0x00,0x00);
__EEPROM_DATA(0x0A,0x00,0x90,0x1B,0x1A,0x28,0x64,0x00); /*trips*/
__EEPROM_DATA(0x02,0x02,0x00,0x00,0x22,0x01,0xDD,0x3A);
__EEPROM_DATA(0x3D,0x64,0x11,0x0F,0xC8,0x4F,0x00,0x00);
__EEPROM_DATA(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
__EEPROM_DATA(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
__EEPROM_DATA(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
__EEPROM_DATA(0xBA,0x15,0x03,0x00,0x00,0x00,0x16,0x01); /*services*/
__EEPROM_DATA(0x0A,0x22,0x01,0x21,0x99,0x13,0x0A,0xFF);
__EEPROM_DATA(0xFF,0xFF,0x99,0x13,0x0A,0x21,0x03,0x20);
__EEPROM_DATA(0xAA,0x04,0x0A,0x07,0x01,0x20,0x00,0x00);
// ds18b20 serial numbers (OUT, IN, ENGINE))
__EEPROM_DATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF); /*ds18b20 serial numbers*/
__EEPROM_DATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
__EEPROM_DATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);

//========================================================================================

// key variables and flags
unsigned char key1_counter, key2_counter, shutdown_counter;
volatile __bit key1_press, key1_longpress, key2_press, key2_longpress;

// 0.1s flag and counter
unsigned char counter_01sec;
volatile __bit counter_01sec_fl;

// main interval
unsigned char main_interval_counter;
volatile __bit screen_refresh;

// timeout
volatile unsigned int timeout_timer;
volatile __bit timeout;

#ifdef USE_DS18B20
volatile unsigned char timeout_temperature;
volatile __bit temperature_fl, temperature_conv_fl;
#endif

// misc flags
volatile __bit odom_fl, drive_fl, motor_fl, fuel_fl, taho_fl, taho_measure_fl, shutdown_fl;

// speed100 flags and variables
volatile __bit speed100_fl, speed100_ok_fl, speed100_timer_fl;
volatile unsigned short speed100_const, speed100_timer;

unsigned short kmh_tmp, fuel_tmp, taho_tmp;
volatile unsigned short kmh, fuel, taho, adc;

#ifdef ADC_AVERAGE_SAMPLES
unsigned short adc_tmp;
unsigned char adc_counter = 1; // init value for first reading
#endif             

// xc8 handmade optimization
__bank3 unsigned short speed;

#ifdef USE_DS18B20
// ds18b20 temperatures and eeprom pointer
#define TEMP_OUT 0
#define TEMP_IN 1
#define TEMP_ENGINE 2
uint16_t temps[3] = {0, 0, 0};
#endif

unsigned char temps_ee_addr;

#ifdef USE_SOUND
__bit buzzer_fl, buzzer_init_fl, buzzer_snd_fl, buzzer_repeat_fl;
unsigned char buzzer_counter_r;
unsigned char buzzer_counter;

typedef struct {
    unsigned char counter; // number of repeats
    unsigned char sound;   // sound on duration  (*0.1ms)
    unsigned char pause;   // sound off duration (*0.1ms)
} buzzer_t;

buzzer_t *buzzer_mode;

#define BUZZER_KEY 0
#define BUZZER_LONGKEY 1
#define BUZZER_WARN 2

buzzer_t buzzer[3] = {
    {1,1,1}, // BUZZER_KEY
    {1,4,1}, // BUZZER_LONG_KEY
    {3,3,2}  // BUZZER_WARN
};
#endif

unsigned char tbuf[8];
unsigned char tmp_param = 0;

// buffer for strings
char buf[16];
//char buf2[16];
unsigned char len = 0;

ds_time time;

unsigned char fuel1_const;

#ifdef EEPROM_CUSTOM_CHARS
#define CUSTOM_CHAR_DATA(a) __EEPROM_DATA(a);
#else
#define CUSTOM_CHAR_DATA(a) a,
#endif

#ifndef EEPROM_CUSTOM_CHARS
// custom lcd characters data
const unsigned char custom_chars[] = {
#endif    
CUSTOM_CHAR_DATA(DATA_KMH_0)    // kmh[0]
CUSTOM_CHAR_DATA(DATA_KMH_1)    // kmh[1]
CUSTOM_CHAR_DATA(DATA_OMIN_0)   // omin[0]
CUSTOM_CHAR_DATA(DATA_OMIN_1)   // omin[1]
CUSTOM_CHAR_DATA(DATA_L100_0)   // L100[0]
CUSTOM_CHAR_DATA(DATA_L100_1)   // L100[1]
CUSTOM_CHAR_DATA(DATA_LH_0)     // l/h[0]
CUSTOM_CHAR_DATA(DATA_LH_1)     // l/h[1]
#ifndef EEPROM_CUSTOM_CHARS
};
#endif

typedef void (*screen_func) (void);

void screen_main(void);
void screen_tripC(void);
void screen_temp(void);
void screen_max(void);
void screen_tripA(void);
void screen_tripB(void);
void screen_time(void);
void screen_service_counters(void);

typedef struct {
    screen_func screen;
    unsigned char drive_mode; // show screen id drive mode
} screen_item_t;

const screen_item_t items_main[] = {
    {screen_main, 1},
    {screen_tripC, 1},
#ifdef USE_DS18B20
    {screen_temp, 1},
#endif
    {screen_tripA, 0},
    {screen_tripB, 0},
    {screen_time, 0},
    {screen_service_counters, 0},
};

typedef struct screen_service_item_t screen_service_item_t;
typedef void (*screen_service_func) (screen_service_item_t *);

struct screen_service_item_t {
    const char* name;
    screen_service_func screen;
};

void screen_service_fuel_constant(screen_service_item_t *);
void screen_service_vss_constant(screen_service_item_t *);
void screen_service_total_trip(screen_service_item_t *);
void screen_service_settings_bits(screen_service_item_t *);
void screen_service_temp_sensors(screen_service_item_t *);
void screen_service_service_counters(screen_service_item_t *);
void screen_service_ua_const(screen_service_item_t *);

const screen_service_item_t items_service[] = {
    {(const char*) &fuel_constant_str, (screen_service_func) &screen_service_fuel_constant},
    {(const char*) &vss_constant_str, (screen_service_func) &screen_service_vss_constant},
    {(const char*) &total_trip_str, (screen_service_func) &screen_service_total_trip},
    {(const char*) &voltage_adjust_str, (screen_service_func) &screen_service_ua_const},
    {(const char*) &settings_bits_str, (screen_service_func) &screen_service_settings_bits},
#ifdef USE_DS18B20
    {(const char*) &temp_sensor_str, (screen_service_func) &screen_service_temp_sensors},
#endif    
    {(const char*) &service_counters_str, (screen_service_func) &screen_service_service_counters},
};

unsigned char c_item = 0, c_sub_item = 0;

unsigned char request_screen(char *);

int_handler_GLOBAL_begin

    int_handler_fuel_speed_begin
        // fuel injector
        if (FUEL_ACTIVE) {
            if (fuel_fl == 0) {
                // start timer0
                start_timer_fuel();
                fuel_fl = 1;
                motor_fl = 1;

                // taho calculation
                if (taho_measure_fl == 1) {
                    if (taho_fl == 1) {
                       // stop timer2
                        stop_timer_taho();
                        taho = taho_tmp;
                        taho_fl = 0;
                        taho_measure_fl = 0;
                    } else {
                        // start timer2
                        start_timer_taho();
                        taho_tmp = 0;
                        taho_fl = 1;
                    }
                }
            }
        } else {
            if (fuel_fl == 1) {
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
                
                // speed 100 calculation
                if (speed100_timer_fl == 1) {
                    if (speed100_fl == 1) {
                       // stop timer2
                        stop_timer_taho();
                        if (taho_tmp < speed100_const) {
                            speed100_ok_fl = 1;
                            speed100_timer_fl = 0;
                        } else {
                            speed100_fl = 0;
                        }
                    }
                    if (speed100_fl == 0) {
                        // start timer2 
                        taho_tmp = 0;
                        start_timer_taho();
                        speed100_fl = 1;
                    }
                }

                kmh_tmp++;

                // xc8 handmade optimization
                unsigned short config_odo_const = config.odo_const;
                
                // main odometer
                if (++config.odo_temp >= config_odo_const) {
                    config.odo_temp = 0;
                    // increment odometer counters
                    config.odo++;
                    services.oil_engine.counter++;
                    services.oil_gearbox.counter++;
                    services.air_filter.counter++;
                    services.sparks.counter++;
                }
                
                // trip A
                if (++trips.tripA.odo_temp > config_odo_const) {
                    trips.tripA.odo_temp = 0;
                    trips.tripA.odo++;
                }

                // trip B
                if (++trips.tripB.odo_temp >= config_odo_const) {
                    trips.tripB.odo_temp = 0;
                    trips.tripB.odo++;
                }

                // trip C
                if (++trips.tripC.odo_temp >= config_odo_const) {
                    trips.tripC.odo_temp = 0;
                    trips.tripC.odo++;
                }
            }
        } else {
            odom_fl = 0;
        }
    int_handler_fuel_speed_end
        
    int_handler_timer0_begin
            
        fuel_tmp++;
        
        // xc8 handmade optimization
        unsigned char fuel1_const_b3 = fuel1_const;
        unsigned char config_fuel_const = config.fuel_const;
        
        if (--trips.tripA.fuel_tmp1 == 0) {
            trips.tripA.fuel_tmp1 = fuel1_const_b3;
            if (++trips.tripA.fuel_tmp2 >= config_fuel_const) {
                trips.tripA.fuel_tmp2 = 0;
                trips.tripA.fuel++;
            }
        }
        
        if (--trips.tripB.fuel_tmp1 == 0) {
            trips.tripB.fuel_tmp1 = fuel1_const_b3;
            if (++trips.tripB.fuel_tmp2 >= config_fuel_const) {
                trips.tripB.fuel_tmp2 = 0;
                trips.tripB.fuel++;
            }
        }

        if (--trips.tripC.fuel_tmp1 == 0) {
            trips.tripC.fuel_tmp1 = fuel1_const_b3;
            if (++trips.tripC.fuel_tmp2 >= config_fuel_const) {
                trips.tripC.fuel_tmp2 = 0;
                trips.tripC.fuel++;
            }
        }

    int_handler_timer0_end

    int_handler_timer1_begin

        if (KEY1_PRESSED) // key pressed
        {
            if (key1_counter <= LONGKEY) {
                key1_counter++;
            }
            if (key1_counter == LONGKEY) {
                // long keypress
                key1_longpress = 1;
                screen_refresh = 1;
#ifdef USE_SOUND
                if (config.settings.key_sound) {
                    buzzer_fl = 1; buzzer_mode = &buzzer[BUZZER_LONGKEY];
                }
#endif
            }
        } else // key released
        {
            if (key1_counter > DEBOUNCE && key1_counter < SHORTKEY) {
                // key press
                key1_press = 1;
                screen_refresh = 1;
#ifdef USE_SOUND
                if (config.settings.key_sound) {
                    buzzer_fl = 1; buzzer_mode = &buzzer[BUZZER_KEY];
                }
#endif
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
#ifdef USE_SOUND
                if (config.settings.key_sound) {
                    buzzer_fl = 1; buzzer_mode = &buzzer[BUZZER_LONGKEY];
                }
#endif
            }
        } else // key released
        {
            if (key2_counter > DEBOUNCE && key2_counter < SHORTKEY) {
                // key press
                key2_press = 1;
                screen_refresh = 1;
#ifdef USE_SOUND
                if (config.settings.key_sound) {
                    buzzer_fl = 1; buzzer_mode = &buzzer[BUZZER_KEY];
                }
#endif
            }
            key2_counter = 0;
        }

        if (++main_interval_counter >= MAIN_INTERVAL) {
             main_interval_counter = 0;

             // screen refresh_flag
             screen_refresh = 1;
             
             // copy temp interval variables to main
             fuel = fuel_tmp;
             fuel_tmp = 0;
             kmh = kmh_tmp;
             kmh_tmp = 0;
             
             // increase time counters
             if (motor_fl == 1 || drive_fl == 1) {
                 services.mh.counter++;
                 trips.tripA.time++;
                 trips.tripB.time++;
                 trips.tripC.time++;
             }
             
#ifdef USE_DS18B20
             if (timeout_temperature > 0) {
                if (--timeout_temperature == 0) {
                    temperature_fl = 1;
                }
             }
#endif
        }
        
        if (timeout_timer > 0) {
            if (--timeout_timer == 0) {
                timeout = 1;
            }
        }

        if (speed100_timer_fl == 1) {
            speed100_timer++;
        }

        if (counter_01sec == 0) {
            counter_01sec_fl = 1;
            counter_01sec = 10;
#ifdef USE_SOUND    
            if (buzzer_fl == 1) {
                if (buzzer_init_fl == 0) {
                    buzzer_init_fl = 1;
                    buzzer_repeat_fl = 1;
                    buzzer_counter_r = buzzer_mode->counter + 1;
                }
                
                if (buzzer_repeat_fl == 1) {
                    buzzer_repeat_fl = 0;
                    if (--buzzer_counter_r > 0) {
                        buzzer_snd_fl = 1;
                        buzzer_counter = buzzer_mode->sound;
                    } else {
                        buzzer_fl = 0;
                        buzzer_init_fl = 0;
                    }
                }
        
                if (buzzer_snd_fl == 1) {
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
#endif    
        } else {
            counter_01sec--;
        }
    int_handler_timer1_end

    int_handler_timer2_begin
        if (++taho_tmp > TAHO_TIMEOUT)
        {
            // stop timer2
            stop_timer_taho();
            // stop timer0
            stop_timer_fuel();
            taho_measure_fl = 0;
            taho_fl = 0;
            speed100_fl = 0;
            motor_fl = 0;
            taho = 0;
        }
    int_handler_timer2_end    

    int_handler_adc_begin
#ifdef ADC_AVERAGE_SAMPLES
        adc_tmp += adc_read_value();
        if (--adc_counter == 0) {
            adc_counter = ADC_AVERAGE_SAMPLES;
            adc = adc_tmp;
            adc_tmp = 0;
        }
#else
        adc = adc_read_value();
#endif    
        power_supply_read_digital();
        // read power supply status
        if (POWER_SUPPLY_ACTIVE) {
            shutdown_counter = 0;
        } else {
            if (++shutdown_counter == 8) {
                shutdown_fl = 1;
            }
        }
        power_supply_read_analog();
        
    int_handler_adc_end
    
int_handler_GLOBAL_end

void _LCD_Init(void) {
    LCD_Init(0x4E);    // Initialize LCD module with I2C address = 0x40 ((0x20<<1) for PCF8574) or 0x70 ((0x38<<1) for PCF8574A)
    
    // LCD set custom characters
    unsigned char i = 0;
#ifdef EEPROM_CUSTOM_CHARS
    for (i = 0; i < 64; i = i + 8) {
        LCD_CMD(LCD_SETCGRAMADDR | (i & ~0x07));
        HW_read_eeprom_block((unsigned char*) buf, temps_ee_addr + 24 + i, 8);
        unsigned char j;
        for (j = 0; j < 8; j++) {
            LCD_Write_Char(buf[j]);
        }
    }
#else        
    for (i = 0; i < 64; i++) {
        // LCD_SETCGRAMADDR | (location << 3)
        if ((i & 0x07) == 0) {
            LCD_CMD(LCD_SETCGRAMADDR | (i & ~0x07));
        }
        LCD_Write_Char(custom_chars[i]);
    }
#endif
}

void print_current_time_hm(unsigned char hour, unsigned char minute, align_t align) {
    bcd8_to_str(buf, hour);
    buf[2] = ':';
    bcd8_to_str(&buf[3], minute);
    LCD_Write_String8(buf, 5, align);
}

void print_current_time_dmy(unsigned char day, unsigned char month, unsigned char year) {
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

void print_current_time(ds_time* time) {

    LCD_CMD(0x80);
    print_current_time_hm(time->hour, time->minute, LCD_ALIGN_LEFT);

    LCD_CMD(0x88);
    print_current_time_dmy(time->day, time->month, time->year);

    LCD_CMD(0xc0);
    LCD_Write_String16(buf, strcpy2((char *)buf, (char*) day_of_week_str, time->day_of_week), LCD_ALIGN_LEFT);
}

void screen_time(void) {

    get_ds_time(&time);
    if (request_screen((char *) &time_correction) == 1) {

        unsigned char c = 0;
        const char cursor_position[] = {0x81, 0x84, 0x89, 0x8c, 0x8f, 0xc0};

        LCD_Clear();
        while (c < 6) {
            LCD_CMD(LCD_CURSOR_OFF);
            print_current_time(&time);
            LCD_CMD(cursor_position[c]);
            LCD_CMD(LCD_UNDERLINE_ON);
            while (key1_press == 0 && key2_press == 0);
            if (key1_press != 0) {
                // key1 press - edit next element
                c++;
            } else {
                // key2 press - increment current element
                switch (c) {
                    case 0:
                        time.hour = bcd8_inc(time.hour, 23);
                        break;
                    case 1:
                        time.minute = bcd8_inc(time.minute, 59);
                        break;
                    case 2:
                        time.day = bcd8_inc(time.day, 31);
                        if (time.day == 0) time.day++;
                        break;
                    case 3:
                        time.month = bcd8_inc(time.month, 12);
                        if (time.month == 0) time.month++;
                        break;
                    case 4:
                        time.year = bcd8_inc(time.year, 99);
                        break;
                    case 5:
                        time.day_of_week = bcd8_inc(time.day_of_week, 7);
                        if (time.day_of_week == 0) time.day_of_week++;
                        break;
                }
            }

            key1_press = 0;
            key2_press = 0;
        }
        LCD_CMD(LCD_CURSOR_OFF);
        // save time
        set_ds_time(&time);
        key1_longpress = 0;
        key2_longpress = 0;

    } else {
        print_current_time(&time);
    }
}

unsigned char get_fractional_string(char * buf, unsigned short num) {
    unsigned char len = ultoa2(buf, (unsigned int) num, 10);

    if (num < 10) {
        buf[1] = buf[0];
        buf[0] = '0';
        len++;
    }
    
    buf[len + 1] = 0;
    buf[len] = buf[len - 1];
    buf[len - 1] = '.';
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
    bcd8_to_str(&buf[len], time % 60);
    len += 2;

    LCD_Write_String8(buf, len, align);
}

/**
 * show trip average speed
 * @param t
 * @param align
 */
void print_trip_average_speed(trip_t* t, align_t align) {
    
    unsigned short speed = 0;
    if (t->time > 0) {
        speed = (unsigned short) ((unsigned long) ((t->odo * 18000UL) + (t->odo_temp * 18000UL / config.odo_const)) / t->time);
    }
    
    if (speed == 0) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        len = get_fractional_string(buf, speed);
    }

    buf[len++] = _kmh0;
    buf[len++] = _kmh1;

    LCD_Write_String8(buf, len, align);

}

/**
 * show trip odometer (km)
 * @param t
 * @param align
 */
void print_trip_odometer(trip_t* t, align_t align) {
    
    unsigned short odo = (unsigned short) ((unsigned long) (t->odo * 10UL) + (t->odo_temp * 10UL / config.odo_const));
    
    len = get_fractional_string(buf, odo);
    buf[len++] = KM1_SYMBOL;
    buf[len++] = KM2_SYMBOL;
   
    LCD_Write_String8(buf, len, align);

}

/**
 * show trip total fuel consumption (l)
 * @param t
 * @param align
 */
void print_trip_total_fuel(trip_t* t, align_t align) {
    len = get_fractional_string(buf, t->fuel / 10);
    buf[len++] = LITRE_SYMBOL;
    LCD_Write_String8(buf, len, align);
}

/**
 * show trip average fuel consumption (l/100km)
 * @param t
 * @param align
 */
void print_trip_average_fuel(trip_t* t, align_t align) {
    
    if (t->fuel < AVERAGE_MIN_FUEL) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        unsigned short odo = (unsigned short) ((unsigned long) (t->odo * 10UL) + (t->odo_temp * 10UL / config.odo_const));
        if (odo < AVERAGE_MIN_DIST) {
            len = strcpy2(buf, (char *) &empty_string, 0);
        } else {
            len = get_fractional_string(buf, (unsigned short) (t->fuel * 100UL / odo));
        }
    }

    buf[len++] = _lkm0;
    buf[len++] = _lkm1;

    LCD_Write_String8(buf, len, align);
}

void print_speed(unsigned short speed, align_t align) {
    if (speed > 1000) {
        // more than 100 km/h, skip fractional
        len = ultoa2(buf, (unsigned int) speed, 10) - 1;
    } else {
        // lower than 100 km/h, use fractional
        len = get_fractional_string(buf, speed);
    }

    buf[len++] = _kmh0;
    buf[len++] = _kmh1;

    LCD_Write_String8(buf, len, align);
}

void print_taho(align_t align) {
    taho_measure_fl = 1;
    timeout = 0; timeout_timer = TAHO_TIMEOUT_TIMER;
    while (motor_fl == 1 && taho_measure_fl == 1 && timeout == 0)
        ;
    if (timeout == 1) {
        motor_fl = 0;
    }

    if (taho == 0 || motor_fl == 0) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        unsigned short res = (unsigned short) ((TAHO_CONST / taho));
#ifdef TAHO_ROUND
        res = res / TAHO_ROUND * TAHO_ROUND;
#endif        
        len = ultoa2(buf, (unsigned int) res, 10);
        buf[len++] = _omin0;
        buf[len++] = _omin1;
    }
    LCD_Write_String8(buf, len, align);
}

void print_current_fuel_km(align_t align) {
    unsigned short odo_con4 = config.odo_const * 2 / (config.settings.dual_injection ? 26 : 13);
    unsigned short t = (unsigned short) ((((unsigned long) fuel * (unsigned long) odo_con4) / (unsigned long) kmh) / (unsigned char) config.fuel_const);
    len = get_fractional_string(buf, t);
    buf[len++] = _lkm0;
    buf[len++] = _lkm1;
    LCD_Write_String8(buf, len, align);
}

void print_current_fuel_lh(align_t align) {
    len = get_fractional_string(buf, fuel * (config.settings.dual_injection ? 14UL : 28UL) / config.fuel_const / 10);
    buf[len++] = _lh0;
    buf[len++] = _lh1;
    LCD_Write_String8(buf, len, align);
}

void print_main_odo(align_t align) {
    len = ultoa2(buf, (unsigned long) config.odo, 10);
    buf[len++] = KM1_SYMBOL;
    buf[len++] = KM2_SYMBOL;
    LCD_Write_String8(buf, len, align);
}

#ifdef USE_DS18B20
void print_temp(unsigned char index, bool header, align_t align) {
    uint16_t _t = temps[index];
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
        buf[len++] = CELSIUS_SYMBOL;
        LCD_Write_String8(buf, len, align);
    } else {
        len = get_fractional_string(&buf[1], _t) + 1;
        if (header) {
            add_leading_symbols(buf, ' ', len, 8);
            strcpy2(buf, (char *) &temp_sensors, (index + 1) + 1);
            len = 8;
        }
        
        LCD_Write_String8(buf, len, align);
    }
}
#endif

void print_voltage(align_t align) {
    len = get_fractional_string(buf, (unsigned short) (adc << 5) / config.vcc_const);
    buf[len++] = VOLT_SYMBOL;
    LCD_Write_String8(buf, len, align);
}

unsigned char select_param(unsigned char* param, unsigned char total) {
    if (key2_press == 1) {
        key2_press = 0;
        *param = *param + 1;
    }
    if (*param >= total) {
        *param = 0;
    }
    return *param;
}

void print_selected_param2(align_t align) {
    switch (select_param(&config.selected_param2, 4)) {
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
            print_speed(trips.tripC_max_speed, align);
            break;
    }
}

#ifdef USE_DS18B20
#define COUNT_SELECTED_PARAM1 4
#else
#define COUNT_SELECTED_PARAM1 3
#endif

void print_selected_param1(align_t align) {
    switch (select_param(&config.selected_param1, COUNT_SELECTED_PARAM1)) {
#ifdef USE_DS18B20
        case COUNT_SELECTED_PARAM1 - 4:
            print_temp(TEMP_OUT, false, align);
            break;
#endif
        case COUNT_SELECTED_PARAM1 - 3:
            print_voltage(align);
            break;
        case COUNT_SELECTED_PARAM1 - 2:
            print_trip_odometer(&trips.tripC, align);
            break;
        case COUNT_SELECTED_PARAM1 - 1:
            print_trip_average_fuel(&trips.tripC, align);
            break;
    }
}

void print_speed100(void) {
    len = get_fractional_string((char*) tbuf, speed100_timer / 10U);
    tbuf[len++] = SECONDS_SYMBOL;
    LCD_CMD(0xC4);
    LCD_Write_String8((char*) tbuf, len, LCD_ALIGN_RIGHT);
}

void screen_main(void) {
//; первый экран

    LCD_CMD(0x80);
    if (drive_fl == 0 || speed < MIN_SPEED) {
//; 1) на месте с заглушенным двигателем
//; время текущее       общий пробег (км)
//; нар.темп./пробег C  вольтметр
        get_ds_time(&time);
        print_current_time_hm(time.hour, time.minute, LCD_ALIGN_LEFT);

        if (motor_fl == 0) {
            print_main_odo(LCD_ALIGN_RIGHT);

            LCD_CMD(0xC0);
#ifdef USE_DS18B20            
            print_temp(TEMP_OUT, false, LCD_ALIGN_LEFT);
#else
            print_trip_odometer(&trips.tripC, LCD_ALIGN_LEFT);
#endif            
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
        print_speed(speed, LCD_ALIGN_LEFT);
        print_taho(LCD_ALIGN_RIGHT);

        LCD_CMD(0xC0);
        print_selected_param1(LCD_ALIGN_LEFT);
        print_current_fuel_km(LCD_ALIGN_RIGHT);
    }

    if (drive_fl == 0 && motor_fl == 1 && request_screen((char *) &speed100_string) == 1) {
        LCD_CMD(0xC0);
        LCD_Write_String16(buf, strcpy2(buf, (char *) &speed100_wait_string, 0), LCD_ALIGN_CENTER);

        memset(buf, ' ', 16);
        
        // 36000000 / 80us / odo_const
        speed100_const = (unsigned short) (450000UL / config.odo_const);

        timeout = 0; timeout_timer = 3000;

        speed100_fl = 0; speed100_ok_fl = 0; speed100_timer = 0;

        unsigned char _speed100_skip_pulses = 8; // skip some pulses from speed sensor before start acceleration measurement
        
        while (timeout == 0 && speed100_ok_fl == 0 && key1_press == 0 && key2_press == 0) {
            if (_speed100_skip_pulses != 0) {
                if (drive_fl == 1) {
                    drive_fl = 0;
                    _speed100_skip_pulses--;
                }
            } else {
                
                if (speed100_timer_fl == 0) {
                    speed100_timer_fl = 1;
                    LCD_CMD(0xC0);
                    LCD_Write_String16(buf, 16, LCD_ALIGN_LEFT);
                }

                print_speed100();

                counter_01sec_fl = 0; while (counter_01sec_fl == 0);
            }
        }
        speed100_timer_fl = 0;
        
        if (key1_press == 0 && key2_press == 0) {
            if (speed100_ok_fl) {
                // достигнута скорость 100 км/ч
                print_speed100();
            } else {
                // timeout
                LCD_Clear();
                LCD_CMD(0xC0);
                LCD_Write_String16(buf, strcpy2(buf, (char *) &timeout_string, 0), LCD_ALIGN_CENTER);

            }
#ifdef USE_SOUND    
            buzzer_fl = 1; buzzer_init_fl = 0; buzzer_mode = &buzzer[BUZZER_WARN];
#endif
            timeout = 0; timeout_timer = 600; while (timeout == 0);
        }
        key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0;
    }
}

/**
 * 
 * @param _len
 * @return 
 */
unsigned char request_screen(char* request_str) {
    unsigned char reset = 0;
    if (key2_longpress == 1) {
        key2_longpress = 0;
        key2_press = 0;
        
        LCD_Clear();
        LCD_CMD(0x80);
        LCD_Write_String16(buf, strcpy2(buf, request_str, 0), LCD_ALIGN_CENTER);

        timeout = 0; timeout_timer = 500;
        while (timeout == 0 && key2_press == 0);
        
        if (key2_press == 1) {
            key2_press = 0;
            reset = 1;
        }
        
        screen_refresh = 1;
    }
    return reset;
}


void clear_trip(trip_t* trip) {
    if (request_screen((char *) &reset_string) == 1) {
        memset(trip, 0, sizeof(trip_t));
    }
}

void screen_tripC(void) {
//; второй экран
    LCD_CMD(0x80);
    if (drive_fl == 0 || speed < MIN_SPEED) {
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
        print_speed(speed, LCD_ALIGN_LEFT);
        print_taho(LCD_ALIGN_RIGHT);
    }

    LCD_CMD(0xC0);
    print_trip_odometer(&trips.tripC, LCD_ALIGN_LEFT);
    print_selected_param2(LCD_ALIGN_RIGHT);
    
    clear_trip(&trips.tripC);
}

void screen_tripAB(trip_t* trip, unsigned char ch) {
    LCD_CMD(0x80);

    len = strcpy2(buf, (char *) &trip_string, 0);
    buf[len++] = ch;
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

void screen_tripA() {
    // экран счетчика A
    screen_tripAB(&trips.tripA, 'A');
}

void screen_tripB() {
    // экран счетчика B
    screen_tripAB(&trips.tripB, 'B');
}

#ifdef USE_DS18B20
void screen_temp() {
    if (config.settings.skip_temp_screen) {
        key1_press = 1;
        screen_refresh = 1;
    } else {
        LCD_CMD(0x80);
        print_temp(TEMP_OUT, true, LCD_ALIGN_RIGHT);
        LCD_CMD(0xC0);
        print_temp(TEMP_IN, true, LCD_ALIGN_RIGHT);
        LCD_CMD(0xC8);
        print_temp(TEMP_ENGINE, false, LCD_ALIGN_RIGHT);
    }
}
#endif

void screen_service_counters() {
    
    srv_t* srv;
    service_time_t s_time;
    unsigned short v;
    
    switch (select_param(&tmp_param, 5)) {
        case 0:
        case 1:
            srv = &services.oil_engine;
            break;
        case 2:
            srv = &services.oil_gearbox;
            break;
        case 3:
            srv = &services.air_filter;
            break;
        case 4:
            srv = &services.sparks;
            break;
    }

    LCD_CMD(0x80);
    LCD_Write_String16(buf, strcpy2((char*)buf, (char *) &service_counters, tmp_param + 1), LCD_ALIGN_LEFT);

    LCD_CMD(0xC0);
    if (tmp_param == 0) {
        s_time = services.oil_engine.time;
        v = (unsigned short) (services.mh.counter / 1800L);
    } else {
        s_time = srv->time;
        v = srv->counter;
    }
    len = ultoa2(buf, v, 10);
    if (tmp_param == 0) {
        buf[len++] = HOUR_SYMBOL;
    } else {
        buf[len++] = KM1_SYMBOL;
        buf[len++] = KM2_SYMBOL;
    }
    buf[len++] = ' ';
    LCD_Write_String8(buf, len, LCD_ALIGN_RIGHT);
    
    LCD_CMD(0xC8);
    print_current_time_dmy(s_time.day, s_time.month, s_time.year);
    
    if (request_screen((char *) &reset_string) == 1) {
//    if (reset_screen() == 1) {
        get_ds_time(&time);
        if (tmp_param == 0 || tmp_param == 1) {
            services.mh.counter = 0;
        }
        srv->counter = 0;
        srv->time.day = time.day;
        srv->time.month = time.month;
        srv->time.year = time.year;
    }
}
unsigned char edit_value_char(unsigned char v, bool thousands) {
    timeout = 0; timeout_timer = 300;
    while (timeout == 0) {
        screen_refresh = 0;

        if (key1_press == 1 || key2_press == 1) {
            if (key1_press == 1) {
                key1_press = 0;
                v++;
                if (thousands && v == 60) {
                    v = 0;
                }
            } else {
                key2_press = 0;
                v--;
                if (thousands && v == 0xFF) {
                    v = 60;
                }
            }
            timeout = 0; timeout_timer = 300;
        }
        
        len = ultoa2(buf, thousands ? (unsigned short) (v * 1000) : v , 10);
        
        LCD_CMD(0xC4);
        LCD_Write_String8(buf, len, LCD_ALIGN_RIGHT);

        while (screen_refresh == 0 && timeout == 0);
    }
    key1_press = 0;
    key2_press = 0;

    return v;
}

unsigned long edit_value_long(unsigned long v, unsigned long max_value) {
    // number of symbols to edit
    unsigned char max_len = ultoa2(buf, max_value, 10);
    
    if (v > max_value) {
        v = max_value;
    }
    
    // convert value
    unsigned char v_len = ultoa2(buf, v, 10);
    
    add_leading_symbols(buf, '0', v_len, max_len);
    
    unsigned char cursor_pos = 0xC0 + (16 - max_len) / 2U;
    unsigned char pos = 0;

    timeout = 0; timeout_timer = 300;
    while (timeout == 0) {
        screen_refresh = 0;

        // change cursor position
        if (key1_press == 1) {
            key1_press = 0;
            pos++;
            if (pos == max_len) {
                pos = 0;
            }
            timeout = 0; timeout_timer = 300;
        }
        
        // edit number in cursor position
        if (key2_press == 1) {
            key2_press = 0;
            
            if (++buf[pos] > '9') {
                buf[pos] = '0';
            }
            
            unsigned long _t = strtoul(buf, NULL, 10);
            if (_t > max_value) {
                buf[pos] = '0';
            }

            timeout = 0; timeout_timer = 300;
        }

        LCD_CMD(LCD_CURSOR_OFF);

        LCD_CMD(cursor_pos);
        LCD_Write_String8(buf, max_len, LCD_ALIGN_LEFT);
        LCD_CMD(cursor_pos + pos);

        LCD_CMD(LCD_UNDERLINE_ON);
    
        while (screen_refresh == 0 && timeout == 0);
    }
    key1_press = 0;
    key2_press = 0;
    
    LCD_CMD(LCD_CURSOR_OFF);

    return strtoul(buf, NULL, 10);
}

unsigned char edit_value_bits(unsigned char v, char* str) {
    // convert value
    add_leading_symbols((char*) tbuf, '0', ultoa2((char*) tbuf, v, 2), 8);
    
    unsigned char cursor_pos = 0xC4;
    unsigned char pos = 0;

    timeout = 0; timeout_timer = 300;
    while (timeout == 0) {
        screen_refresh = 0;
        
        // change cursor position
        if (key1_press == 1) {
            key1_press = 0;
            if (++pos == 8) {
                pos = 0;
            }
            timeout = 0; timeout_timer = 300;
        }
        
        // edit number in cursor position
        if (key2_press == 1) {
            key2_press = 0;
            
            v ^= (1 << (7 - pos));
            // invert bit
            tbuf[pos] = ('1' - tbuf[pos]) + '0';
            
            timeout = 0; timeout_timer = 300;
        }

        LCD_CMD(LCD_CURSOR_OFF);

        memset(buf, ' ', 16);
        strcpy2((char*) buf, (char *)str, pos + 1);
        // print on/off
        strcpy2((char*) &buf[12], (char *) &onoff_string, (tbuf[pos] - '0') + 1);
        LCD_CMD(0x80);
        LCD_Write_String16(buf, 16, LCD_ALIGN_LEFT);
        
        LCD_CMD(cursor_pos);
        LCD_Write_String8((char*) tbuf, 8, LCD_ALIGN_LEFT);

        LCD_CMD(cursor_pos + pos);
        LCD_CMD(LCD_UNDERLINE_ON);

        while (screen_refresh == 0 && timeout == 0);
    }
    key1_press = 0;
    key2_press = 0;
    
    LCD_CMD(LCD_CURSOR_OFF);

    return v;
}

void screen_service_fuel_constant(screen_service_item_t* item) {
    config.fuel_const = edit_value_char(config.fuel_const, false);
}

void screen_service_vss_constant(screen_service_item_t* item) {
    config.odo_const = (unsigned short) edit_value_long(config.odo_const, 29999L);
}

void screen_service_total_trip(screen_service_item_t* item) {
    config.odo = (uint32_t) edit_value_long((unsigned long) config.odo, 999999L);
}

void screen_service_settings_bits(screen_service_item_t* item) {
    config.settings.byte = edit_value_bits(config.settings.byte, (char *) &settings_bits);
}

#ifdef USE_DS18B20
void screen_service_temp_sensors(screen_service_item_t* item) {
    
    memset(tbuf, 0xFF, 8);
    ds18b20_read_rom((unsigned char*) tbuf);
    ds18b20_serial_to_string(tbuf, (unsigned char*) buf);
    
    unsigned char t_num = 0;
    
    timeout = 0; timeout_timer = 300;
    while (timeout == 0) {
        screen_refresh = 0;
        if (key1_press == 1 || key2_press == 1) {
            key1_press = 0;
            key2_press = 0;
            t_num++;
            if (t_num >= 4) {
                t_num = 0;
            }
            timeout = 0; timeout_timer = 300;
        }
        len = strcpy2(&buf[12], (char *) &temp_sensors, t_num + 1);
        add_leading_symbols(&buf[12], ' ', len, 4);
        
        LCD_CMD(0xC0);
        LCD_Write_String16(buf, 16, LCD_ALIGN_LEFT);
        
        while (screen_refresh == 0 && timeout == 0);
    }
    
    if (t_num != 0) {
        // save ds18b20 serial number to eeprom
        HW_write_eeprom_block(tbuf, temps_ee_addr + (t_num - 1) * 8, 8);
    }
}
#endif

void screen_service_service_counters(screen_service_item_t* item) {

    if (key1_press == 1) {
        key1_press = 0;
        c_sub_item++;

        if (c_sub_item == 5) {
            c_sub_item = 0;
        }

        timeout = 0; timeout_timer = 300;
    }

    LCD_CMD(0x80);
    LCD_Write_String16(buf, strcpy2(buf, (char *) item->name, 0), LCD_ALIGN_LEFT);
    LCD_CMD(0xC0);
    LCD_Write_String16(buf, strcpy2(buf, (char *) &service_counters, c_sub_item + 1), LCD_ALIGN_LEFT);

    if (key2_press == 1) {
        key2_press = 0;

        LCD_CMD(0x80);
        LCD_Write_String16(buf, strcpy2(buf, (char *) &service_counters, c_sub_item + 1), LCD_ALIGN_LEFT);
        LCD_CMD(0xC0);
        memset(buf, ' ', 16);
        LCD_Write_String16(buf, 16, LCD_ALIGN_LEFT);

        switch (c_sub_item) {
            case 0:
                services.mh.limit = (unsigned short) edit_value_long(services.mh.limit, 1999L);
                break;
            case 1:
                services.oil_engine.limit = edit_value_char(services.oil_engine.limit, true);
                break;
            case 2:
                services.oil_gearbox.limit = edit_value_char(services.oil_gearbox.limit, true);
                break;
            case 3:
                services.air_filter.limit = edit_value_char(services.air_filter.limit, true);
                break;
            case 4:
                services.sparks.limit = edit_value_char(services.sparks.limit, true);
                break;

        }

        timeout = 0; timeout_timer = 300;
    }
}

void screen_service_ua_const(screen_service_item_t* item) {

    if (key1_press == 1 || key2_press == 1) {
        if (key2_press == 1) {
            key2_press = 0;
            if (config.vcc_const < 230) {
                config.vcc_const++;
            }
        } else {
            key1_press = 0;
            if (config.vcc_const >= 140) {
                config.vcc_const--;
            }
        }
        timeout = 0; timeout_timer = 300;
    }

    LCD_CMD(0xC4);
    print_voltage(LCD_ALIGN_RIGHT);
    
}

void read_eeprom() {
    unsigned char ee_addr = 0;
    
    HW_read_eeprom_block((unsigned char*) &config, ee_addr, sizeof(config_t));
    ee_addr += (sizeof(config_t) / 8 + 1) * 8;
    
    HW_read_eeprom_block((unsigned char*) &trips, ee_addr, sizeof(trips_t));
    ee_addr += (sizeof(trips_t) / 8 + 1) * 8;
    
    HW_read_eeprom_block((unsigned char*) &services, ee_addr, sizeof(services_t));
    temps_ee_addr = ee_addr + (sizeof(services_t) / 8 + 1) * 8;
    
}

void save_eeprom() {
    unsigned char ee_addr = 0;
    HW_write_eeprom_block((unsigned char*) &config, ee_addr, sizeof(config_t));
    ee_addr += (sizeof(config_t) / 8 + 1) * 8;
    
    HW_write_eeprom_block((unsigned char*) &trips, ee_addr, sizeof(trips_t));
    ee_addr += (sizeof(trips_t) / 8 + 1) * 8;
    
    HW_write_eeprom_block((unsigned char*) &services, ee_addr, sizeof(services_t));
}

void power_off() {
    // save and shutdown;
    disable_interrupts();
    
    // save current time
    get_ds_time(&time);
    trips.tripC_time.minute = time.minute;
    trips.tripC_time.hour = time.hour;
    trips.tripC_time.day = time.day;
    trips.tripC_time.month = time.month;
    trips.tripC_time.year = time.year;
    
    save_eeprom();
    
    PWR_OFF; while (1);
}

const unsigned short ydayArray[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

unsigned char check_tripC_time() {
    short diff;

    // clear trip C if diff between dates is more than TRIPC_PAUSE_MINUTES minutes
    get_ds_time(&time);

#ifndef SIMPLE_TRIPC_TIME_CHECK    
    diff = bcd_subtract(time.year, trips.tripC_time.year);
    if (diff < 0 || diff > 1) return 0;
    
    unsigned short yday = ydayArray[bcd8_to_bin(time.month)] + bcd8_to_bin(time.day);
    unsigned short yday_c = ydayArray[bcd8_to_bin(trips.tripC_time.month)] + bcd8_to_bin(trips.tripC_time.day);
    diff = (short) ((diff == 1 ? 365 : 0) + yday - yday_c);
#else    
    diff = bcd_subtract(time.day,trips.tripC_time.day);
#endif    
    if (diff < 0 || diff > 1) return 0;

    diff = (diff == 1 ? 24 : 0) + bcd_subtract(time.hour, trips.tripC_time.hour);
    if (diff < 0) return 0;
    
    diff = 60 * diff + bcd_subtract(time.minute, trips.tripC_time.minute);
    if (diff < TRIPC_PAUSE_MINUTES) return 0;

    return 1;
}

void power_on() {
    HW_Init();
    
    read_eeprom();
    
    _LCD_Init();

    fuel1_const = config.settings.dual_injection ? 130 : 65;
    
    if (check_tripC_time() == 1) {
        // clear tripC
        memset(&trips.tripC, 0, sizeof(trip_t));
        trips.tripC_max_speed = 0;
    }
}

void check_service_counters() {
    unsigned char i;
    for (i = 0; i < 5; i++) {
        unsigned char warn = 0;
        if (i == 0) {
            if (services.mh.limit != 0 && (services.mh.counter / 1800UL) > services.mh.limit) {
                warn = 1;
            }
        } else {
            srv_t* srv;
            switch (i) {
                case 1:
                    srv = &services.oil_engine;
                    break;
                case 2:
                    srv = &services.oil_gearbox;
                    break;
                case 3:
                    srv = &services.air_filter;
                    break;
                case 4:
                    srv = &services.sparks;
                    break;
            }
            if (srv->limit != 0 && srv->counter > (srv->limit * 1000U)) {
                warn = 1;
            }
        }
        if (warn == 1) {
#ifdef USE_SOUND            
            buzzer_fl = 1; buzzer_init_fl = 0; buzzer_mode = &buzzer[BUZZER_WARN];
#endif            
            LCD_CMD(0x80);
            LCD_Write_String16(buf, strcpy2(buf, (char*) &warning_str, 0), LCD_ALIGN_CENTER);

            LCD_CMD(0xC0);
            LCD_Write_String16(buf, strcpy2(buf, (char*) &service_counters, i + 1), LCD_ALIGN_CENTER);

            timeout = 0; timeout_timer = 300;
            while (timeout == 0 && key1_press == 0 && key2_press == 0);
            key1_press = 0; key2_press = 0;
        }
    }
}

#ifdef USE_DS18B20
void handle_temp() {
    temperature_fl = 0;
    if (temperature_conv_fl == 1) {
        // read temperature for ds18b20
        timeout_temperature = TIMEOUT_TEMPERATURE;
        temperature_conv_fl = 0;
        unsigned char i;
        for (i = 0; i < 3; i++) {
            HW_read_eeprom_block((unsigned char *) &tbuf, temps_ee_addr + i * 8, 8);
            ds18b20_read_temp_matchrom((unsigned char *) &tbuf, &temps[i]);
        }
    } else {
        // start conversion for ds18b20
        temperature_conv_fl = 1;
        timeout_temperature = 1;
        ds18b20_start_conversion();
    }
}
#endif

void service_screen(unsigned char c_item) {
    screen_service_item_t item = items_service[c_item];
    
    LCD_CMD(0x80);
    LCD_Write_String16(buf, strcpy2(buf, (char *) &service_menu_title, 0), LCD_ALIGN_LEFT);
    LCD_CMD(0xC0);

    buf[0] = '1' + c_item;
    buf[1] = '.';
    LCD_Write_String16(buf, strcpy2(&buf[2], (char*) item.name, 0) + 2, LCD_ALIGN_LEFT);

    if (key2_press == 1) {
        key2_press = 0;
        LCD_Clear();
        c_sub_item = 0;
        timeout = 0;
        timeout_timer = 300;
        while (timeout == 0) {
            screen_refresh = 0;

            LCD_CMD(0x80);
            LCD_Write_String16(buf, strcpy2(buf, (char*) item.name, 0), LCD_ALIGN_LEFT);

            item.screen(&item);

            while (screen_refresh == 0 && timeout == 0);
        }
        screen_refresh = 1;
    }
}

void handle_misc_values() {
    speed = (unsigned short) ((18000UL * kmh) / config.odo_const);
    if (trips.tripC_max_speed < speed) {
        trips.tripC_max_speed = speed;
    }
    if (drive_fl == 1 && speed == 0) {
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

    unsigned char service_mode = 0;

    power_on();

    if (KEY1_PRESSED) {
        delay_ms(40);
        if (KEY1_PRESSED) {
            service_mode = 1;
        }
    }
    
    // select main or service items
    if (service_mode == 0) {
#ifdef USE_DS18B20        
        temperature_fl = 1;
#endif        
    } else {
        LCD_CMD(0x80);
        LCD_Write_String8(buf, strcpy2(buf, (char *) &service_menu_title, 0), LCD_ALIGN_LEFT);
        while (KEY1_PRESSED);
    }

    start_timer1();
    enable_interrupts();
    
    if (service_mode == 0 && config.settings.service_alarm) {
        check_service_counters();
        key1_press = 0; key2_press = 0; key1_longpress = 0; key2_longpress = 0;
    }
    
    while (1) {
        screen_refresh = 0;

        // check power
        if (shutdown_fl) {
            power_off();
        }

        if (service_mode == 0) {
#ifdef USE_DS18B20        
            if (temperature_fl == 1) {
                handle_temp();
            }
#endif            
            handle_misc_values();
        }        

        // show next screen
        if (key1_press) {
            key1_press = 0;
            key2_press = 0;
            key1_longpress = 0;
            key2_longpress = 0;
            tmp_param = 0;
            do {
                c_item++;
                if ((service_mode == 0 && c_item >= sizeof(items_main) / sizeof(screen_item_t)) 
                    || (service_mode == 1 && c_item >= sizeof(items_service) / sizeof(screen_service_item_t))) {
                    c_item = 0;
                }
            } while (service_mode == 0 && drive_fl == 1 && items_main[c_item].drive_mode == 0);
            LCD_Clear();
        }
        
        if (service_mode == 1) {
            service_screen(c_item);
        } else {
            items_main[c_item].screen();
        }
        
        while (screen_refresh == 0);
    }
}

