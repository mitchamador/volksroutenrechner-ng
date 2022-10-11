#include "main.h"
#include "locale.h"
#include "eeprom.h"
#include "lcd.h"
#include "ds3231.h"
#include "ds18b20.h"
#include "i2c-eeprom.h"
#include "utils.h"
#include "journal.h"
#include <stdbool.h>
#include <string.h>

//========================================================================================

// buffer for strings (LCD_WIDTH + 4 bytes for prefix/index (iii.))
char buf[LCD_WIDTH + 4];

__bit drive_min_speed_fl;

#ifdef TEMPERATURE_SUPPORT
__bit temperature_conv_fl;

uint16_t _t;
uint16_t temps[4] = {DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE};
#endif

typedef struct {
    uint8_t main;
    uint8_t main_add;
    uint8_t tmp;
} params_t;

params_t params = {0, 0, 0};

#ifdef SERVICE_COUNTERS_SUPPORT
uint8_t service_param = 0;
#endif
        
uint8_t fuel2_const;
uint16_t odo_con4;

uint16_t speed;

#ifdef CONTINUOUS_DATA_SUPPORT
__bit drive_min_cd_speed_fl;
uint16_t cd_speed;
#endif

screen_item_t *current_item_main;
screen_config_item_t *current_item_config;

void wait_refresh_timeout(void);
void wait_timeout(uint8_t timeout);

void screen_main(void);
void screen_tripC(void);
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
    drive_mode_screen_max
} drive_mode_screens;

typedef enum {
    SCREEN_INDEX_MAIN = 0,
    SCREEN_INDEX_TRIP_C,
    SCREEN_INDEX_TRIP_A,
    SCREEN_INDEX_TRIP_B,
    SCREEN_INDEX_TIME,
    SCREEN_INDEX_SERVICE_COUNTERS,
    SCREEN_INDEX_JOURNAL
} screen_item_index;

screen_item_t items_main[] = {
    { .screen = screen_main, .page.index = SCREEN_INDEX_MAIN, .page.config_switch = 1, .page.drive_mode = 1},
    { .screen = screen_trip, .page.index = SCREEN_INDEX_TRIP_C, .page.drive_mode = 1},
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

screen_config_item_t items_config[] = {
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

uint8_t request_screen(char *);

void config_screen();

uint16_t get_speed(uint16_t);

void journal_save_trip(trip_t *trip);
void journal_save_accel(uint8_t index);

void save_eeprom(void);
void save_eeprom_trips(void);
void save_eeprom_config(void);

#define timeout_timer1_loop(timeout) timeout_timer1 = timeout; while (screen_refresh = 0, timeout_timer1 != 0)

void wait_refresh_timeout() {

    if (key2_longpress != 0) {
        screen_refresh = 1;
        timeout_timer1 = 0;
    }

    clear_keys_state();

    LCD_flush_buffer();

    while (timeout_timer1 != 0 && screen_refresh == 0);
}

void wait_timeout(uint8_t timeout) {
    
    LCD_flush_buffer();

    timeout_timer1 = timeout;
    while (timeout_timer1 != 0 && no_key_pressed());
}

/**
 * print symbols from symbols_str with [index] in buf at [len] position
 * @param len
 * @param index
 * @return 
 */
#define print_symbols_str(len, index) strcpy2(&buf[len], (char *)symbols_array, index)

/**
 * print aligned string from buffer (lcd full width bytes total length)
 * @param len
 * @param align
 */
void _print_full_width(uint8_t cursor_pos, uint8_t len, align_t align) {
    if (cursor_pos != 0xFF) {
        LCD_cursor_set_position(cursor_pos);
        LCD_Write_String(buf, len, LCD_WIDTH, align);
    }
}

/**
 * print aligned string from buffer (lcd half width bytes total length)
 * @param len
 * @param align
 */
void _print_half_width(uint8_t cursor_pos, uint8_t len, align_t align) {
    if (cursor_pos != 0xFF) {
        LCD_cursor_set_position(cursor_pos);
        LCD_Write_String(buf, len, LCD_WIDTH / 2, align);
    }
}

/**
 * print aligned string with prefix/suffix from buffer (lcd half width bytes total length)
 * @param len
 * @param pos
 * @param align
 * @return 
 */
uint8_t _print_half_width2(uint8_t cursor_pos, uint8_t len, unsigned char pos_prefix, unsigned char pos_suffix, align_t align) {

    // prefix
    uint8_t len_prefix = print_symbols_str(LCD_WIDTH, pos_prefix);

    add_leading_symbols((char *) &buf, ' ', len, len + len_prefix);

    len += len_prefix;

    while (len_prefix-- != 0) {
        buf[len_prefix] = buf[LCD_WIDTH + len_prefix];
    }

    // suffix
    len += print_symbols_str(len, pos_suffix);

    if (cursor_pos != 0xFF) {
        LCD_cursor_set_position(cursor_pos);
        LCD_Write_String(buf, len, LCD_WIDTH / 2, align);
    }
    return len;
}

/**
 * print fractional number [num/10^frac].[num%10^frac]
 * @param num
 * @param frac numbers after '.'
 * @return 
 */
uint8_t print_fract(uint16_t num, uint8_t frac) {
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

uint8_t print_time_hm(uint8_t cursor_pos, uint8_t hour, uint8_t minute, align_t align) {
    bcd8_to_str(buf, hour);
    buf[2] = ':';
    bcd8_to_str(&buf[3], minute);
    _print_half_width(cursor_pos, 5, align);
    return 5;
}

uint8_t print_time_dmy(uint8_t cursor_pos, uint8_t day, uint8_t month, uint8_t year, align_t align) {
    if (day == 0x00 || day == 0xFF) {
        strcpy2(buf, (char*) &no_time_string, 0);
    } else {
        bcd8_to_str(buf, day);
        strcpy2(&buf[2], (char *) &month_array, bcd8_to_bin(month));
        buf[5] = '\'';
        bcd8_to_str(&buf[6], year);
    }
    _print_half_width(cursor_pos, 8, align);
    return 8;
}

uint8_t print_time_dow(uint8_t cursor_pos, uint8_t day_of_week, align_t align) {
    uint8_t len = strcpy2((char *) buf, (char*) day_of_week_array, day_of_week);
    _print_full_width(cursor_pos, len, align);
    return len;
}

/**
 * print trip time
 * @param t
 * @param align
 */
uint8_t print_trip_time(uint8_t cursor_pos, trip_t* t, align_t align) {
    uint16_t time = (uint16_t) (t->time / 60);

    uint8_t len = ultoa2(buf, (uint16_t) (time / 60), 10);
    buf[len++] = ':';

    bcd8_to_str(&buf[len], bin8_to_bcd(time % 60));
    len += 2;
   
    _print_half_width(cursor_pos, len, align);

    return len;
}

/**
 * print trip average speed
 * @param t
 * @param align
 */
uint8_t print_trip_average_speed(uint8_t cursor_pos, trip_t* t, align_t align) {
    uint8_t len;
    uint16_t average_speed = get_trip_average_speed(t);
    
    if (average_speed == 0) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        len = print_fract(average_speed, 1);
    }

    return _print_half_width2(cursor_pos, len, POS_NONE, POS_KMH, align);
}

/**
 * print trip odometer (km)
 * @param t
 * @param align
 */
uint8_t print_trip_odometer(uint8_t cursor_pos, trip_t* t, align_t align) {
    uint16_t odo = get_trip_odometer(t);
    uint8_t len = print_fract(odo, 1);
    return _print_half_width2(cursor_pos, len, POS_NONE, POS_KM, align);
}

/**
 * print trip total fuel consumption (l)
 * @param t
 * @param align
 */
uint8_t print_trip_total_fuel(uint8_t cursor_pos, trip_t* t, align_t align) {
    uint8_t len;
    len = print_fract(t->fuel / 10, 1);
    return _print_half_width2(cursor_pos, len, POS_NONE, POS_LITR, align);
}

/**
 * print trip average fuel consumption (l/100km)
 * @param t
 * @param align
 */
uint8_t print_trip_average_fuel(uint8_t cursor_pos, trip_t* t, align_t align) {
    uint8_t len;
    uint16_t odo = get_trip_odometer(t);
    if (t->fuel < AVERAGE_MIN_FUEL || odo < AVERAGE_MIN_DIST) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        len = print_fract((uint16_t) (t->fuel * 100UL / odo), 1);
    }

    return _print_half_width2(cursor_pos, len, POS_NONE, POS_LKM, align);
}

/**
 * print speed
 * @param speed
 * @param pos_prefix
 * @param frac
 * @param align
 */
uint8_t print_speed(uint8_t cursor_pos, uint16_t speed, uint8_t pos_prefix, uint8_t frac, align_t align) {
    // use fractional by default
    uint8_t len = print_fract(speed, 1);

    if (speed > 1000 || frac == 0) {
        // more than 100 km/h (or current speed), skip fractional
        len -= 2;
    }

    return _print_half_width2(cursor_pos, len, pos_prefix, POS_KMH, align);
}

/**
 * print taho
 * @param align
 */
uint8_t print_taho(uint8_t cursor_pos, align_t align) {
    uint8_t len;
    if (taho_fl == 0) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        unsigned short res = (unsigned short) (((config.settings.par_injection != 0 ? (TAHO_CONST) : (TAHO_CONST*2)) / taho));
#ifdef TAHO_ROUND
        res = (res + TAHO_ROUND / 2) / TAHO_ROUND * TAHO_ROUND;        
#endif
        len = ultoa2(buf, res, 10);
    }

    return _print_half_width2(cursor_pos, len, POS_NONE, POS_OMIN, align);
}

