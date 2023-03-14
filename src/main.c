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

flag_t drive_min_speed_fl;
uint8_t fuel_instant_pos;
#ifdef CONTINUOUS_DATA_SUPPORT
uint8_t cd_fuel_instant_pos;
#endif

#ifdef TEMPERATURE_SUPPORT
flag_t temperature_conv_fl;
uint8_t main_temp_index;

uint16_t _t;
uint16_t temps[4] = {DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE};
#endif

typedef struct {
    uint8_t main;
    uint8_t main_add;
    uint8_t tmp;        // cleared on screen item change, used also as additional page index
} params_t;

params_t params = {0, 0, 0};

#ifdef SERVICE_COUNTERS_SUPPORT
uint8_t service_param = 0;
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
void lcd_print_full_width(uint8_t cursor_pos, uint8_t len, align_t align) {
    if (cursor_pos != LCD_CURSOR_POS_NONE) {
        LCD_cursor_set_position(cursor_pos);
        LCD_Write_String(buf, len, LCD_WIDTH, align);
    }
}

#if 1
/**
 * print aligned string from buffer (lcd half width bytes total length)
 * @param len
 * @param align
 */
void lcd_print_half_width(uint8_t cursor_pos, uint8_t len, align_t align) {
    if (cursor_pos != LCD_CURSOR_POS_NONE) {
        LCD_cursor_set_position(cursor_pos);
        LCD_Write_String(buf, len, LCD_WIDTH / 2, align);
    }
}
#else
#define lcd_print_half_width(cursor_pos, len, align) lcd_print_half_width2(cursor_pos, len, POS_NONE, POS_NONE, align)
#endif

/**
 * print aligned string with prefix/suffix from buffer (lcd half width bytes total length)
 * @param len
 * @param pos
 * @param align
 * @return 
 */
void lcd_print_half_width2(uint8_t cursor_pos, uint8_t len, uint8_t pos_prefix, uint8_t pos_suffix, align_t align) {
    // prefix
    uint8_t len_prefix = print_symbols_str(LCD_WIDTH, pos_prefix);

    add_leading_symbols((char *) &buf, ' ', len, len + len_prefix);

    len += len_prefix;

    while (len_prefix-- != 0) {
        buf[len_prefix] = buf[LCD_WIDTH + len_prefix];
    }

    // suffix
    len += print_symbols_str(len, pos_suffix);

    if (cursor_pos != LCD_CURSOR_POS_NONE) {
        LCD_cursor_set_position(cursor_pos);
        LCD_Write_String(buf, len, LCD_WIDTH / 2, align);
    }
}

/**
 * print fractional number [num/10^frac].[num%10^frac]
 * @param num
 * @param frac numbers after '.'
 * @return 
 */
uint8_t print_fract(uint24_t num, uint8_t frac) {
    uint8_t len = ultoa2_10(buf, num);
    if (frac == 0) return len;

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

#define VALUE_EMPTY     0x80

#if 0

uint8_t buf_print_value(uint24_t value, uint8_t frac) {
    uint8_t len;
    if (value == 0 && (frac & VALUE_EMPTY) != 0) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        len = print_fract(value, frac & ~VALUE_EMPTY);
    }
    return len;
}

#define POS_COMPOSE(prefix, suffix)  prefix, suffix
#define print_value(cursor_pos, value, frac, pos_compose, align) { lcd_print_half_width2(cursor_pos, buf_print_value(value, frac), pos_compose, align); }

#else

#define POS_COMPOSITE

