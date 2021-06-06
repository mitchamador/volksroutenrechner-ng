
/********************************************************************************************
* PIC16F876A 
* LCD 16x2 with 4 bits
*********************************************************************************************/

#include <xc.h>

#include "hw.h"
#include <stdbool.h>
#include <string.h>
#include "lcd.h"
#include "ds1307.h"
#include "ds18b20.h"
#include "utils.h"

#if !defined(_16F876A) && !defined(_16F1936)
#error "only pic16f876a or pic 16f1936 supported"
#endif

// use ds18b20 temperature sensors
#define USE_DS18B20

// place custom chars data in eeprom
#define EEPROM_CUSTOM_CHARS

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
  unsigned char byte;
  // a structure with 8 single bit bit-field objects, overlapping the union member "byte"
  struct {
    unsigned b0:1;
    unsigned b1:1;
    unsigned b2:1;
    unsigned service_alarm:1;
    unsigned engine_alarm:1;
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

//=======================================================================================
// eeprom data

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
__EEPROM_DATA(0x7F,0x9F,0x04,0x00,0x16,0x13,0x80,0x3E);
__EEPROM_DATA(0x6E,0xA6,0x80,0x01,0x00,0x00,0x00,0x00);
trips_t trips;
__EEPROM_DATA(0x0A,0x00,0x90,0x1B,0x1A,0x28,0x64,0x00);
__EEPROM_DATA(0x02,0x02,0x00,0x00,0x22,0x01,0xDD,0x3A);
__EEPROM_DATA(0x3D,0x64,0x11,0x0F,0xC8,0x4F,0x00,0x00);
__EEPROM_DATA(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
__EEPROM_DATA(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
__EEPROM_DATA(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
services_t services;
__EEPROM_DATA(0xBA,0x15,0x03,0x00,0x00,0x00,0x16,0x0A);
__EEPROM_DATA(0x0A,0x22,0x01,0x21,0x99,0x13,0x0A,0xFF);
__EEPROM_DATA(0xFF,0xFF,0x99,0x13,0x28,0x21,0x03,0x20);
__EEPROM_DATA(0xAA,0x0A,0x28,0x07,0x01,0x20,0x00,0x00);
// ds18b20 serial numbers (OUT, IN, ENGINE))
__EEPROM_DATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
__EEPROM_DATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
__EEPROM_DATA(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);

//========================================================================================

// key variables and flags
unsigned char key1_counter, key2_counter;
volatile __bit key1_press, key1_longpress, key2_press, key2_longpress;

// main interval
unsigned char main_interval_counter;
__bit screen_refresh;

// timeout
volatile unsigned int timeout_timer;
volatile __bit timeout;

#ifdef USE_DS18B20
volatile unsigned char timeout_temperature;
volatile __bit temperature_fl, temperature_conv_fl;
#endif

// misc flags
volatile __bit odom_fl, drive_fl, motor_fl, fuel_fl, taho_fl, taho_measure_fl;

// speed100 flags and variables
volatile __bit speed100_fl, speed100_measure_fl, speed100_ok_fl, speed100_timer_fl;
volatile unsigned short speed100_const, speed100_timer;

volatile unsigned short kmh_temp, kmh;
volatile unsigned short fuel_tmp, fuel;
volatile unsigned short taho_tmp, taho;

unsigned short speed;

#ifdef USE_DS18B20
// ds18b20 temperatures and eeprom pointer
#define TEMP_OUT 0
#define TEMP_IN 1
#define TEMP_ENGINE 2
uint16_t temps[3] = {0, 0, 0};
unsigned char temps_ee_addr;
#else
unsigned char data_ee_addr;
#endif

unsigned char tbuf[8];
unsigned char tmp_param = 0;

// buffer for strings
char buf[16];
//char buf2[16];
unsigned char len = 0;

ds_time time;

unsigned char fuel1_const;

#define HOUR_SYMBOL 'h'
#define LITRE_SYMBOL 'l'
#define VOLT_SYMBOL 'V'
#define KM1_SYMBOL 'k'
#define KM2_SYMBOL 'm'
#define SECONDS_SYMBOL 's'
#define CELSIUS_SYMBOL 0xDF

// custom lcd characters values
#define _kmh0   0x00
#define _kmh1	0x01
#define _omin0	0x02
#define _omin1	0x03
#define _lkm0	0x04
#define _lkm1	0x05
#define _lh0	0x06
#define _lh1    0x07

const char empty_string[] = "----";
const char no_time_string[] = "-----'--";
const char trip_string[] = "trip ";
const char onoff_string[] = "\0 OFF\0  ON";
const char time_correction[] = "time correction?";
const char reset_string[] = "reset?"; 
const char speed100_string[] = "0-100 timing"; 
const char speed100_wait_string[] = "wait for start"; 
const char timeout_string[] = "timeout"; 

const char service_menu_title[] = "SERVICE MENU";
const char service_counters[] = "\0engine hours\0engine oil\0gearbox oil\0air filter\0spark plugs";
const char settings_bits[] = "\0dual inj\0skip temp\0key sound\0t'engn alarm\0serv alarm\0\0\0";
const char temp_sensors[] = "\0---\0out\0in\0eng";

const char fuel_constant_str[] = "FUEL constant";
const char vss_constant_str[] = "VSS constant";
const char total_trip_str[] = "total trip";
const char voltage_adjust_str[] = "voltage adjust";
const char settings_bits_str[] = "settings bits";
const char temp_sensor_str[] = "temp sensors";
const char service_counters_str[] = "service cntrs";

const char day_of_week_str[] = "\0sunday\0monday\0tuesday\0wednesday\0thursday\0friday\0saturday";
const char month_str[] = "\0jan\0feb\0mar\0apr\0may\0jun\0jul\0aug\0sep\0oct\0nov\0dec";

#ifdef EEPROM_CUSTOM_CHARS
#define CUSTOM_CHAR_DATA(a0,a1,a2,a3,a4,a5,a6,a7) __EEPROM_DATA(a0,a1,a2,a3,a4,a5,a6,a7);
#else
#define CUSTOM_CHAR_DATA(a0,a1,a2,a3,a4,a5,a6,a7) a0,a1,a2,a3,a4,a5,a6,a7,
#endif

#ifndef EEPROM_CUSTOM_CHARS
// custom lcd characters data
const unsigned char custom_chars[] = {
#endif    
CUSTOM_CHAR_DATA(0x05, 0x06, 0x05, 0x00, 0x00, 0x00, 0x01, 0x02) // kmh[0]
CUSTOM_CHAR_DATA(0x0A, 0x15, 0x15, 0x00, 0x10, 0x05, 0x07, 0x01) // kmh[1]
CUSTOM_CHAR_DATA(0x07, 0x05, 0x07, 0x00, 0x00, 0x01, 0x02, 0x00) // omin[0]
CUSTOM_CHAR_DATA(0x00, 0x00, 0x08, 0x10, 0x00, 0x0A, 0x15, 0x15) // omin[1]
CUSTOM_CHAR_DATA(0x0C, 0x14, 0x14, 0x01, 0x02, 0x05, 0x01, 0x01) // L100[0]
CUSTOM_CHAR_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x15, 0x1F) // L100[1]
CUSTOM_CHAR_DATA(0x03, 0x05, 0x05, 0x00, 0x00, 0x01, 0x02, 0x00) // l/h[0]
CUSTOM_CHAR_DATA(0x00, 0x00, 0x08, 0x10, 0x00, 0x14, 0x1C, 0x04) // l/h[1]
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
} screen_item_t;

const screen_item_t items_main[] = {
    {screen_main},
    {screen_tripC},
#ifdef USE_DS18B20
    {screen_temp},
#endif
    {screen_tripA},
    {screen_tripB},
    {screen_time},
    {screen_service_counters},
};

typedef struct screen_service_item_t screen_service_item_t;
typedef void (*screen_service_func) (screen_service_item_t *);

struct screen_service_item_t {
    char* name;
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
    {(char*) &fuel_constant_str, (screen_service_func) &screen_service_fuel_constant},
    {(char*) &vss_constant_str, (screen_service_func) &screen_service_vss_constant},
    {(char*) &total_trip_str, (screen_service_func) &screen_service_total_trip},
    {(char*) &voltage_adjust_str, (screen_service_func) &screen_service_ua_const},
    {(char*) &settings_bits_str, (screen_service_func) &screen_service_settings_bits},
#ifdef USE_DS18B20
    {(char*) &temp_sensor_str, (screen_service_func) &screen_service_temp_sensors},
#endif    
    {(char*) &service_counters_str, (screen_service_func) &screen_service_service_counters},
};

unsigned char c_item = 0, c_sub_item = 0;

unsigned char request_screen(unsigned char);

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
                if (speed100_measure_fl == 1) {
                    if (speed100_fl == 1) {
                       // stop timer2
                        stop_timer_taho();
                        if (taho_tmp < speed100_const) {
                            speed100_ok_fl = 1;
                            speed100_timer_fl = 0;
                        }
                        speed100_fl = 0;
                        speed100_measure_fl = 0;
                    } else {
                        // start timer2
                        taho_tmp = 0;
                        start_timer_taho();
                        speed100_fl = 1;
                    }
                }

                kmh_temp++;

                // handmade xc8 optimizing
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
        
        // handmade xc8 optimizing
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
            }
        } else // key released
        {
            if (key1_counter > DEBOUNCE && key1_counter < SHORTKEY) {
                // key press
                key1_press = 1;
                screen_refresh = 1;
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
            }
        } else // key released
        {
            if (key2_counter > DEBOUNCE && key2_counter < SHORTKEY) {
                // key press
                key2_press = 1;
                screen_refresh = 1;
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
             kmh = kmh_temp;
             kmh_temp = 0;
             
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
            speed100_measure_fl = 0;
            speed100_fl = 0;
            motor_fl = 0;
            taho = 0;
        }
    int_handler_timer2_end    

int_handler_GLOBAL_end

void _LCD_Init(void) {
    LCD_Init(0x40);    // Initialize LCD module with I2C address = 0x40 ((0x20<<1) for PCF8574) or 0x70 ((0x38<<1) for PCF8574A)
    
    // LCD set custom characters
    unsigned char i = 0;
#ifdef EEPROM_CUSTOM_CHARS
    for (i = 0; i < 64; i = i + 8) {
        LCD_CMD(LCD_SETCGRAMADDR | (i & ~0x07));
#ifdef USE_DS18B20
        HW_read_eeprom_block((unsigned char*) buf, temps_ee_addr + 24 + i, 8);
#else        
        HW_read_eeprom_block((unsigned char*) buf, data_ee_addr + i, 8);
#endif
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

void print_current_time_hm(unsigned char hour, unsigned char minute, bool right_align) {
    bcd8_to_str(buf, hour);
    buf[2] = ':';
    bcd8_to_str(&buf[3], minute);
    LCD_Write_String8(buf, 5, right_align);
}

void print_current_time_dmy(unsigned char day, unsigned char month, unsigned char year) {
    if (day == 0x00 || day == 0xFF) {
        memcpy(buf, &no_time_string, 8);
    } else {
        bcd8_to_str(buf, day);
        strcpy2(&buf[2], (char *) &month_str, bcd8_to_bin(month));
        buf[5] = '\'';
        bcd8_to_str(&buf[6], year);
    }
    LCD_Write_String8(buf, 8, false);

}
void print_current_time(ds_time* time) {

    LCD_CMD(0x80);
    print_current_time_hm(time->hour, time->minute, false);

    LCD_CMD(0x88);
    print_current_time_dmy(time->day, time->month, time->year);

    LCD_CMD(0xc0);
    LCD_Write_String16(buf, strcpy2((char *)buf, (char*) day_of_week_str, time->day_of_week), false);
}

void screen_time(void) {

    get_ds_time(&time);
    if (request_screen(strcpy2(buf,(char *) &time_correction, 0)) == 1) {

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
    unsigned char len = utoa2(buf, (unsigned int) num, 10);

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
 * @param right_align
 */
void print_trip_time(trip_t* t, bool right_align) {
    
    unsigned short time = (unsigned short) (t->time / 30);
    
    len = utoa2(buf, (unsigned short) (time / 60), 10);
    
    buf[len++] = ':';
    bcd8_to_str(&buf[len], time % 60);
    len += 2;

    LCD_Write_String8(buf, len, right_align);
}

/**
 * show trip average speed
 * @param t
 * @param right_align
 */
void print_trip_average_speed(trip_t* t, bool right_align) {
    
    unsigned short speed = 0;
    if (t->time > 0) {
        speed = (unsigned short) ((unsigned long) ((t->odo * 18000UL) + (t->odo_temp * 18000UL / config.odo_const)) / t->time);
    }
    
    if (speed == 0) {
        memcpy(buf, &empty_string, 4);
        len = 4;
    } else {
        len = get_fractional_string(buf, speed);
        buf[len++] = _kmh0;
        buf[len++] = _kmh1;
    }

    LCD_Write_String8(buf, len, right_align);

}

/**
 * show trip odometer (km)
 * @param t
 * @param right_align
 */
void print_trip_odometer(trip_t* t, bool right_align) {
    
    unsigned short odo = (unsigned short) ((unsigned long) (t->odo * 10UL) + (t->odo_temp * 10UL / config.odo_const));
    
    len = get_fractional_string(buf, odo);
    buf[len++] = KM1_SYMBOL;
    buf[len++] = KM2_SYMBOL;
   
    LCD_Write_String8(buf, len, right_align);

}

/**
 * show trip total fuel consumption (l)
 * @param t
 * @param right_align
 */
void print_trip_total_fuel(trip_t* t, bool right_align) {
    len = get_fractional_string(buf, t->fuel / 10);
    buf[len++] = LITRE_SYMBOL;
    LCD_Write_String8(buf, len, right_align);
}

/**
 * show trip average fuel consumption (l/100km)
 * @param t
 * @param right_align
 */
void print_trip_average_fuel(trip_t* t, bool right_align) {
    
    if (t->fuel < AVERAGE_MIN_FUEL) {
        memcpy(buf, &empty_string, 4);
        len = 4;
    } else {
        unsigned short odo = (unsigned short) ((unsigned long) (t->odo * 10UL) + (t->odo_temp * 10UL / config.odo_const));
        if (odo < AVERAGE_MIN_DIST) {
            memcpy(buf, &empty_string, 4);
            len = 4;
        } else {
            len = get_fractional_string(buf, (unsigned short) (t->fuel * 100UL / odo));
            buf[len++] = _lkm0;
            buf[len++] = _lkm1;
        }
    }
    LCD_Write_String8(buf, len, right_align);
}

void print_speed(unsigned short speed, bool right_align) {
    if (speed > 1000) {
        // more than 100 km/h, skip fractional
        len = utoa2(buf, (unsigned int) speed, 10) - 1;
    } else {
        // lower than 100 km/h, use fractional
        len = get_fractional_string(buf, speed);
    }

    buf[len++] = _kmh0;
    buf[len++] = _kmh1;

    LCD_Write_String8(buf, len, right_align);
}

void print_taho(bool right_align) {
    taho_measure_fl = 1;
    timeout = 0; timeout_timer = TAHO_TIMEOUT_TIMER;
    while (motor_fl == 1 && taho_measure_fl == 1 && timeout == 0)
        ;
    if (timeout == 1) {
        motor_fl = 0;
    }

    if (taho == 0) {
        memcpy(buf, &empty_string, 4);
        len = 4;
    } else {
        unsigned short res = (unsigned short) ((TAHO_CONST / taho));
#ifdef TAHO_ROUND
        res = res / TAHO_ROUND * TAHO_ROUND;
#endif        
        len = utoa2(buf, (unsigned int) res, 10);
        buf[len++] = _omin0;
        buf[len++] = _omin1;
    }
    LCD_Write_String8(buf, len, right_align);
}

void print_current_fuel_km(bool right_align) {
    unsigned short odo_con4 = config.odo_const * 2 / (config.settings.dual_injection ? 26 : 13);
    unsigned short t = (unsigned short) ((((unsigned long) fuel * (unsigned long) odo_con4) / (unsigned long) kmh) / (unsigned char) config.fuel_const);
    len = get_fractional_string(buf, t);
    buf[len++] = _lkm0;
    buf[len++] = _lkm1;
    LCD_Write_String8(buf, len, right_align);
}

void print_current_fuel_lh(bool right_align) {
    len = get_fractional_string(buf, fuel * (config.settings.dual_injection ? 14UL : 28UL) / config.fuel_const / 10);
    buf[len++] = _lh0;
    buf[len++] = _lh1;
    LCD_Write_String8(buf, len, right_align);
}

void print_main_odo(bool right_align) {
    len = ultoa2(buf, (unsigned long) config.odo);
    buf[len++] = KM1_SYMBOL;
    buf[len++] = KM2_SYMBOL;
    LCD_Write_String8(buf, len, right_align);
}

#ifdef USE_DS18B20
void print_temp(unsigned char index, bool header, bool right_align) {
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
        len = utoa2(buf, _t / 10, 10);
        buf[len++] = CELSIUS_SYMBOL;
        LCD_Write_String8(buf, len, right_align);
    } else {
        len = get_fractional_string(&buf[1], _t) + 1;
        if (header) {
            add_leading_symbols(buf, ' ', len, 8);
            strcpy2(buf, (char *) &temp_sensors, (index + 1) + 1);
            len = 8;
        }
        
        LCD_Write_String8(buf, len, right_align);
    }
}
#endif

void print_voltage(bool right_align) {
    uint16_t adc_voltage = HW_adc_read();
    len = get_fractional_string(buf, (unsigned short) adc_voltage * 30UL / config.vcc_const);
    buf[len++] = VOLT_SYMBOL;
    LCD_Write_String8(buf, len, right_align);
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

void print_selected_param2(bool right_align) {
    switch (select_param(&config.selected_param2, 4)) {
        case 0:
            print_trip_total_fuel(&trips.tripC, true);
            break;
        case 1:
            print_trip_time(&trips.tripC, right_align);
            break;
        case 2:
            print_trip_average_speed(&trips.tripC, right_align);
            break;
        case 3:
            print_trip_average_fuel(&trips.tripC, right_align);
            break;
        case 4:
            print_speed(trips.tripC_max_speed, right_align);
            break;
    }
}

#ifdef DS18B20
#define COUNT_SELECTED_PARAM1 4
#else
#define COUNT_SELECTED_PARAM1 3
#endif

void print_selected_param1(bool right_align) {
    switch (select_param(&config.selected_param1, COUNT_SELECTED_PARAM1)) {
#ifdef DS18B20
        case COUNT_SELECTED_PARAM1 - 4:
            print_temp(TEMP_OUT, false, right_align);
            break;
#endif
        case COUNT_SELECTED_PARAM1 - 3:
            print_voltage(right_align);
            break;
        case COUNT_SELECTED_PARAM1 - 2:
            print_trip_odometer(&trips.tripC, right_align);
            break;
        case COUNT_SELECTED_PARAM1 - 1:
            print_trip_average_fuel(&trips.tripC, right_align);
            break;
    }
}

void screen_main(void) {
//; первый экран

    LCD_CMD(0x80);
    if (drive_fl == 0 || speed < MIN_SPEED) {
//; 1) на месте с заглушенным двигателем
//; время текущее       общий пробег (км)
//; нар.темп./пробег C  вольтметр
        get_ds_time(&time);
        print_current_time_hm(time.hour, time.minute, false);

        if (motor_fl == 0) {
            print_main_odo(true);

            LCD_CMD(0xC0);
#ifdef USE_DS18B20            
            print_temp(TEMP_OUT, false, false);
#else
            print_trip_odometer(&trips.tripC, false);
#endif            
            print_voltage(true);
        } else {
//; 2) на месте с работающим двигателем
//; время текущее       тахометр (об/мин)
//; selected_param1 	мгновенный расход (л/ч)
            print_taho(true);
            LCD_CMD(0xC0);
            print_selected_param1(false);
            print_current_fuel_lh(true);
        }
    } else {
//; 3) в движении
//; скорость (км/ч)     тахометр (об/мин)
//; selected_param1 	мгновенный расход (л/100км)
        print_speed(speed, false);
        print_taho(true);

        LCD_CMD(0xC0);
        print_selected_param1(false);
        print_current_fuel_km(true);
    }
    
    if (request_screen(strcpy2(buf, (char *) &speed100_string, 0)) == 1) {
        
        len = strcpy2(buf, (char *) &speed100_wait_string, 0);
        LCD_CMD(0xC0 + (16 - len) / 2U);
        __LCD_Write_String(buf, len, len, false);
        
        LCD_CMD(0xC0);
        memset(buf, ' ', 16);
        
        // 36000000 / 80us / odo_const
        speed100_const = (unsigned short) (450000UL / config.odo_const);
        timeout = 0; timeout_timer = 3000;
        speed100_ok_fl = 0; speed100_timer = 0;
        while (timeout == 0 && speed100_ok_fl == 0) {
            if (drive_fl == 1) {
                if (speed100_timer_fl == 0) {
                    speed100_timer_fl = 1;
                    LCD_Write_String16(buf, 16, false);
                }
                speed100_measure_fl = 1;

//                len = get_fractional_string(buf, speed100_timer / 10);
//                buf[len++] = SECONDS_SYMBOL;
//                LCD_CMD(0xC4);
//                LCD_Write_String8(buf, len, true);

                while (speed100_measure_fl == 1 && timeout == 0 && speed100_ok_fl == 0)
                    ;

            }
        }
        speed100_timer_fl = 0;
        if (speed100_ok_fl) {
            // достигнута скорость 100 км/ч
            len = get_fractional_string(buf, speed100_timer / 10);
            buf[len++] = SECONDS_SYMBOL;
        } else {
            // timeout
            len = strcpy2(buf, (char *) &timeout_string, 0);
        }
        LCD_CMD(0xC4);
        LCD_Write_String8(buf, len, true);
        
        timeout = 0; timeout_timer = 500; while (timeout == 0);
    }
    
}

/**
 * 
 * @param _len
 * @return 
 */
unsigned char request_screen(unsigned char len) {
    unsigned char reset = 0;
    if (key2_longpress == 1) {
        key2_longpress = 0;
        
        LCD_Clear();
        LCD_CMD(0x80 + ((16 - len) / 2U));
        __LCD_Write_String(buf, len, len, false);

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
    if (request_screen(strcpy2(buf, (char *) &reset_string, 0)) == 1) {
        memset(trip, 0, sizeof(trip_t));
    }
}

void screen_tripC(void) {
//; второй экран
    LCD_CMD(0x80);
    if (drive_fl == 0 || speed < MIN_SPEED) {
        print_trip_average_fuel(&trips.tripC, false);
        if (motor_fl == 0) {
//; 1) на месте с заглушенным двигателем
//; средний расход C (л/100км)  общий пробег
//; пробег C (км)               selected_param2
            print_main_odo(true);
        } else {
//; 2) на месте с работающим двигателем
//; средний расход C (л/100км)  тахометр (об/мин)	
//; пробег C (км)               selected_param2
            print_taho(true);
        }
    } else {
//; 3) в движении
//; скорость (км/ч)             тахометр (об/мин)
//; пробег C (км)               selected_param2
        print_speed(speed, false);
        print_taho(true);
    }

    LCD_CMD(0xC0);
    print_trip_odometer(&trips.tripC, false);
    print_selected_param2(true);
    
    clear_trip(&trips.tripC);
}

void screen_tripAB(trip_t* trip, unsigned char ch) {
    LCD_CMD(0x80);

    len = strcpy2(buf, (char *) &trip_string, 0);
    buf[len++] = ch;
    LCD_Write_String8(buf, len, false);

    print_trip_odometer(trip, true);
    
    LCD_CMD(0xC0);
    switch (select_param(&tmp_param, 2)) {
        case 0:
            print_trip_average_fuel(trip, false);
            print_trip_average_speed(trip, true);
            break;
        case 1:
            print_trip_time(trip, false);
            print_trip_total_fuel(trip, true);
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
        print_temp(TEMP_OUT, true, true);
        LCD_CMD(0xC0);
        print_temp(TEMP_IN, true, true);
        LCD_CMD(0xC8);
        print_temp(TEMP_ENGINE, false, true);
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
    LCD_Write_String16(buf, strcpy2((char*)buf, (char *) &service_counters, tmp_param + 1), false);

    LCD_CMD(0xC0);
    if (tmp_param == 0) {
        s_time = services.oil_engine.time;
        v = (unsigned short) (services.mh.counter / 1800L);
    } else {
        s_time = srv->time;
        v = srv->counter;
    }
    len = utoa2(buf, v, 10);
    if (tmp_param == 0) {
        buf[len++] = HOUR_SYMBOL;
    } else {
        buf[len++] = KM1_SYMBOL;
        buf[len++] = KM2_SYMBOL;
    }
    __LCD_Write_String(buf, len, 7, true);
    
    LCD_CMD(0xC8);
    print_current_time_dmy(s_time.day, s_time.month, s_time.year);
    
    if (request_screen(strcpy2(buf, (char *) &reset_string, 0)) == 1) {
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
        
        len = utoa2(buf, thousands ? (unsigned short) (v * 1000) : v , 10);
        
        LCD_CMD(0xC4);
        LCD_Write_String8(buf, len, true);

        while (screen_refresh == 0 && timeout == 0);
    }
    key1_press = 0;
    key2_press = 0;

    return v;
}

unsigned long edit_value_long(unsigned long v, unsigned long max_value) {
    // number of symbols to edit
    unsigned char max_len = ultoa2(buf, max_value);;
    
    if (v > max_value) {
        v = max_value;
    }
    
    // convert value
    unsigned char v_len = ultoa2(buf, v);
    
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
        LCD_Write_String8(buf, max_len, false);
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
    add_leading_symbols((char*) tbuf, '0', utoa2((char*) tbuf, v, 2), 8);
    
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
            if (tbuf[pos] == '0') {
                tbuf[pos] = '1';
            } else {
                tbuf[pos] = '0';
            }
            
            timeout = 0; timeout_timer = 300;
        }

        LCD_CMD(LCD_CURSOR_OFF);

        memset(buf, ' ', 16);
        strcpy2((char*) buf, (char *)str, pos + 1);
        unsigned char _onoff_index = 0;
        if (tbuf[pos] == '1') {
            _onoff_index++;
        }
        strcpy2((char*) &buf[12], (char *) &onoff_string, _onoff_index + 1);
        LCD_CMD(0x80);
        LCD_Write_String16(buf, 16, false);
        
        LCD_CMD(cursor_pos);
        LCD_Write_String8((char*) tbuf, 8, false);

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
        LCD_Write_String16(buf, 16, false);
        
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
    LCD_Write_String16(buf, strcpy2(buf, item->name, 0), false);
    LCD_CMD(0xC0);
    LCD_Write_String16(buf, strcpy2(buf, (char *) &service_counters, c_sub_item + 1), false);

    if (key2_press == 1) {
        key2_press = 0;

        LCD_CMD(0x80);
        LCD_Write_String16(buf, strcpy2(buf, (char *) &service_counters, c_sub_item + 1), false);
        LCD_CMD(0xC0);
        memset(buf, ' ', 16);
        LCD_Write_String16(buf, 16, false);

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

void screen_service_ua_const() {

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
    print_voltage(true);
    
}

void read_eeprom() {
    unsigned char ee_addr = 0;
    
    HW_read_eeprom_block((unsigned char*) &config, ee_addr, sizeof(config_t));
    ee_addr += (sizeof(config_t) / 8 + 1) * 8;
    
    HW_read_eeprom_block((unsigned char*) &trips, ee_addr, sizeof(trips_t));
    ee_addr += (sizeof(trips_t) / 8 + 1) * 8;
    
    HW_read_eeprom_block((unsigned char*) &services, ee_addr, sizeof(services_t));
#ifdef USE_DS18B20
    temps_ee_addr = ee_addr + (sizeof(services_t) / 8 + 1) * 8;
#else
    data_ee_addr = ee_addr + (sizeof(services_t) / 8 + 1) * 8;
#endif
    
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
    
    if (POWER_SUPPLY_ACTIVE) {
        PWR_ON;
    }
    
    read_eeprom();
    
    _LCD_Init();

    fuel1_const = config.settings.dual_injection ? 130 : 65;
    
    if (check_tripC_time() == 1) {
        // clear tripC
        memset(&trips.tripC, 0, sizeof(trip_t));
        trips.tripC_max_speed = 0;
    }
}

void main()
{
    unsigned char service_mode = 0;

    power_on();

    if (KEY1_PRESSED) {
        __delay_ms(40);
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
        LCD_Write_String8(buf, strcpy2(buf, (char *) &service_menu_title, 0), false);
        while (KEY1 == 0);
    }
    
    enable_interrupts();
    
    while (1) {
        screen_refresh = 0;

        // check power
        if (!POWER_SUPPLY_ACTIVE) {
            __delay_us(40);
            if (!POWER_SUPPLY_ACTIVE) {
                power_off();
            }
        }

        if (service_mode == 0) {
#ifdef USE_DS18B20        
            if (temperature_fl == 1) {
                temperature_fl = 0;
                if (temperature_conv_fl == 1) {
                    // read temperature for ds18b20
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

        // show next screen
        if (key1_press) {
            key1_press = 0;
            key2_press = 0;
            key1_longpress = 0;
            key2_longpress = 0;
            tmp_param = 0;
            c_item++;
            if ((service_mode == 0 && c_item >= sizeof(items_main) / sizeof(screen_item_t)) 
                    || (service_mode == 1 && c_item >= sizeof(items_service) / sizeof(screen_service_item_t))) {
                c_item = 0;
            }
            LCD_Clear();
        }
        
        if (service_mode == 1) {
            screen_service_item_t item = items_service[c_item];
            LCD_CMD(0x80);
            LCD_Write_String16(buf, strcpy2(buf, (char *) &service_menu_title, 0), false);
            LCD_CMD(0xC0);

            buf[0] = '1' + c_item;
            buf[1] = '.';
            LCD_Write_String16(buf, strcpy2(&buf[2], item.name, 0) + 2, false);

            if (key2_press == 1) {
                key2_press = 0;
                LCD_Clear();
                c_sub_item = 0;
                timeout = 0; timeout_timer = 300;
                while (timeout == 0) {
                    screen_refresh = 0;

                    LCD_CMD(0x80);
                    LCD_Write_String16(buf, strcpy2(buf, item.name, 0), false);

                    item.screen(&item);

                    while (screen_refresh == 0 && timeout == 0);
                }
                screen_refresh = 1;
            }

        } else {
            items_main[c_item].screen();
        }
        
        while (screen_refresh == 0);
    }
}