/**
 * print fuel duration
 * @param align
 */
uint8_t print_fuel_duration(uint8_t cursor_pos, align_t align) {
    uint8_t len;
//    if (fuel_duration == 0) {
//        len = strcpy2(buf, (char *) &empty_string, 0);
//    } else
    {
        uint16_t res = (uint16_t) (fuel_duration * (1000 / 250) / (TAHO_TIMER_TICKS_PER_PERIOD / 250));
        len = print_fract(res, 2);
    }
    return _print_half_width2(cursor_pos, len, POS_NONE, POS_MS, align);
}

/**
 * print fuel consumption (l/h or l/100km)
 * @param fuel
 * @param kmh
 * @param drive_fl
 * @param align
 */
uint8_t print_fuel(uint8_t cursor_pos, uint16_t fuel, uint16_t kmh, uint8_t drive_fl, align_t align) {
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

    return _print_half_width2(cursor_pos, print_fract(t, 1), POS_NONE, pos, align);
}

/**
 * print main odometer
 * @param align
 */
uint8_t print_main_odo(uint8_t cursor_pos, align_t align) {
    uint8_t len = ultoa2(buf, (unsigned long) config.odo, 10);
    return _print_half_width2(cursor_pos, len, POS_NONE, POS_KM, align);
}

#if defined(TEMPERATURE_SUPPORT)
uint8_t print_temp(uint8_t cursor_pos, uint8_t index, align_t align) {
    uint8_t len;
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
    
    return _print_half_width2(cursor_pos, len, POS_NONE, pos_suffix, align);
}

#endif

uint8_t print_voltage(uint8_t cursor_pos, uint16_t *adc_voltage, uint8_t prefix_pos, align_t align) {
    uint8_t len = print_fract((uint16_t) (*adc_voltage << 5) / (uint8_t) (VOLTAGE_ADJUST_CONST_MAX - (config.vcc_const - VOLTAGE_ADJUST_CONST_MIN)), 1);
    return _print_half_width2(cursor_pos, len, prefix_pos, POS_VOLT, align);
}

void print_time(ds_time* time) {
    print_time_hm(LCD_CURSOR_POS_00, time->hour, time->minute, ALIGN_LEFT);

    print_time_dmy(LCD_CURSOR_POS_01, time->day, time->month, time->year, ALIGN_RIGHT);

    print_time_dow(LCD_CURSOR_POS_10, time->day_of_week, ALIGN_LEFT);
}