#if defined(POS_COMPOSITE)
#define POS_COMPOSE(prefix, suffix)  (uint8_t) ((((uint8_t) prefix) << 4) | (uint8_t) suffix)
void print_value(uint8_t cursor_pos, uint24_t value, uint8_t frac, uint8_t pos_prefix_suffix, align_t align) {
#else
#define POS_COMPOSE(prefix, suffix)  prefix, suffix
void print_value(uint8_t cursor_pos, uint24_t value, uint8_t frac, uint8_t pos_prefix, uint8_t pos_suffix, align_t align) {
#endif
    // print value with empty instead of 0
    uint8_t len;
    if (value == 0 && (frac & VALUE_EMPTY) != 0) {
        len = strcpy2(buf, (char *) &empty_string, 0);
    } else {
        len = print_fract(value, frac & ~VALUE_EMPTY);
    }

#if defined(POS_COMPOSITE)
    lcd_print_half_width2(cursor_pos, len, pos_prefix_suffix >> 4, pos_prefix_suffix & 0x0F, align);
#else
    lcd_print_half_width2(cursor_pos, len, pos_prefix, pos_suffix, align);
#endif
}
#endif

uint8_t print_index_number(uint8_t index) {
    uint8_t len = ultoa2_10(buf, index);
    buf[len++] = '.';
    return len;
}

// print trip time
void print_trip_time(uint8_t cursor_pos, print_trip_t* pt, align_t align) {

    uint8_t len = ultoa2_10(buf, (uint16_t) (pt->time / 60));
    buf[len++] = ':';

    bcd8_to_str(&buf[len], bin8_to_bcd(pt->time % 60));
    len += 2;
   
    lcd_print_half_width(cursor_pos, len, align);
}

// print trip average speed
#define print_trip_average_speed(cursor_pos, pt, align) print_value(cursor_pos, (*(pt)).average_speed, VALUE_EMPTY | 1, POS_COMPOSE(POS_NONE, POS_KMH), align);

// print trip average fuel
#define print_trip_average_fuel(cursor_pos, pt, align) print_value(cursor_pos, (*(pt)).average_fuel, VALUE_EMPTY | 1, POS_COMPOSE(POS_NONE, POS_LKM), align);

// print trip odometer
#define print_trip_odometer(cursor_pos, pt, align) print_value(cursor_pos, (*(pt)).odo, 1, POS_COMPOSE(POS_NONE, POS_KM), align);

// print trip total fuel
#define print_trip_total_fuel(cursor_pos, pt, align) print_value(cursor_pos, (*(pt)).fuel, 1, POS_COMPOSE(POS_NONE, POS_LITR), align);

// print speed
#define print_speed(cursor_pos, speed, frac, pos_prefix, align) print_value(cursor_pos, speed, frac, POS_COMPOSE(pos_prefix, POS_KMH), align);

// print taho
#define print_taho(cursor_pos, align) print_value(cursor_pos, data.taho_rpm, VALUE_EMPTY | 0, POS_COMPOSE(POS_NONE, POS_OMIN), align);

// print fuel duration
#define print_fuel_duration(cursor_pos, align) print_value(cursor_pos, data.fuel_duration_ms, 2, POS_COMPOSE(POS_NONE, POS_MS), align);

// print fuel consumption (l/h or l/100km)
#define print_fuel(cursor_pos, fuel, fuel_suffix_pos, align) print_value(cursor_pos, fuel, 1, POS_COMPOSE(POS_NONE, fuel_suffix_pos), align);

// print main odometer
#define print_main_odo(cursor_pos, align) print_value(cursor_pos, (uint24_t) config.odo, 0, POS_COMPOSE(POS_NONE, POS_KM), align);

// print voltage
void print_voltage(uint8_t cursor_pos, uint16_t *adc_voltage, uint8_t prefix_pos, align_t align) {
    uint8_t len = print_fract(get_voltage_value(adc_voltage), 1);
    lcd_print_half_width2(cursor_pos, len, prefix_pos, POS_VOLT, align);
}

#if defined(TEMPERATURE_SUPPORT)
void print_temp(uint8_t cursor_pos, uint8_t index, align_t align) {
    uint8_t len;
    uint8_t param = index & ~PRINT_TEMP_PARAM_MASK;
    index &= PRINT_TEMP_PARAM_MASK;

    _t = temps[index];

    if (_t == DS18B20_TEMP_NONE) {
        len = strcpy2(buf, (char *) &temp_sensors_array, 1);
    } else {
#if defined(TEMPERATURE_FRACTIONAL)
        char sign = '\0';
        if (_t & 0x8000) // if temperature is negative
        {
            _t = (~_t) + 1; // change temperature value to positive form
            sign = '-'; // put minus sign (-)
        } else {
            if ((param & PRINT_TEMP_PARAM_NO_PLUS_SIGN) == 0) {
                sign = '+'; // put plus sign (+)
            }
        }
        _t = (unsigned short) ((_t >> 4) * 10 + (((_t & 0x000F) * 10) >> 4));

        uint8_t frac = 0;
        if ((param & PRINT_TEMP_PARAM_FRACT) != 0) {
            frac = 1;
        } else {
            _t /= 10;
        }
        len = print_fract(_t, frac);
        if (sign != '\0') {
            add_leading_symbols(buf, sign, len, len + 1);
            len++;
        }
#else
        uint8_t _pos = 0;
        if (_t & 0x8000) // if temperature is negative
        {
            _t = (~_t) + 1; // change temperature value to positive form
            buf[_pos++] = '-'; // put minus sign (-)
        } else {
            if ((param & PRINT_TEMP_PARAM_NO_PLUS_SIGN) == 0) {
                buf[_pos++] = '+'; // put plus sign (+)
            }
        }
        _t = (uint16_t) ((_t >> 4) * 10 + (((_t & 0x000F) * 10) >> 4));

        len = ultoa2_10(&buf[_pos], _t / 10) + _pos;
#endif
    }

#if defined(DS18B20_CONFIG_EXT) || defined(DS18B20_CONFIG_SHOW_TEMP)
    if (index == TEMP_CONFIG) {
        add_leading_symbols(buf, ' ', len, 16);
    } else
#endif
    {
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
        lcd_print_half_width2(cursor_pos, len, POS_NONE, pos_suffix, align);
    }
    
}

#endif

void print_time_hm(uint8_t cursor_pos, uint8_t hour, uint8_t minute, align_t align) {
    bcd8_to_str(buf, hour);
    buf[2] = ':';
    bcd8_to_str(&buf[3], minute);
    lcd_print_half_width(cursor_pos, 5, align);
}

void print_time_dmy(uint8_t cursor_pos, uint8_t day, uint8_t month, uint8_t year, align_t align) {
    if (day == 0x00 || day == 0xFF) {
        strcpy2(buf, (char*) &no_time_string, 0);
    } else {
        bcd8_to_str(buf, day);
        strcpy2(&buf[2], (char *) &month_array, bcd8_to_bin(month));
        buf[5] = '\'';
        bcd8_to_str(&buf[6], year);
    }
    lcd_print_half_width(cursor_pos, 8, align);
}

void print_time_dow(uint8_t cursor_pos, uint8_t day_of_week, align_t align) {
    uint8_t len = strcpy2((char *) buf, (char*) day_of_week_array, day_of_week);
    lcd_print_full_width(cursor_pos, len, align);
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
        
        timeout_timer1 = DEFAULT_TIMEOUT;
        while (screen_refresh = 0, timeout_timer1 != 0) {
            //screen_refresh = 0;

#if defined(ENCODER_SUPPORT)
            if (config.settings.encoder != 0 && key2_press != 0) {
                edit_mode = ~edit_mode;
            }
            if (config.settings.encoder == 0 || edit_mode == 0)
#endif
            {
                handle_keys_next_prev(&c, 0, sizeof(time_editor_items_array) / sizeof(time_editor_items_array[0]) - 1, DEFAULT_TIMEOUT);
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
                timeout_timer1 = DEFAULT_TIMEOUT;
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


uint8_t edit_value_char(uint8_t v, edit_value_char_t mode, uint8_t min_value, uint8_t max_value) {
    timeout_timer1_loop(DEFAULT_TIMEOUT) {
        handle_keys_up_down(&v, min_value, max_value, DEFAULT_TIMEOUT);

        uint8_t len = ultoa2_10(buf, (uint24_t) (mode == CHAREDIT_MODE_10000KM ? (v * 1000L) : v));

        lcd_print_half_width2(LCD_CURSOR_POS_10 + LCD_WIDTH / 4, len, POS_NONE, (uint8_t) mode, ALIGN_RIGHT);

        wait_refresh_timeout();
    }
    screen_refresh = 1;

    return v;
}

//#define EDIT_VALUE_LONG_FRACTION

#if defined(EDIT_VALUE_LONG_FRACTION)
#define LONG_VALUE_EDITOR_DEFAULT_FRACTION , 0
#else
#define LONG_VALUE_EDITOR_DEFAULT_FRACTION
#endif

#if defined(EDIT_VALUE_LONG_FRACTION)
uint24_t edit_value_long(uint24_t v, uint24_t max_value, uint8_t frac) {
#else
uint24_t edit_value_long(uint24_t v, uint24_t max_value) {
#endif
    // number of symbols to edit
#ifdef KEY3_SUPPORT        
    uint8_t _max_symbol_pos0 = buf[0];
#endif    
#if defined(ENCODER_SUPPORT)
    uint8_t edit_mode = 0, buf_prev;
#endif

#if defined(EDIT_VALUE_LONG_FRACTION)
    // edit with fractional part
    uint8_t max_len = print_fract((uint24_t) max_value, frac);
    if (v < max_value) {
        add_leading_symbols(buf, '0', print_fract((uint24_t) v, frac), max_len);
    }
    buf[max_len] = 0;
#else
    uint8_t max_len = ultoa2_10(buf, max_value);
    if (v < max_value) {
        // convert value
        add_leading_symbols(buf, '0', ultoa2_10(buf, v), max_len);
    }
#endif
    
    uint8_t cursor_pos = LCD_CURSOR_POS_10 + (LCD_WIDTH - max_len) / 2U;
    uint8_t pos = 0;

    timeout_timer1_loop(DEFAULT_TIMEOUT) {

#if defined(ENCODER_SUPPORT)
        if (config.settings.encoder != 0 && edit_mode != 0) {
            buf_prev = buf[pos];
            handle_keys_next_prev(&buf[pos], '0', '9', DEFAULT_TIMEOUT);
        } else
#endif
        {
            handle_keys_next_prev(&pos, 0, max_len - 1, DEFAULT_TIMEOUT);
        }

        // edit number in cursor position
        if (key2_press != 0) {
            key2_press = 0;
#if defined(ENCODER_SUPPORT)
            if (config.settings.encoder != 0) {
                edit_mode = ~edit_mode;
            } else
#endif

#if defined(EDIT_VALUE_LONG_FRACTION)
            if (buf[pos] != '.')
#endif
            {
                if (++buf[pos] > '9') {
                    buf[pos] = '0';
                }
            }

            timeout_timer1 = DEFAULT_TIMEOUT;
        }

        uint24_t _t = strtoul2(buf);
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
        lcd_print_half_width(cursor_pos, max_len, ALIGN_LEFT);
        
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

    timeout_timer1_loop(DEFAULT_TIMEOUT) {

        handle_keys_next_prev(&pos, 0, 16 - 1, DEFAULT_TIMEOUT);

        uint16_t mask = (1 << (15 - pos));
        // edit number in cursor position
        if (key2_press != 0) {
            v ^= mask;
            timeout_timer1 = DEFAULT_TIMEOUT;
        }

        LCD_cursor_off();

        uint8_t _onoff = ((v & mask) != 0) + 1;

        char *_buf = (char *) &buf;
        uint16_t _mask = 0x8000;
        for (uint8_t _i = 16; _i != 0; --_i) {
            if ((v & _mask) != 0) {
                *_buf++ = '*';
            } else {
                *_buf++ = '-';
            }
            _mask >>= 1;
        }

        lcd_print_full_width(LCD_CURSOR_POS_10, LCD_WIDTH, ALIGN_NONE);

        _memset(buf, ' ', 16);
        uint8_t len = strcpy2((char*) buf, (char *) str, pos + 1);
        if (len != 0) {
            // print on/off
            strcpy2((char*) &buf[12], (char *) &on_off_array, _onoff);
        }
        lcd_print_full_width(LCD_CURSOR_POS_00, LCD_WIDTH, ALIGN_NONE);

        LCD_cursor_blink(LCD_CURSOR_POS_10 + pos);
        
        wait_refresh_timeout();
    }

    screen_refresh = 1;

    return v;
}

uint8_t request_screen(char* request_str) {
    uint8_t res = 0;
    if (key2_longpress != 0) {
        key2_longpress = 0;

        LCD_Clear();

        lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, request_str, 0), ALIGN_CENTER);
        
        wait_timeout(5);

        if (key2_press != 0) {
            res = 1;
        }

        clear_keys_state();
        screen_refresh = 1;
    }
    return res;
}

uint8_t select_param(uint8_t* param, uint8_t total) {
    if (key2_press != 0) {
        key2_press = 0;
        *param = *param + 1;
    }
    if (*param >= total) {
        *param = 0;
    }
    return *param;
}

// extended params for main screen
//#define EXT_PARAMS

typedef enum {
#if defined(TEMPERATURE_SUPPORT)
    selected_param_temp,
#endif
    selected_param_voltage,
    selected_param_tripC_time,
    selected_param_tripC_odometer,
#if defined(EXT_PARAMS)
    selected_param_tripC_average_fuel,
    selected_param_tripC_average_speed,
    selected_param_tripC_total_fuel,
    selected_param_tripC_max_speed,
#endif
    selected_param_fuel_duration,
    selected_param_total
} selected_param_t;

void print_selected_param1(uint8_t cursor_pos, align_t align) {
    switch (select_param(&params.main, selected_param_total)) {
#if defined(TEMPERATURE_SUPPORT)
        case selected_param_temp:
            print_temp(cursor_pos, main_temp_index, align);
            break;
#endif            
        case selected_param_voltage:
            print_voltage(cursor_pos, (uint16_t *) &adc_voltage.current, POS_NONE, align);
            break;
        case selected_param_tripC_time:
            print_trip_time(cursor_pos, &ptrip, align);
            break;
        case selected_param_tripC_odometer:
            print_trip_odometer(cursor_pos, &ptrip, align);
            break;
#if defined(EXT_PARAMS)
        case selected_param_tripC_average_fuel:
            print_trip_average_fuel(cursor_pos, &ptrip, align);
            break;
        case selected_param_tripC_average_speed:
            print_trip_average_speed(cursor_pos, &ptrip, align);
            break;
        case selected_param_tripC_total_fuel:
            print_trip_total_fuel(cursor_pos, &ptrip, align);
            break;
        case selected_param_tripC_max_speed:
            print_speed(cursor_pos, trips.tripC_max_speed, 1, POS_MAX, align);
            break;
#endif
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
    static flag_t accel_meas_final_fl, _accel_meas_exit, _accel_meas_started_fl;

    lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) &accel_meas_wait_string, 0), ALIGN_CENTER);

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
                lcd_print_full_width(LCD_CURSOR_POS_10, timeout_timer1, ALIGN_CENTER);
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
                lcd_print_half_width2(LCD_CURSOR_POS_10, ultoa2_10(buf, accel_meas_drive_fl != 0 ? (uint16_t) ((360000UL * HW_SPEED_TIMER_TICKS_PER_PERIOD / config.odo_const) / accel_meas_speed) : 0), POS_NONE, POS_KMH, ALIGN_LEFT);
                // time
                lcd_print_half_width2(LCD_CURSOR_POS_11, print_fract(accel_meas_timer, 2), POS_NONE, POS_SEC, ALIGN_RIGHT);
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
                        lcd_print_full_width(LCD_CURSOR_POS_10, strcpy2(buf, (char *) &timeout_string, 0), ALIGN_CENTER);
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

    timeout_timer1_loop(DEFAULT_TIMEOUT) {

        handle_keys_next_prev(&v, 0, max_value, DEFAULT_TIMEOUT);

        if (key2_press != 0) {
            timeout_timer1 = 0;
            index = v;
        }

        uint8_t len = strcpy2(buf, (char *) &accel_meas_timing_string, 0);
        len += strcpy2(&buf[len], (char *) accel_meas_array, v + 1);

        lcd_print_full_width(LCD_CURSOR_POS_00, len, ALIGN_CENTER);

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

// timeouts for additional page (default >= idle (optional) >= drive (optional))
#define ADDPAGE_TIMEOUT         60
//#define ADDPAGE_TIMEOUT_IDLE    30
#define ADDPAGE_TIMEOUT_DRIVE   10

#define main_screen_page params.tmp

void screen_main(void) {
    
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
    //  key1/key3 press to switch params (main_add_param)
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

    if (timeout_timer1 == 0
#if defined(ADDPAGE_TIMEOUT_IDLE) && (ADDPAGE_TIMEOUT > ADDPAGE_TIMEOUT_IDLE)
            || (motor_fl != 0 && (timeout_timer1 <= (ADDPAGE_TIMEOUT - ADDPAGE_TIMEOUT_IDLE)))
#endif
#if defined(ADDPAGE_TIMEOUT_DRIVE) && (ADDPAGE_TIMEOUT > ADDPAGE_TIMEOUT_DRIVE)
            || (drive_min_speed_fl != 0 && (timeout_timer1 <= (ADDPAGE_TIMEOUT - ADDPAGE_TIMEOUT_DRIVE)))
#endif
        ) {
        main_screen_page = 0;
    }

    fill_print_trip(&ptrip, &trips.tripC);

    if (main_screen_page == 0) {
        current_item_main->page.skip_key_handler = 0;
        // main page
        if (drive_min_speed_fl == 0) {
            read_ds_time();
            print_time_hm(LCD_CURSOR_POS_00, time.hour, time.minute, ALIGN_LEFT);
        } else {
            print_speed(LCD_CURSOR_POS_00, round_div(data.speed, 10), 0, POS_NONE, ALIGN_LEFT);
        }

        if (drive_min_speed_fl == 0 && motor_fl == 0) {
            print_main_odo(LCD_CURSOR_POS_01, ALIGN_RIGHT);
        } else {
            print_taho(LCD_CURSOR_POS_01, ALIGN_RIGHT);
        }

        if (drive_min_speed_fl == 0 && motor_fl == 0) {
#if defined(TEMPERATURE_SUPPORT)
            print_temp(LCD_CURSOR_POS_10, main_temp_index, ALIGN_LEFT);
#else
            print_trip_odometer(LCD_CURSOR_POS_10, &ptrip, ALIGN_LEFT);
#endif
            print_voltage(LCD_CURSOR_POS_11, (uint16_t *) &adc_voltage.current, POS_NONE, ALIGN_RIGHT);
        } else {
            print_selected_param1(LCD_CURSOR_POS_10, ALIGN_LEFT);
            print_fuel(LCD_CURSOR_POS_11, data.fuel_instant, fuel_instant_pos, ALIGN_RIGHT);
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
        // additional page
        current_item_main->page.skip_key_handler = 1;

#if defined(TEMPERATURE_SUPPORT)
        // force faster temperature update
        if (temperature_conv_fl == 0 && timeout_temperature <= (TIMEOUT_TEMPERATURE - FORCED_TIMEOUT_TEMPERATURE + 1)) {
            timeout_temperature = 0;
        }
#endif

        handle_keys_next_prev(&params.main_add, 0, main_screen_page1_param_max - 1, ADDPAGE_TIMEOUT);

        if (key2_press != 0) {
            timeout_timer1 = ADDPAGE_TIMEOUT;
        }

        switch (params.main_add) {
            case main_screen_page1_param_avg:
#if defined(TEMPERATURE_SUPPORT)
                print_temp(LCD_CURSOR_POS_00, main_temp_index, ALIGN_LEFT);
#else
                print_trip_odometer(LCD_CURSOR_POS_00, &ptrip, ALIGN_LEFT);
#endif
                print_voltage(LCD_CURSOR_POS_01, (uint16_t *) &adc_voltage.current, POS_NONE, ALIGN_RIGHT);
                print_trip_average_speed(LCD_CURSOR_POS_10, &ptrip, ALIGN_LEFT);
                print_trip_average_fuel(LCD_CURSOR_POS_11, &ptrip, ALIGN_RIGHT);
                break;

#if defined(TEMPERATURE_SUPPORT)
            case main_screen_page1_param_temperature:
                // show temp sensors config on ok key longpress
#if defined(DS18B20_CONFIG)
                if (key2_longpress != 0) {
                    key2_longpress = 0;

                    if (motor_fl == 0 && drive_fl == 0) {
                        config_screen_temp_sensors();
                    }

                    timeout_timer1 = ADDPAGE_TIMEOUT;

                    screen_refresh = 1;
                } else
#endif
                {
                    print_temp(LCD_CURSOR_POS_00, TEMP_OUT | PRINT_TEMP_PARAM_HEADER | PRINT_TEMP_PARAM_FRACT, ALIGN_RIGHT);
                    lcd_print_half_width(LCD_CURSOR_POS_01, 0, ALIGN_RIGHT); //empty 

                    print_temp(LCD_CURSOR_POS_10, TEMP_IN | PRINT_TEMP_PARAM_HEADER | PRINT_TEMP_PARAM_FRACT, ALIGN_RIGHT);
                    print_temp(LCD_CURSOR_POS_11, TEMP_ENGINE | PRINT_TEMP_PARAM_HEADER | PRINT_TEMP_PARAM_NO_PLUS_SIGN, ALIGN_RIGHT);
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
                lcd_print_half_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) voltage_string, 0), ALIGN_LEFT);
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
                add_leading_symbols(buf, ' ', ultoa2_10(buf, adc_fuel_tank), 16);
                strcpy2(buf, (char *) continuous_data_string, 0);
                lcd_print_full_width(LCD_CURSOR_POS_00, LCD_WIDTH, ALIGN_NONE);
#else
                lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) continuous_data_string, 0), ALIGN_LEFT);
#endif
                
                if (continuous_data_fl != 0) {
                    print_speed(LCD_CURSOR_POS_10, round_div(data.cd_speed, 10), 0, POS_NONE, ALIGN_LEFT);
                    print_fuel(LCD_CURSOR_POS_11, data.cd_fuel_instant, cd_fuel_instant_pos, ALIGN_RIGHT);
                } else {
                    uint8_t len = strcpy2(buf, (char *) &empty_string, 0);
                    lcd_print_half_width2(LCD_CURSOR_POS_10, len, POS_NONE, POS_KMH, ALIGN_LEFT);
                    lcd_print_half_width2(LCD_CURSOR_POS_11, len, POS_NONE, POS_LKM, ALIGN_RIGHT);
                }
                break;
#endif
        }
        clear_keys_state();
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
    uint8_t trips_pos, max_trip_params;
    
    uint8_t item_index = current_item_main->page.index;

    if (item_index == SCREEN_INDEX_TRIP_C) {
        trip = &trips.tripC;
#ifdef SIMPLE_TRIPC_TIME_CHECK
        trips_pos = TRIPS_POS_DAY;
#else
        trips_pos = config.settings.daily_tripc ? TRIPS_POS_DAY : TRIPS_POS_CURR;
#endif
        max_trip_params = 3;
    } else if (item_index == SCREEN_INDEX_TRIP_A) {
        trip = &trips.tripA;
        trips_pos = TRIPS_POS_A;
        max_trip_params = 2;
    } else /*if (item_index == SCREEN_INDEX_TRIP_B) */{
        trip = &trips.tripB;
        trips_pos = TRIPS_POS_B;
        max_trip_params = 2;
    };

    uint8_t len = strcpy2(buf, (char *) &trip_string, 0);
    len += strcpy2(&buf[len], (char *) trips_array, trips_pos);
    lcd_print_half_width(LCD_CURSOR_POS_00, len, ALIGN_LEFT);

    fill_print_trip(&ptrip, trip);

    print_trip_odometer(LCD_CURSOR_POS_01, &ptrip, ALIGN_RIGHT);
    
    switch (select_param(&params.tmp, max_trip_params)) {
        case 0:
            print_trip_average_fuel(LCD_CURSOR_POS_10, &ptrip, ALIGN_LEFT);
            print_trip_average_speed(LCD_CURSOR_POS_11, &ptrip, ALIGN_RIGHT);
            break;
        case 1:
            print_trip_time(LCD_CURSOR_POS_10, &ptrip, ALIGN_LEFT);
            print_trip_total_fuel(LCD_CURSOR_POS_11, &ptrip, ALIGN_RIGHT);
            break;
        case 2:
            print_speed(LCD_CURSOR_POS_10, trips.tripC_max_speed, 1, POS_MAX, ALIGN_LEFT);
            lcd_print_half_width(LCD_CURSOR_POS_11, 0, ALIGN_RIGHT); // empty
            break;
    }

    clear_trip(false, trip);
}

#ifdef SERVICE_COUNTERS_SUPPORT
/**
 * get motor hours
 * @return 
 */
uint16_t get_mh() {
    // time based motor hours
    return (uint16_t) (services.mh.time / 3600UL);
}

void screen_service_counters() {
    
    srv_t* srv;
    unsigned short v;
    
    select_param(&service_param, 5);

    lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2((char*)buf, (char *) &service_counters_array, service_param + 1), ALIGN_LEFT);

    if (service_param == 0) {
        srv = &services.srv[0];
        v = get_mh();
    } else {
        srv = &services.srv[service_param - 1];
        v = srv->counter;
    }

    uint8_t len = ultoa2_10(buf, v);

    len += print_symbols_str(len, service_param == 0 ? POS_HOUR : POS_KM);

    buf[len++] = ' ';
    lcd_print_half_width(LCD_CURSOR_POS_10, len, ALIGN_RIGHT);
    
    print_time_dmy(LCD_CURSOR_POS_11, srv->day, srv->month, srv->year, ALIGN_RIGHT);
    
    if (request_screen((char *) &reset_string) != 0) {
        read_ds_time();
        if (service_param == 0 || service_param == 1) {
            services.mh.time = 0;
        }
        srv->counter = 0;
        srv->day = time.day;
        srv->month = time.month;
        srv->year = time.year;
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

#define JOURNAL_IDLE_TIMEOUT 30

typedef enum {
    screen_journal_viewer_page_title=0,
    screen_journal_viewer_page_type,
    screen_journal_viewer_page_item
} screen_journal_viewer_page_t;

#define screen_journal_viewer_page params.tmp

void screen_journal_viewer() {
    static uint8_t journal_type = 0;
    static uint8_t item_page = 0;
    static journal_reader_t jr;

    static journal_accel_item_t *accel_item;
    static journal_trip_item_t *trip_item;

    uint8_t len;

    if (key2_press != 0) {
        if (screen_journal_viewer_page < screen_journal_viewer_page_item) {
            screen_journal_viewer_page++;
            key2_press = 0;
        }
        timeout_timer1 = JOURNAL_IDLE_TIMEOUT;
    }

    if (timeout_timer1 == 0 || key2_longpress != 0) {
        key2_longpress = 0;
        if (screen_journal_viewer_page != 0) {
            screen_journal_viewer_page--;
        }
        if (screen_journal_viewer_page == 0) {
            timeout_timer1 = 0;
        } else {
            timeout_timer1 = JOURNAL_IDLE_TIMEOUT;
        }
    }

    if (screen_journal_viewer_page == 0) {
        current_item_main->page.skip_key_handler = 0;
    } else {
        current_item_main->page.skip_key_handler = 1;
    }

    switch(screen_journal_viewer_page) {
        case screen_journal_viewer_page_title:
            lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) &journal_viewer_string, 0), ALIGN_LEFT);
            lcd_print_full_width(LCD_CURSOR_POS_10, 0, ALIGN_NONE);
            break;
        case screen_journal_viewer_page_type:
            handle_keys_next_prev(&journal_type, 0, 3, JOURNAL_IDLE_TIMEOUT);
            
            lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) &journal_viewer_string, 0), ALIGN_LEFT);

            len = print_index_number(journal_type + 1);
            len += strcpy2(&buf[len], (char *) journal_viewer_items_array, journal_type + 1);

            lcd_print_full_width(LCD_CURSOR_POS_10, len, ALIGN_LEFT);

            jr.item_current = journal_header.journal_type_pos[journal_type].current;
            jr.item_max = journal_header.journal_type_pos[journal_type].max;
            jr.item_num = 0;
            jr.item_prev = 0xFF;

            item_page = 0;

            break;
        case screen_journal_viewer_page_item:
            if (jr.item_current != 0xFF) {
                handle_keys_next_prev(&jr.item_num, 0, jr.item_max - 1, JOURNAL_IDLE_TIMEOUT);

                if (jr.item_prev != jr.item_num) {
                    unsigned char *item = journal_read_item(&jr, journal_type);

                    jr.item_prev = jr.item_num;

                    if ((*item != JOURNAL_ITEM_V1 && *item != JOURNAL_ITEM_V2) && jr.item_num == 0) {
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
                        len = ultoa2_10(buf, accel_item->lower);
                        buf[len++] = '-';
                        len += ultoa2_10(&buf[len], accel_item->upper);

                        lcd_print_half_width(LCD_CURSOR_POS_10, len, ALIGN_LEFT);

                        // time
                        len = print_fract(accel_item->time, 2);
                        lcd_print_half_width2(LCD_CURSOR_POS_11, len, POS_NONE, POS_SEC, ALIGN_RIGHT);

                    } else {
                        // trip_item
                        trip_time = &trip_item->start_time;

                        print_trip_t *pt;
                        if (trip_item->status == JOURNAL_ITEM_V1) {
                            fill_print_trip(&ptrip, &trip_item->trip);
                            pt = &ptrip;
                        } else {
                            pt = &trip_item->ptrip;
                        }

                        switch(item_page) {
                            case 0:
                                print_trip_odometer(LCD_CURSOR_POS_10, pt, ALIGN_LEFT);
                                print_trip_average_fuel(LCD_CURSOR_POS_11, pt, ALIGN_RIGHT);
                                break;
                            case 1:
                                print_trip_average_speed(LCD_CURSOR_POS_10, pt, ALIGN_LEFT);
                                print_trip_time(LCD_CURSOR_POS_11, pt, ALIGN_RIGHT);
                                break;
                            case 2:
                                print_trip_total_fuel(LCD_CURSOR_POS_10, pt, ALIGN_LEFT);
                                lcd_print_half_width(LCD_CURSOR_POS_11, 0, ALIGN_RIGHT);
                                break;
                        }
                    }

                    len = print_index_number(jr.item_num + 1);
                    len += journal_print_item_time((char *) &buf[len], trip_time);

                    lcd_print_full_width(LCD_CURSOR_POS_00, len, ALIGN_LEFT);

                    if (key2_press != 0) {
                        if (journal_type != 3 && ++item_page > 2) {
                            item_page = 0;
                        }
                        timeout_timer1 = JOURNAL_IDLE_TIMEOUT;
                        screen_refresh = 1;
                    }
                }
            }

            if (jr.item_current == 0xFF) {
                // no items for current journal
                lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) journal_viewer_items_array, journal_type + 1), ALIGN_LEFT);
                lcd_print_full_width(LCD_CURSOR_POS_10, strcpy2(buf, (char *) &journal_viewer_no_items_string, 0), ALIGN_LEFT);

                if (key2_press != 0) {
                    timeout_timer1 = 0;
                    screen_refresh = 1;
                }
            }
            break;
    }
}