// default time format
// 0123456701234567
//  +  +    +  +  + (cursor blink position)
// HH:MM   DDmmm'YY
const time_editor_item_t time_editor_items_array[] = {
    {&time.hour, 0, 23, LCD_CURSOR_POS_00 + 0x01},
    {&time.minute, 0, 59, LCD_CURSOR_POS_00 + 0x04},
    {&time.day, 1, 31, LCD_CURSOR_POS_01 + 0x01},
    {&time.month, 1, 12, LCD_CURSOR_POS_01 + 0x04},
    {&time.year, VERSION_YEAR, VERSION_YEAR + 10, LCD_CURSOR_POS_01 + 0x07},
    {&time.day_of_week, 1, 7, LCD_CURSOR_POS_10},
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
        while (screen_refresh = 0, timeout_timer1 != 0) {
            //screen_refresh = 0;

#if defined(ENCODER_SUPPORT)
            if (config.settings.encoder != 0 && key2_press != 0) {
                edit_mode = ~edit_mode;
            }
            if (config.settings.encoder == 0 || edit_mode == 0)
#endif
            {
                handle_keys_next_prev(&c, 0, sizeof(time_editor_items_array) / sizeof(time_editor_items_array[0]) - 1);
                time_editor_item = (time_editor_item_t *) &time_editor_items_array[c];
            }
#if defined(ENCODER_SUPPORT)
            if ((config.settings.encoder != 0 && (key1_press != 0 || key3_press != 0) && edit_mode != 0) || (config.settings.encoder == 0 && key2_press != 0)) {           
#else
            if (key2_press != 0) {
#endif

                save_time = 1;
#if defined(ENCODER_SUPPORT)
                if (config.settings.encoder != 0 && key3_press != 0) {
                    *time_editor_item->p = bcd8_dec(*time_editor_item->p, time_editor_item->min, time_editor_item->max);
                } else if ((config.settings.encoder != 0 && key1_press != 0) || (config.settings.encoder == 0 && key2_press != 0))
#endif
                {
                    *time_editor_item->p = bcd8_inc(*time_editor_item->p, time_editor_item->min, time_editor_item->max);
                }

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


            LCD_cursor_off();
            print_time(&time);

#if defined(ENCODER_SUPPORT)
            if (config.settings.encoder != 0 && edit_mode == 0) {
                LCD_cursor_underline(time_editor_item->pos);
            } else {
                LCD_cursor_blink(time_editor_item->pos);
            }
#else
            LCD_cursor_blink(time_editor_item->pos);
#endif

            wait_refresh_timeout();
        }   
        screen_refresh = 1;

        // save time
        if (save_time != 0) {
            DS3231_time_write(&time);
        }

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
    timeout_timer1_loop(5) {
        handle_keys_up_down(&v, min_value, max_value);

        uint8_t len = ultoa2(buf, (unsigned long) (mode == CHAREDIT_MODE_10000KM ? (v * 1000L) : v), 10);

        _print_half_width2(LCD_CURSOR_POS_10 + LCD_WIDTH / 4, len, POS_NONE, (unsigned char) mode, ALIGN_RIGHT);

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

    unsigned char cursor_pos = LCD_CURSOR_POS_10 + (LCD_WIDTH - max_len) / 2U;
    unsigned char pos = 0;

    timeout_timer1_loop(5) {

#if defined(ENCODER_SUPPORT)
        if (config.settings.encoder != 0 && edit_mode != 0) {
            buf_prev = buf[pos];
            handle_keys_next_prev(&buf[pos], '0', '9');
        } else
#endif
        {
            handle_keys_next_prev(&pos, 0, max_len - 1);
        }

        // edit number in cursor position
        if (key2_press != 0) {
            key2_press = 0;
#if defined(ENCODER_SUPPORT)
            if (config.settings.encoder != 0) {
                edit_mode = ~edit_mode;
            } else
#endif            
            if (++buf[pos] > '9') {
                buf[pos] = '0';
            }

            timeout_timer1 = 5;
        }

        unsigned long _t = strtoul2(buf);
        if (_t > max_value) {
#if defined(ENCODER_SUPPORT)
            if (config.settings.encoder != 0) {
                buf[pos] = buf_prev;
            } else
#endif
            {
                buf[pos] = '0';
            }
        }

        LCD_cursor_off();
        _print_half_width(cursor_pos, max_len, ALIGN_LEFT);
        
#if defined(ENCODER_SUPPORT)
        if (config.settings.encoder != 0 && edit_mode == 0) {
            LCD_cursor_underline(cursor_pos + pos);
        } else {
            LCD_cursor_blink(cursor_pos + pos);
        }
#else
        LCD_cursor_blink(cursor_pos + pos);
#endif

        wait_refresh_timeout();
    }

    screen_refresh = 1;

    return strtoul2(buf);
}

uint16_t edit_value_bits(uint16_t v, char* str) {

    uint8_t pos = 0;

    timeout_timer1_loop(5) {

        handle_keys_next_prev(&pos, 0, 16 - 1);

        // edit number in cursor position
        if (key2_press != 0) {
            v ^= (1 << (15 - pos));
            timeout_timer1 = 5;
        }

        LCD_cursor_off();

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

        _print_full_width(LCD_CURSOR_POS_10, LCD_WIDTH, ALIGN_NONE);

        _memset(buf, ' ', 16);
        uint8_t len = strcpy2((char*) buf, (char *) str, pos + 1);
        if (len != 0) {
            // print on/off
            strcpy2((char*) &buf[12], (char *) &on_off_array, _onoff);
        }
        _print_full_width(LCD_CURSOR_POS_00, LCD_WIDTH, ALIGN_NONE);

        LCD_cursor_blink(LCD_CURSOR_POS_10 + pos);
        
        wait_refresh_timeout();
    }

    screen_refresh = 1;

    return v;
}

unsigned char request_screen(char* request_str) {
    unsigned char res = 0;
    if (key2_longpress != 0) {
        key2_longpress = 0;

        LCD_Clear();

        _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, request_str, 0), ALIGN_CENTER);
        
        wait_timeout(5);

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

void print_selected_param1(uint8_t cursor_pos, align_t align) {
    switch (select_param(&params.main, selected_param_total)) {
#if defined(TEMPERATURE_SUPPORT)
        case selected_param_temp:
            print_temp(cursor_pos, (config.settings.show_inner_temp != 0 ? TEMP_IN : TEMP_OUT) | PRINT_TEMP_PARAM_FRACT | PRINT_TEMP_PARAM_DEG_SIGN, align);
            break;
#endif            
        case selected_param_voltage:
            print_voltage(cursor_pos, (uint16_t *) &adc_voltage.current, POS_NONE, align);
            break;
        case selected_param_trip_time:
            print_trip_time(cursor_pos, &trips.tripC, align);
            break;
        case selected_param_trip_odometer:
            print_trip_odometer(cursor_pos, &trips.tripC, align);
            break;
        case selected_param_trip_average_fuel:
            print_trip_average_fuel(cursor_pos, &trips.tripC, align);
            break;
        case selected_param_trip_average_speed:
            print_trip_average_speed(cursor_pos, &trips.tripC, align);
            break;
        case selected_param_trip_total_fuel:
            print_trip_total_fuel(cursor_pos, &trips.tripC, align);
            break;
        case selected_param_max_speed:
            print_speed(cursor_pos, trips.tripC_max_speed, POS_MAX, 1, align);
            break;
        case selected_param_fuel_duration:
            print_fuel_duration(cursor_pos, align);
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
    // acceleration measurement flags and variables
    static __bit accel_meas_final_fl, _accel_meas_exit, _accel_meas_started_fl;

    _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) &accel_meas_wait_string, 0), ALIGN_CENTER);

    accel_meas_fl = 0; accel_meas_ok_fl = 0; accel_meas_timer = 0; accel_meas_final_fl = 0; _accel_meas_exit = 0, _accel_meas_started_fl = 0;

#ifdef EXTENDED_ACCELERATION_MEASUREMENT
    accel_meas_lower_const = (unsigned short) (accel_meas_limits[index].lower / config.odo_const);
    accel_meas_upper_const = (unsigned short) (accel_meas_limits[index].upper / config.odo_const);
#endif
    
    _memset(buf, '=', 16);

    clear_keys_state();
    // 16 sec waiting for start
    timeout_timer1 = 16;
    timeout_timer2 = 0;
    
    accel_meas_process_fl = 1;
    while (_accel_meas_exit == 0 && no_key_pressed()) {
        if (timeout_timer1 != 0 && accel_meas_drive_fl == 0) {
            if (timeout_timer2 == 0) {
                timeout_timer2 = INIT_TIMEOUT(0.25f);
                _print_full_width(LCD_CURSOR_POS_10, timeout_timer1, ALIGN_CENTER);
            }
        } else {
            if (timeout_timer1 != 0) {
                if (accel_meas_ok_fl == 0 && _accel_meas_started_fl == 0) {
                    // 30 sec for acceleration measurement
                    timeout_timer1 = 30;
                    _accel_meas_started_fl = 1;

                    LCD_Clear();
                }
                
                // speed
                _print_half_width2(LCD_CURSOR_POS_10, ultoa2((char*) buf, accel_meas_drive_fl != 0 ? (uint16_t) ((360000UL * SPEED_TIMER_TICKS_PER_PERIOD / config.odo_const) / accel_meas_speed) : 0, 10), POS_NONE, POS_KMH, ALIGN_LEFT);
                // time
                _print_half_width2(LCD_CURSOR_POS_11, print_fract(accel_meas_timer, 2), POS_NONE, POS_SEC, ALIGN_RIGHT);
            }

            if (timeout_timer1 != 0 && accel_meas_ok_fl == 0) {
                timeout_timer2 = INIT_TIMEOUT(0.11f); while (timeout_timer2 != 0);
            } else {
                if (accel_meas_ok_fl != 0 && accel_meas_final_fl == 0) {
                    // print final result
                    accel_meas_final_fl = 1;
                } else {
                    // timeout or 100 km/h
                    accel_meas_process_fl = 0;
                    if (timeout_timer1 == 0) {
                        // timeout
                        _print_full_width(LCD_CURSOR_POS_10, strcpy2(buf, (char *) &timeout_string, 0), ALIGN_CENTER);
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
                    wait_timeout(10);

                    _accel_meas_exit = 1;
                }
            }
        }
        
        LCD_flush_buffer();
    }
    accel_meas_process_fl = 0;
}

#ifdef EXTENDED_ACCELERATION_MEASUREMENT
void select_acceleration_measurement() {

    LCD_Clear();

    uint8_t v = 0, max_value = sizeof (accel_meas_limits) / sizeof (accel_meas_limits[0]) - 1, index = 0xFF;

    timeout_timer1_loop(5) {

        handle_keys_next_prev(&v, 0, max_value);

        if (key2_press != 0) {
            timeout_timer1 = 0;
            index = v;
        }

        uint8_t len = strcpy2(buf, (char *) &accel_meas_timing_string, 0);
        len += strcpy2(&buf[len], (char *) accel_meas_array, v + 1);

        _print_full_width(LCD_CURSOR_POS_00, len, ALIGN_CENTER);

        wait_refresh_timeout();
    }
    
    screen_refresh = 1;

    if (index != 0xFF) {
        acceleration_measurement(index);
    }
}
#endif

typedef enum {
    main_screen_page1_param_avg = 0,
#if defined(MIN_MAX_VOLTAGES_SUPPORT)
    main_screen_page1_param_voltages,
#endif
#if defined(TEMPERATURE_SUPPORT)
    main_screen_page1_param_temperature,
#endif
#if defined(CONTINUOUS_DATA_SUPPORT)
    main_screen_page1_param_cd,
#endif
    main_screen_page1_param_max
} main_screen_add_page_param;

#define ADDPAGE_TIMEOUT         20
#define ADDPAGE_TIMEOUT_IDLE    10
#define ADDPAGE_TIMEOUT_DRIVE   4

void screen_main(void) {
    
    static uint8_t main_screen_page;

    //  long press key1 (or doubleclick key2 when encoder control) to switch page (main_screen_page)

    //  main_screen_page == 0
    //  1) drive_min_speed_fl == 0 && motor_fl == 0 (engine off)
    //  time            main_odo
    //  temp/odo        voltage
    //
    //  2) drive_min_speed_fl == 0 && motor_fl != 0 (idling)
    //  time            taho
    //  param1          fuel_lh
    //
    //  3) drive_min_speed_fl != 0                  (driving)
    //  speed           taho
    //  param1          fuel_km
    
    //  main_screen_page != 0 (auto timeout when idling (ADDPAGE_TIMEOUT_IDLE) or driving (ADDPAGE_TIMEOUT_DRIVE))
    //  key2 press to switch params (main_add_param)
    //
    //  main_screen_page1_param_avg
    //  temp/odo        voltage
    //  avg_speed       avg_fuel 
    //
    //  main_screen_page1_param_cd
    //  temp/odo        voltage
    //  cd_speed        cd_fuel

#if defined(ENCODER_SUPPORT)
    if ((config.settings.encoder != 0 && key2_doubleclick != 0) || (config.settings.encoder == 0 && key1_longpress != 0)) {
#else
    if (key1_longpress != 0) {
#endif
        timeout_timer1 = ADDPAGE_TIMEOUT;
        main_screen_page = ~main_screen_page;
    }

    uint8_t _d_timeout = ADDPAGE_TIMEOUT - timeout_timer1;
    if (timeout_timer1 == 0
        || (drive_min_speed_fl != 0 && (_d_timeout >= ADDPAGE_TIMEOUT_DRIVE))
        || (motor_fl != 0 && (_d_timeout >= ADDPAGE_TIMEOUT_IDLE))) {
        main_screen_page = 0;
    }

#if defined(KEY3_SUPPORT)
    if (key1_press != 0 || key3_press != 0) {
        key3_press = 0;
#else
    if (key1_press != 0) {
#endif
        key1_press = 0;
        main_screen_page = 0;                
    }

    if (main_screen_page == 0) {
        current_item_main->page.skip_key_handler = 0;
        // main page
        if (drive_min_speed_fl == 0) {
            read_ds_time();
            print_time_hm(LCD_CURSOR_POS_00, time.hour, time.minute, ALIGN_LEFT);
        } else {
            print_speed(LCD_CURSOR_POS_00, speed, POS_NONE, 0, ALIGN_LEFT);
        }

        if (drive_min_speed_fl == 0 && motor_fl == 0) {
            print_main_odo(LCD_CURSOR_POS_01, ALIGN_RIGHT);
        } else {
            print_taho(LCD_CURSOR_POS_01, ALIGN_RIGHT);
        }

        if (drive_min_speed_fl == 0 && motor_fl == 0) {
#if defined(TEMPERATURE_SUPPORT)
            print_temp(LCD_CURSOR_POS_10, (config.settings.show_inner_temp ? TEMP_IN : TEMP_OUT) | PRINT_TEMP_PARAM_FRACT | PRINT_TEMP_PARAM_DEG_SIGN, ALIGN_LEFT);
#else
            print_trip_odometer(LCD_CURSOR_POS_10, &trips.tripC, ALIGN_LEFT);
#endif
            print_voltage(LCD_CURSOR_POS_11, (uint16_t *) & adc_voltage.current, POS_NONE, ALIGN_RIGHT);
        } else {
            print_selected_param1(LCD_CURSOR_POS_10, ALIGN_LEFT);
            print_fuel(LCD_CURSOR_POS_11, fuel, kmh, drive_min_speed_fl, ALIGN_RIGHT);
        }
#ifdef EXTENDED_ACCELERATION_MEASUREMENT
        if (drive_fl == 0 && motor_fl != 0 && key2_longpress != 0) {
            key2_longpress = 0;
            select_acceleration_measurement();
        }
#else
        if (drive_fl == 0 && motor_fl != 0 && request_screen((char *) &accel_meas_string) != 0) {
            acceleration_measurement(0);
        }
#endif        
    } else {
        current_item_main->page.skip_key_handler = 1;
        // additional page
        if (key2_press != 0) {
            timeout_timer1 = ADDPAGE_TIMEOUT;
        }
        switch (select_param(&params.main_add, main_screen_page1_param_max)) {
            case main_screen_page1_param_avg:
#if defined(TEMPERATURE_SUPPORT)
                print_temp(LCD_CURSOR_POS_00, (config.settings.show_inner_temp ? TEMP_IN : TEMP_OUT) | PRINT_TEMP_PARAM_FRACT | PRINT_TEMP_PARAM_DEG_SIGN, ALIGN_LEFT);
#else
                print_trip_odometer(LCD_CURSOR_POS_00, &trips.tripC, ALIGN_LEFT);
#endif
                print_voltage(LCD_CURSOR_POS_01, (uint16_t *) & adc_voltage.current, POS_NONE, ALIGN_RIGHT);

                print_trip_average_speed(LCD_CURSOR_POS_10, &trips.tripC, ALIGN_LEFT);
                print_trip_average_fuel(LCD_CURSOR_POS_11, &trips.tripC, ALIGN_RIGHT);
                break;

#if defined(TEMPERATURE_SUPPORT)
            case main_screen_page1_param_temperature:
                // show temp sensors config on ok key longpress
#if defined(DS18B20_CONFIG)
                if (key2_longpress != 0) {
                    key2_longpress = 0;

                    config_screen_temp_sensors();

                    timeout_timer1 = ADDPAGE_TIMEOUT;

                    clear_keys_state();
                    screen_refresh = 1;
                } else
#endif
                {
                    print_temp(LCD_CURSOR_POS_00, TEMP_OUT | PRINT_TEMP_PARAM_HEADER | PRINT_TEMP_PARAM_FRACT, ALIGN_RIGHT);
                    _print_half_width(LCD_CURSOR_POS_01, 0, ALIGN_RIGHT); //empty 

                    print_temp(LCD_CURSOR_POS_10, TEMP_IN | PRINT_TEMP_PARAM_HEADER | PRINT_TEMP_PARAM_FRACT, ALIGN_RIGHT);
                    print_temp(LCD_CURSOR_POS_11, TEMP_ENGINE | PRINT_TEMP_PARAM_HEADER | PRINT_TEMP_PARAM_NO_PLUS_SIGN, ALIGN_RIGHT);

                    // force faster temperature update
                    if (temperature_conv_fl == 0 && timeout_temperature > FORCED_TIMEOUT_TEMPERATURE) {
                        timeout_temperature = FORCED_TIMEOUT_TEMPERATURE;
                    }
                }
                break;
#endif
#if defined(MIN_MAX_VOLTAGES_SUPPORT)
            case main_screen_page1_param_voltages:
                if (key2_longpress != 0) {
                    if (request_screen((char *) &reset_string) != 0) {
                        adc_voltage.min = adc_voltage.max = adc_voltage.current;
                    }
                    timeout_timer1 = ADDPAGE_TIMEOUT;
                }
                _print_half_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) voltage_string, 0), ALIGN_LEFT);
                print_voltage(LCD_CURSOR_POS_01, (uint16_t*) &adc_voltage.current, POS_NONE, ALIGN_RIGHT);

                print_voltage(LCD_CURSOR_POS_10, (uint16_t*) &adc_voltage.min, POS_MIN, ALIGN_LEFT);
                print_voltage(LCD_CURSOR_POS_11, (uint16_t*) &adc_voltage.max, POS_MAX, ALIGN_RIGHT);
                break;
#endif
#if defined(CONTINUOUS_DATA_SUPPORT)
            case main_screen_page1_param_cd:
                if (key2_longpress != 0) {
                    if (request_screen((char *) &reset_string) != 0) {
                        cd.filter = 0;
                        cd_init();
                    }
                    timeout_timer1 = ADDPAGE_TIMEOUT;
                }

#ifdef FUEL_TANK_SUPPORT
                add_leading_symbols(buf, ' ', ultoa2(buf, adc_fuel_tank, 10), 16);
                strcpy2(buf, (char *) continuous_data_string, 0);
                _print_full_width(LCD_CURSOR_POS_00, LCD_WIDTH, ALIGN_NONE);
#else
                _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) continuous_data_string, 0), ALIGN_LEFT);
#endif
                
                if (continuous_data_fl != 0) {
                    print_speed(LCD_CURSOR_POS_10, cd_speed, POS_NONE, 0, ALIGN_LEFT);
                    print_fuel(LCD_CURSOR_POS_11, cd_fuel, cd_kmh, drive_min_cd_speed_fl, ALIGN_RIGHT);
                } else {
                    uint8_t len = strcpy2(buf, (char *) &empty_string, 0);
                    _print_half_width2(LCD_CURSOR_POS_10, len, POS_NONE, POS_KMH, ALIGN_LEFT);
                    _print_half_width2(LCD_CURSOR_POS_11, len, POS_NONE, POS_LKM, ALIGN_RIGHT);
                }
                break;
#endif
        }

    }
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
    
    uint8_t item_index = current_item_main->page.index;

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

    uint8_t len = strcpy2(buf, (char *) &trip_string, 0);
    len += strcpy2(&buf[len], (char *) trips_array, trips_pos);

    _print_half_width(LCD_CURSOR_POS_00, len, ALIGN_LEFT);

    print_trip_odometer(LCD_CURSOR_POS_01, trip, ALIGN_RIGHT);
    
    switch (select_param(&params.tmp, 2)) {
        case 0:
            print_trip_average_fuel(LCD_CURSOR_POS_10, trip, ALIGN_LEFT);
            print_trip_average_speed(LCD_CURSOR_POS_11, trip, ALIGN_RIGHT);
            break;
        case 1:
            print_trip_time(LCD_CURSOR_POS_10, trip, ALIGN_LEFT);
            print_trip_total_fuel(LCD_CURSOR_POS_11, trip, ALIGN_RIGHT);
            break;
    }

    clear_trip(false, trip);
}

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

    _print_full_width(LCD_CURSOR_POS_00, strcpy2((char*)buf, (char *) &service_counters_array, service_param + 1), ALIGN_LEFT);

    if (service_param == 0) {
        srv = &services.srv[0];
        v = get_mh();
    } else {
        srv = &services.srv[service_param - 1];
        v = srv->counter;
    }

    uint8_t len = ultoa2(buf, v, 10);

    len += print_symbols_str(len, service_param == 0 ? POS_HOUR : POS_KM);

    buf[len++] = ' ';
    _print_half_width(LCD_CURSOR_POS_10, len, ALIGN_RIGHT);
    
    s_time = srv->time;

    print_time_dmy(LCD_CURSOR_POS_11, s_time.day, s_time.month, s_time.year, ALIGN_RIGHT);
    
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
    uint8_t len;

    _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) &journal_viewer_string, 0), ALIGN_LEFT);

    _print_full_width(LCD_CURSOR_POS_10, 0, ALIGN_NONE);

    if (key2_press != 0) {
        key2_press = 0;

        uint8_t journal_type = 0;

        timeout_timer1_loop(5) {

            handle_keys_next_prev(&journal_type, 0, 3);

            _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) &journal_viewer_string, 0), ALIGN_LEFT);

            buf[0] = '1' + journal_type;
            buf[1] = '.';
            _print_full_width(LCD_CURSOR_POS_10, strcpy2(&buf[2], (char *) journal_viewer_items_array, journal_type + 1) + 2, ALIGN_LEFT);

            if (key2_press != 0) {
                key2_press = 0;

                journal_reader_t jr = {journal_header.journal_type_pos[journal_type].current, journal_header.journal_type_pos[journal_type].max, 0, 0xFF};

                journal_accel_item_t *accel_item;
                journal_trip_item_t *trip_item;

                uint8_t item_page = 0;

                timeout_timer1_loop(5) {

                    if (jr.item_current != 0xFF) {
                        handle_keys_next_prev(&jr.item_num, 0, jr.item_current);

                        if (jr.item_prev != jr.item_num) {
                            unsigned char *item = journal_read_item(&jr, journal_type);

                            jr.item_prev = jr.item_num;

                            if (*item != JOURNAL_ITEM_OK && jr.item_num == 0) {
                                jr.item_current = 0xFF;
                            } else {
                                item_page = 0;
                                if (journal_type == 3) {
                                    accel_item = (journal_accel_item_t *) item;
                                } else {
                                    trip_item = (journal_trip_item_t *) item;
                                }
                            }
                        }

                        if (jr.item_current != 0xFF) {

                            trip_time_t *trip_time;

                            // show journal item data
                            if (journal_type == 3) {
                                // accel_item
                                trip_time = &accel_item->start_time;

                                // upper-lower
                                len = ultoa2(buf, accel_item->lower, 10);
                                buf[len++] = '-';
                                len += ultoa2(&buf[len], accel_item->upper, 10);

                                _print_half_width(LCD_CURSOR_POS_10, len, ALIGN_LEFT);

                                // time
                                len = print_fract(accel_item->time, 2);
                                _print_half_width2(LCD_CURSOR_POS_11, len, POS_NONE, POS_SEC, ALIGN_RIGHT);

                            } else {
                                // trip_item
                                trip_time = &trip_item->start_time;

                                switch(item_page) {
                                    case 0:
                                        print_trip_odometer(LCD_CURSOR_POS_10, &trip_item->trip, ALIGN_LEFT);
                                        print_trip_average_fuel(LCD_CURSOR_POS_11, &trip_item->trip, ALIGN_RIGHT);
                                        break;
                                    case 1:
                                        print_trip_average_speed(LCD_CURSOR_POS_10, &trip_item->trip, ALIGN_LEFT);
                                        print_trip_time(LCD_CURSOR_POS_11, &trip_item->trip, ALIGN_RIGHT);
                                        break;
                                    case 2:
                                        print_trip_total_fuel(LCD_CURSOR_POS_10, &trip_item->trip, ALIGN_LEFT);
                                        _print_half_width(LCD_CURSOR_POS_11, 0, ALIGN_RIGHT);
                                        break;
                                }
                            }

                            len = ultoa2(buf, jr.item_num + 1, 10);
                            buf[len++] = '.';

                            len += journal_print_item_time((char *) &buf[len], trip_time);

                            _print_full_width(LCD_CURSOR_POS_00, len > LCD_WIDTH ? LCD_WIDTH : len, ALIGN_LEFT);

                            if (key2_press != 0) {
                                if (journal_type != 3 && ++item_page > 2) {
                                    item_page = 0;
                                }
                                timeout_timer1 = 5;
                                screen_refresh = 1;
                            }
                        }
                    }

                    if (jr.item_current == 0xFF) {
                        // no items for current journal
                        _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) journal_viewer_items_array, journal_type + 1), ALIGN_LEFT);
                        _print_full_width(LCD_CURSOR_POS_10, strcpy2(buf, (char *) &journal_viewer_no_items_string, 0), ALIGN_LEFT);

                        if (key2_press != 0) {
                            timeout_timer1 = 0;
                            screen_refresh = 1;
                        }
                    }
                    wait_refresh_timeout();
                }
                screen_refresh = 1;
                timeout_timer1 = 5;
            }
            wait_refresh_timeout();
        }
        screen_refresh = 1;
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
    uint8_t len;
    unsigned char tbuf[24];

    uint16_t _temps[3] = {DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE};
    uint8_t _t_num[3] = {0, 0, 0};
    uint8_t current_device = 0;
    
    // bad timings for pic?
    while (buzzer_fl != 0) {};
    
    uint8_t num_devices = onewire_search_devices((uint8_t *) tbuf, 3);
    
    ds18b20_start_conversion(); config_temperature_conv_fl = 0; timeout_timer2 = 100;

    timeout_timer1_loop(5) {
        
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
                len = print_temp(LCD_CURSOR_POS_NONE, TEMP_CONFIG, ALIGN_LEFT);
            } else {
                len = 0;
            }
            add_leading_symbols(buf, ' ', len, 16);

            // print t.sensor + number/total
            strcpy2(buf, (char *) &temp_sensor_string, 0);
            buf[9]= '1' + current_device;
            buf[10] = '/';
            buf[11] = '0' + num_devices;

            _print_full_width(LCD_CURSOR_POS_00, LCD_WIDTH, ALIGN_NONE);

            // convert id to hex string
            llptrtohex((unsigned char*) &tbuf[current_device * 8], (unsigned char*) buf);

            // print sensor string (in/out/eng)
            len = strcpy2(&buf[12], (char *) &temp_sensors_array, _t_num[current_device] + 1);
            add_leading_symbols(&buf[12], ' ', len, 4);

        } else {
            _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) config_menu_array, TEMP_SENSOR_INDEX), ALIGN_LEFT);
            _memset(buf, ' ', 16);
            strcpy2(buf, (char *) &temp_no_sensors, 0);
        }

        _print_full_width(LCD_CURSOR_POS_10, LCD_WIDTH, ALIGN_NONE);
        
        if (request_screen((char *) &reset_string) != 0) {
            _t_num[0] = 1; _t_num[1] = 2; _t_num[2] = 3;
            temps[0] = DS18B20_TEMP_NONE; temps[1] = DS18B20_TEMP_NONE; temps[2] = DS18B20_TEMP_NONE;
            _memset(&tbuf, 0xFF, 8 * 3);
            timeout_timer1 = 0;
        }

        wait_refresh_timeout();
    }
    
    screen_refresh = 1;

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
    uint8_t len;
    char tbuf[8];
    
    _memset(tbuf, 0xFF, 8);
    ds18b20_read_rom((unsigned char*) tbuf);

#if defined(DS18B20_CONFIG_SHOW_TEMP)
    ds18b20_start_conversion(); config_temperature_conv_fl = 0; timeout_timer2 = 100;
#endif

    unsigned char t_num = 0;

    timeout_timer1_loop(5) {

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
            add_leading_symbols((char *) &buf, ' ', print_temp(LCD_CURSOR_POS_NONE, TEMP_CONFIG, ALIGN_LEFT), 16);
        }
#endif
        _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) config_menu_array, TEMP_SENSOR_INDEX), ALIGN_LEFT);

        llptrtohex((unsigned char*) tbuf, (unsigned char*) buf);
        len = strcpy2(&buf[12], (char *) &temp_sensors_array, t_num + 1);
        add_leading_symbols(&buf[12], ' ', len, 4);
        
        _print_full_width(LCD_CURSOR_POS_10, LCD_WIDTH, ALIGN_NONE);

        wait_refresh_timeout();
    }
    
    screen_refresh = 1; 
    
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
    
    buf[0] = '1' + c_sub_item;
    buf[1] = '.';
    _print_full_width(LCD_CURSOR_POS_10, 2 + strcpy2(&buf[2], (char *) &service_counters_array, c_sub_item + 1), ALIGN_LEFT);

    if (key2_press != 0) {
        key2_press = 0;

        _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) &service_counters_array, c_sub_item + 1), ALIGN_LEFT);
        _print_full_width(LCD_CURSOR_POS_10, 0, ALIGN_NONE);

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

    print_voltage(LCD_CURSOR_POS_10 + LCD_WIDTH / 4, (uint16_t *) &adc_voltage.current, POS_NONE, ALIGN_RIGHT);
}