#endif

void config_screen_fuel_constant() {
    config.fuel_const = edit_value_char(config.fuel_const, CHAREDIT_MODE_NONE, 1, 255);
}

void config_screen_vss_constant() {
    config.odo_const = (uint16_t) edit_value_long(config.odo_const, 29999L LONG_VALUE_EDITOR_DEFAULT_FRACTION);
}

void config_screen_total_trip() {
    config.odo = (uint24_t) edit_value_long((uint24_t) config.odo, 999999L LONG_VALUE_EDITOR_DEFAULT_FRACTION);
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

flag_t config_temperature_conv_fl;

/**
 * extended version of temp sensors' configuration (use onewire search)
 * all sensors can be connected at once
 */
void config_screen_temp_sensors() {
    unsigned char tbuf[24];

    uint16_t _temps[3] = {DS18B20_TEMP_NONE, DS18B20_TEMP_NONE, DS18B20_TEMP_NONE};
    uint8_t _t_num[3] = {0, 0, 0};
    uint8_t current_device = 0;
    
#if defined(HW_LEGACY)
    // bad timings for pic (?) - wait for sound off and disable interrupts while 1-wire searching
    while (buzzer_fl != 0) {};
    HW_disable_interrupts();
    uint8_t num_devices = onewire_search_devices((uint8_t *) tbuf, 3);
    HW_enable_interrupts();
#else
    uint8_t num_devices = onewire_search_devices((uint8_t *) tbuf, 3);
#endif

    ds18b20_start_conversion(); config_temperature_conv_fl = 0; timeout_timer2 = 100;

    timeout_timer1_loop(DEFAULT_TIMEOUT) {
        
        if (num_devices != 0) {

            if (config_temperature_conv_fl == 0 && timeout_timer2 == 0) {
                config_temperature_conv_fl = 1;
                for (uint8_t i = 0; i < 3; i++) {
                    if (ds18b20_read_temp_matchrom((unsigned char *) &tbuf[i * 8], &_t) != 0) {
                        _temps[i] = _t;
                    }
                }
            }

            handle_keys_next_prev(&current_device, 0, num_devices - 1, DEFAULT_TIMEOUT);

            if (key2_press != 0) {
                if (_t_num[current_device]++ == 3) {
                    _t_num[current_device] = 0;
                }
                timeout_timer1 = DEFAULT_TIMEOUT;
            }

            _memset(buf, ' ', 16);
            // print temp for current device (if converted)
            if (config_temperature_conv_fl != 0) {
                temps[TEMP_CONFIG] = _temps[current_device];
                print_temp(LCD_CURSOR_POS_NONE, TEMP_CONFIG, ALIGN_NONE);
            }

            // print t.sensor + number/total
            strcpy2(buf, (char *) &temp_sensor_string, 0);
            buf[9]= '1' + current_device;
            buf[10] = '/';
            buf[11] = '0' + num_devices;

            lcd_print_full_width(LCD_CURSOR_POS_00, LCD_WIDTH, ALIGN_NONE);

            // convert id to hex string
            llptrtohex((unsigned char*) &tbuf[current_device * 8], (unsigned char*) buf);

            // print sensor string (in/out/eng)
            uint8_t len = strcpy2(&buf[12], (char *) &temp_sensors_array, _t_num[current_device] + 1);
            add_leading_symbols(&buf[12], ' ', len, 4);

        } else {
            lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) config_menu_array, TEMP_SENSOR_INDEX), ALIGN_LEFT);
            _memset(buf, ' ', 16);
            strcpy2(buf, (char *) &temp_no_sensors, 0);
        }

        lcd_print_full_width(LCD_CURSOR_POS_10, LCD_WIDTH, ALIGN_NONE);
        
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
flag_t config_temperature_conv_fl;
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

    uint8_t t_num = 0;

    timeout_timer1_loop(DEFAULT_TIMEOUT) {

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
            timeout_timer1 = DEFAULT_TIMEOUT;
        }