void config_screen_version() {
    if (key1_press != 0 || key2_press != 0) {
        timeout_timer1 = 0;
    }
    _print_full_width(LCD_CURSOR_POS_10, strcpy2(buf, (char*) &version_string, 0), ALIGN_LEFT);
}

void config_screen() {

    _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) &config_menu_title_string, 0), ALIGN_LEFT);

    buf[0] = '1' + current_item_config->page.index;
    buf[1] = '.';
    _print_full_width(LCD_CURSOR_POS_10, strcpy2(&buf[2], (char *) config_menu_array, current_item_config->page.title_string_index) + 2, ALIGN_LEFT);

    if (key2_press != 0) {
        key2_press = 0;
        LCD_Clear();

        timeout_timer1_loop(5) {
            _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) config_menu_array, current_item_config->page.title_string_index), ALIGN_LEFT);

            current_item_config->screen();

            LCD_cursor_off();

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
            _print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char*) &warning_string, 0), ALIGN_CENTER);

            _print_full_width(LCD_CURSOR_POS_10, strcpy2(buf, (char*) &service_counters_array, i + 1), ALIGN_CENTER);
            
            wait_timeout(3);

            clear_keys_state();
        }
        warn >>= 1;
    }
}
#endif

#if defined(PROGMEM_EEPROM) || defined(ENCODER_SUPPORT)

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
      SND_ON;
    } else {
      SND_OFF;
    }
    for (r = 0; r < tone; r++) {
      _delay_us(15);
    }
  }
  SND_OFF;
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
#endif