#if defined(DS18B20_CONFIG_SHOW_TEMP)        
        if (config_temperature_conv_fl != 0) {
            print_temp(LCD_CURSOR_POS_NONE, TEMP_CONFIG, ALIGN_NONE);
        } else {
            _memset(buf, ' ', 16);
        }
        strcpy2(buf, (char *) config_menu_array, TEMP_SENSOR_INDEX);
        lcd_print_full_width(LCD_CURSOR_POS_00, 16, ALIGN_NONE);
#else
        lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) config_menu_array, TEMP_SENSOR_INDEX), ALIGN_LEFT);
#endif

        llptrtohex((unsigned char*) tbuf, (unsigned char*) buf);
        len = strcpy2(&buf[12], (char *) &temp_sensors_array, t_num + 1);
        add_leading_symbols(&buf[12], ' ', len, 4);
        
        lcd_print_full_width(LCD_CURSOR_POS_10, LCD_WIDTH, ALIGN_NONE);

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

    handle_keys_next_prev(&c_sub_item, 0, MAX_SUB_ITEM, DEFAULT_TIMEOUT);
    
    uint8_t len = print_index_number(c_sub_item + 1);
    len += strcpy2(&buf[len], (char *) &service_counters_array, c_sub_item + 1);

    lcd_print_full_width(LCD_CURSOR_POS_10, len, ALIGN_LEFT);

    if (key2_press != 0) {
        key2_press = 0;

        lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) &service_counters_array, c_sub_item + 1), ALIGN_LEFT);
        lcd_print_full_width(LCD_CURSOR_POS_10, 0, ALIGN_NONE);

        if (c_sub_item == 0) {
            services.mh.limit = (unsigned short) edit_value_long(services.mh.limit, 1999L LONG_VALUE_EDITOR_DEFAULT_FRACTION);
        } else {
            services.srv[c_sub_item - 1].limit = edit_value_char(services.srv[c_sub_item - 1].limit, CHAREDIT_MODE_10000KM, 0, 60);
        }

        timeout_timer1 = DEFAULT_TIMEOUT;
    }
    
}
#endif