#if defined(PROGMEM_EEPROM) || defined (ENCODER_SUPPORT)

#define SETTINGS_DELAY 150

uint8_t stage_setting = 0;

typedef enum {
#if defined(ENCODER_SUPPORT)
    FORCE_SETTING_ENCODER_OFF=1,        // force encoder off
    FORCE_SETTING_ENCODER_ON,           // force encoder on
#endif
#if defined(PROGMEM_EEPROM)  
    FORCE_SETTING_EEPROM_REWRITE,       // eeprom rewrite for arduino target
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
    while (KEY_OK_PRESSED) {
        uint8_t keytime = SETTINGS_DELAY;
        while (KEY_OK_PRESSED) {
            _delay_ms(10);
            if (--keytime == 0) {
                keytime = SETTINGS_DELAY;
                if (stage_setting < (FORCE_SETTING_MAX - 1)) {
                    stage_setting++;
                    for (uint8_t i = 0; i < stage_setting; i++) {
                        beep(BEEP_OK);
                        delay_ms(150);
                    }
                } else {
                    while (KEY_OK_PRESSED) {};
                    stage_setting = 0;
                }
            }
        }
        if (!KEY_OK_PRESSED && stage_setting != 0) {
            beep(BEEP_UP);
            beep(BEEP_UP);
            beep(BEEP_UP);
        }
    }
#if defined(PROGMEM_EEPROM)
    // check eeprom special mark and save default eeprom content if mark not exists
    check_eeprom(stage_setting);
#endif
}