void config_screen_ua_const() {
    handle_keys_up_down(&config.vcc_const, VOLTAGE_ADJUST_CONST_MIN, VOLTAGE_ADJUST_CONST_MAX, DEFAULT_TIMEOUT);

    print_voltage(LCD_CURSOR_POS_10 + LCD_WIDTH / 4, (uint16_t *) &adc_voltage.current, POS_NONE, ALIGN_RIGHT);
}

void config_screen_version() {
    if (key1_press != 0 || key2_press != 0) {
        timeout_timer1 = 0;
    }
    lcd_print_full_width(LCD_CURSOR_POS_10, strcpy2(buf, (char*) &version_string, 0), ALIGN_LEFT);
}

void config_screen_items() {
    lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) &config_menu_title_string, 0), ALIGN_LEFT);

    uint8_t len = print_index_number(current_item_config->page.index + 1);
    len += strcpy2(&buf[len], (char *) config_menu_array, current_item_config->page.title_string_index);
    lcd_print_full_width(LCD_CURSOR_POS_10, len, ALIGN_LEFT);
}

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

void print_warning_service_counters(uint8_t warn) {
    uint8_t i;
    for (i = 0; i < 5; i++) {
        if ((warn & 0x01) != 0) {
#ifdef SOUND_SUPPORT
            buzzer_mode_index = BUZZER_WARN;
#endif
            lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char*) &warning_string, 0), ALIGN_CENTER);
            lcd_print_full_width(LCD_CURSOR_POS_10, strcpy2(buf, (char*) &service_counters_array, i + 1), ALIGN_CENTER);
            
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
      HW_snd_on();
    } else {
      HW_snd_off();
    }
    for (r = 0; r < tone; r++) {
      _delay_us(15);
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
    while (HW_key_ok_pressed()) {
        uint8_t keytime = SETTINGS_DELAY;
        while (HW_key_ok_pressed()) {
            HW_delay_ms(10);
            if (--keytime == 0) {
                keytime = SETTINGS_DELAY;
                if (stage_setting < (FORCE_SETTING_MAX - 1)) {
                    stage_setting++;
                    for (uint8_t i = 0; i < stage_setting; i++) {
                        beep(BEEP_OK);
                        HW_delay_ms(150);
                    }
                } else {
                    while (HW_key_ok_pressed()) {};
                    stage_setting = 0;
                }
            }
        }
        if (!HW_key_ok_pressed() && stage_setting != 0) {
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

void set_params() {
    // set core constants
    set_consts();

    params.main = config.selected_param.main_param;
    params.main_add = config.selected_param.main_add_param;
#ifdef SERVICE_COUNTERS_SUPPORT
    service_param = config.selected_param.service_param;
#endif
}

void handle_misc_values() {

    if (data.speed >= config.selected_param.min_speed * 10) {
        fuel_instant_pos = POS_LKM;
        drive_min_speed_fl = 1;
    } else {
        fuel_instant_pos = POS_LH;
        drive_min_speed_fl = 0;
    }

#ifdef CONTINUOUS_DATA_SUPPORT
    if (data.cd_speed >= config.selected_param.min_speed * 10) {
        cd_fuel_instant_pos = POS_LKM;
    } else {
        cd_fuel_instant_pos = POS_LH;
    }
#endif

#ifdef TEMPERATURE_SUPPORT
    main_temp_index = (config.settings.show_inner_temp == 0 ? TEMP_OUT : TEMP_IN) | PRINT_TEMP_PARAM_FRACT | PRINT_TEMP_PARAM_DEG_SIGN;
#endif    

    if (trips.tripA.odo > MAX_ODO_TRIPA) {
        clear_trip(true, &trips.tripA);
    }

    if (trips.tripB.odo > MAX_ODO_TRIPB) {
        clear_trip(true, &trips.tripB);
    }

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

    set_params();

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
    HW_disable_interrupts();

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

    HW_pwr_off();

    while (1);
}

void main() {
    static flag_t config_mode;
    static flag_t item_change_fl;
    static flag_t mode_change_fl;

    uint8_t c_item = 0, c_item_prev = 0;
    uint8_t prev_main_item = 0, prev_config_item = 0;
    uint8_t max_item = 0;

    power_on();
    
    HW_start_main_timer();

    HW_enable_interrupts();

#ifdef TEMPERATURE_SUPPORT
    // wait 1 sec before first conversion's request
    timeout_temperature = 1;
#endif        

#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
    if (config.settings.service_alarm) {
        uint8_t warn = check_service_counters();
        print_warning_service_counters(warn);
    }
#endif
    
    //clear_keys_state();
    
    // wait first adc conversion
    while (adc_voltage.current == 0 && screen_refresh == 0)
        ;
    
    // force handle_misc_values
    screen_refresh = 1;
    
    while (1) {
        if (screen_refresh != 0) {           
            screen_refresh = 0;
            fill_live_data();
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

            current_item_main = &items_main[c_item];

            while (current_item_main->page.skip != 0 || (drive_min_speed_fl != 0 && current_item_main->page.drive_mode == 0)) {
#ifdef KEY3_SUPPORT
                    if (c_item < c_item_prev) {
                        if (c_item-- == 0) {
                            c_item = max_item;
                        }
                    } else
#endif
                    {
                        if (c_item++ == max_item) {
                            c_item = 0;
                        }
                    }
                current_item_main = &items_main[c_item];
            }
        } else {
            max_item = sizeof (items_config) / sizeof (items_config[0]) - 1;
            current_item_config = &items_config[c_item];
            current_item_config->page.index = c_item;
        }

        if (mode_change_fl == 0) {
            if (item_change_fl == 0 && key2_longpress != 0) {
                // long keypress for service key - switch service mode and main mode
                if (config_mode == 0 && motor_fl == 0 && drive_fl == 0 && (current_item_main->page.config_switch != 0 && current_item_main->page.skip_key_handler == 0)) {
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
                    set_params();
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

                // skip next/prev key if skip_key_handler is set
                if (config_mode != 0 || current_item_main->page.skip_key_handler == 0) {
                    handle_keys_next_prev(&c_item, 0, max_item, DEFAULT_TIMEOUT);
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
                    // config screen items
                    config_screen_items();

                    // config screen items' editor
                    if (key2_press != 0) {
                        key2_press = 0;
                        LCD_Clear();
                        timeout_timer1_loop(DEFAULT_TIMEOUT) {
                            lcd_print_full_width(LCD_CURSOR_POS_00, strcpy2(buf, (char *) config_menu_array, current_item_config->page.title_string_index), ALIGN_LEFT);
                            current_item_config->screen();
                            LCD_cursor_off();
                            wait_refresh_timeout();
                        }
                        screen_refresh = 1;
                    }
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