#endif

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

    params.main = config.selected_param.main_param;
    params.main_add = config.selected_param.main_add_param;
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

void handle_temp() {
    unsigned char buf[8];

    if (temperature_conv_fl != 0) {
        temperature_conv_fl = 0;
        // read temperature for ds18b20/ds3231
        timeout_temperature = TIMEOUT_TEMPERATURE;
#if defined(DS18B20_TEMP)
        unsigned char _temps_ee_addr = EEPROM_DS18B20_ADDRESS;
        for (unsigned char i = 0; i < 3; i++) {
            HW_read_eeprom_block((unsigned char *) &buf, _temps_ee_addr, 8);
            if (ds18b20_read_temp_matchrom((unsigned char *) &buf, &_t) == 0) {
                temps[i] = DS18B20_TEMP_NONE;
            } else {
                temps[i] = _t;
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
          DS3231_temp_start();
        }
#endif        
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

#if defined(PROGMEM_EEPROM) || defined(ENCODER_SUPPORT)
    preinit_settings();
#endif

    read_eeprom();

#if defined(ENCODER_SUPPORT)
    if (stage_setting == FORCE_SETTING_ENCODER_OFF) {
        config.settings.encoder = 0;
    } else if (stage_setting == FORCE_SETTING_ENCODER_ON) {
        config.settings.encoder = 1;
    }
#endif

    LCD_Init();

    set_consts();

    read_ds_time();

#if defined(JOURNAL_SUPPORT)
    if (journal_check_eeprom() == 0) {
        items_main[SCREEN_INDEX_JOURNAL].page.skip = 1;
    }
#endif

    if (time.flags.is_valid) {
        if (check_tripC_time() != 0) {
            // clear tripC
            clear_trip(true, &trips.tripC);
            trips.tripC_max_speed = 0;
        }

#if defined(JOURNAL_SUPPORT)
        if (check_tripB_month() != 0) {
            clear_trip(true, &trips.tripB);
        }
#endif
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


    config.selected_param.main_param = params.main;
    config.selected_param.main_add_param = params.main_add;
#ifdef SERVICE_COUNTERS_SUPPORT
    config.selected_param.service_param = service_param;
#endif

    save_eeprom();

    PWR_OFF;
    while (1);
}

void main() {
    static __bit config_mode;
    static __bit item_change_fl;
    static __bit mode_change_fl;

    uint8_t c_item = 0, c_item_prev = 0;
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
    
    //clear_keys_state();
    
    // wait first adc conversion
    while (adc_voltage.current == 0 && screen_refresh == 0)
        ;
    
    while (1) {
        if (screen_refresh != 0) {           
            screen_refresh = 0;
            handle_misc_values();
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

        if (config_mode == 0) {
            max_item = sizeof (items_main) / sizeof (items_main[0]) - 1;

            do {
                current_item_main = &items_main[c_item];

                if (current_item_main->page.skip != 0 || (drive_min_speed_fl != 0 && current_item_main->page.drive_mode == 0)) {
#ifdef KEY3_SUPPORT
                    if (c_item < c_item_prev) {
                        if (c_item == 0) {
                            c_item = max_item;
                        } else {
                            c_item--;
                        }
                    } else
#endif
                    {
                        if (c_item == max_item) {
                                c_item = 0;
                        } else {
                            c_item++;
                        }
                    }
                }
            } while (current_item_main->page.skip != 0 || (drive_min_speed_fl != 0 && current_item_main->page.drive_mode == 0));
        } else {
            max_item = sizeof (items_config) / sizeof (items_config[0]) - 1;
            current_item_config = &items_config[c_item];
            current_item_config->page.index = c_item;
        }

        if (mode_change_fl == 0) {
            if (item_change_fl == 0 && key2_longpress != 0) {
                // long keypress for service key - switch service mode and main mode
                if (config_mode == 0 && motor_fl == 0 && drive_fl == 0 && current_item_main->page.config_switch) {
                    prev_main_item = c_item;
                    c_item = prev_config_item;
                    config_mode = 1;
                    mode_change_fl = 1;
                } else if (config_mode != 0) {
                    prev_config_item = c_item;
                    c_item = prev_main_item;
                    // save config
                    save_eeprom_config();
                    // set consts
                    set_consts();
                    config_mode = 0;
                    mode_change_fl = 1;
                }
            }
        } else {
            mode_change_fl = 0;
        }
        
        if (mode_change_fl == 0) {
            if (item_change_fl == 0) {
                // show next/prev screen
                c_item_prev = c_item;

                // skip next/prev key for additional page of main screen
                if (config_mode != 0 || current_item_main->page.skip_key_handler == 0) {
                    handle_keys_next_prev(&c_item, 0, max_item);
                }

                if (c_item != c_item_prev) {
                    item_change_fl = 1;
                }
            } else {
                params.tmp = 0;
                item_change_fl = 0;
            }

            if (item_change_fl == 0) {
                if (config_mode != 0) {
                    config_screen();
                } else {
                    current_item_main->screen();
                    LCD_cursor_off();
                }
                LCD_flush_buffer();
            }
        }

        clear_keys_state();
        
        while (screen_refresh == 0 && mode_change_fl == 0 && item_change_fl == 0);
    }
}
