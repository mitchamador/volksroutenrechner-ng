#ifndef LOCALE_H
#define	LOCALE_H

#include "version.h"

typedef enum {
    POS_KMH=1,        
    POS_OMIN,        
    POS_LKM,        
    POS_LH,        
    POS_HOUR,        
    POS_LITR,        
    POS_VOLT,        
    POS_KM,        
    POS_SEC,        
    POS_CELS,        
    POS_MIN,
    POS_MAX,        
    POS_MS,        
    POS_NONE
} symbols_t;

//#define POS_MIN POS_NONE
//#define POS_MAX POS_NONE

#define SYMBOLS_STR {                             \
        /* POS_KMH=1   */ 0xFF, 0x00, 0x01,       \
        /* POS_OMIN=2  */ 0xFF, 0x02, 0x03,       \
        /* POS_LKM=3   */ 0xFF, 0x04, 0x05,       \
        /* POS_LH=4    */ 0xFF, 0x06, 0x07,       \
        /* POS_HOUR=5  */ 0xFF, 'h',              \
        /* POS_LITR=6  */ 0xFF, 'l',              \
        /* POS_VOLT=7  */ 0xFF, 'V',              \
        /* POS_KM=8    */ 0xFF, 'k', 'm',         \
        /* POS_SEC=9   */ 0xFF, 's',              \
        /* POS_CELS=10 */ 0xFF, 0xDF,             \
        /* POS_MIN=11  */ 0xFF, '>',              \
        /* POS_MAX=12  */ 0xFF, '<',              \
        /* POS_MS=13   */ 0xFF, 'm', 's',         \
        /* POS_NONE=14 */ 0xFF,                   \
        0xFF,                                     \
}

typedef enum {
    TRIPS_POS_CURR=1,
    TRIPS_POS_DAY,
    TRIPS_POS_A,
    TRIPS_POS_B,
} trips_name_t;

#define TRIPS_STR "\0cur\0day\0A\0B"

#define EMPTY_STRING "----"
#define NO_TIME_STRING "-----'--"
#define TRIP_STRING "trip "
#define ONOFF_STRING "\0 off\0  on"
#define TIME_CORRECTION "time correction?"
#define RESET_STRING "reset?"
#define VOLTAGE_STRING "voltage"
#ifdef FUEL_TANK_SUPPORT
#define CONTINUOUS_DATA_STRING "cont./fuel"
#else
#define CONTINUOUS_DATA_STRING "cont. data"
#endif

#define ACCEL_MEAS_STRING "\0 0-100\0 0-60\0 60-100\0 80-120"
#define ACCEL_MEAS_STRING_TIMING "timing"
#define ACCEL_MEAS_SIMPLE_STRING "0-100 timing"
#define ACCEL_MEAS_WAIT_STRING "wait for start"
#define TIMEOUT_STRING "timeout" 

#define WARNING_STR "warning"

#define CONFIG_MENU_TITLE "config menu"
#define SERVICE_COUNTERS "\0engine hours\0engine oil\0gearbox oil\0air filter\0spark plugs"

#define SETTING_SHOW_MISC_SCREEN        "\0misc screen"

#ifdef TEMPERATURE_SUPPORT
#define SETTING_INOUT_TEMP              "\0in/out temp"
#ifdef DS3231_TEMP
#define SETTING_DS3231_TEMP             "\0ds3231 temp"
#endif
#endif

#ifndef SETTING_INOUT_TEMP
#define SETTING_INOUT_TEMP              "\0"
#endif
#ifndef DS3231_TEMP
#define SETTING_DS3231_TEMP             "\0"
#endif

#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
#define SETTING_SERVICE_ALARM           "\0serv alarm"
#else
#define SETTING_SERVICE_ALARM           "\0"
#endif

#ifdef SOUND_SUPPORT
#define SETTING_KEY_SOUND               "\0key sound"
#else
#define SETTING_KEY_SOUND               "\0"
#endif

#ifdef FUEL_TANK_SUPPORT
#define SETTING_ADC_FUEL_NORMALIZE      "\0adc fuel nor"
#else
#define SETTING_ADC_FUEL_NORMALIZE      "\0"
#endif

#define SETTINGS_BITS "\0pair/par inj" SETTING_SHOW_MISC_SCREEN SETTING_KEY_SOUND SETTING_SERVICE_ALARM "\0mh rpm\0trip B month\0trip C day" SETTING_INOUT_TEMP SETTING_DS3231_TEMP SETTING_ADC_FUEL_NORMALIZE "\0\0\0\0\0\0"

#define TEMP_SENSORS "\0---\0out\0in\0 eng\0"
#define TEMP_NO_SENSORS "no sensors found"

// 8 symbols
#define JOURNAL_MARK "JOURTRIP"
#define JOURNAL_VIEWER "journal viewer"
#define JOURNAL_VIEWER_ITEMS "\0current trip\0trip A\0trip B\0accel logger"
#define JOURNAL_VIEWER_NO_ITEMS "no items"

typedef enum {

    FUEL_CONSTANT_INDEX = 1,
#define FUEL_CONSTANT_STR "\0fuel constant"

    VSS_CONSTANT_INDEX,
#define VSS_CONSTANT_STR "\0vss constant"

    TOTAL_TRIP_INDEX,
#define TOTAL_TRIP_STR "\0total trip"

    VOLTAGE_ADJUST_INDEX,
#define VOLTAGE_ADJUST_STR "\0voltage adjust"

    SETTINGS_BITS_INDEX,
#define SETTINGS_BITS_STR "\0settings bits"

#if defined(DS18B20_CONFIG)
    TEMP_SENSOR_INDEX,
#define TEMP_SENSOR_STR "\0temp sensors"
#else
#define TEMP_SENSOR_STR
#endif

#if defined(SERVICE_COUNTERS_CHECKS_SUPPORT)
    SERVICE_COUNTERS_INDEX,
#define SERVICE_COUNTERS_STR "\0service cntrs"
#else
#define SERVICE_COUNTERS_STR
#endif

#if defined(MIN_SPEED_CONFIG)
    MIN_SPEED_INDEX,
#define MIN_SPEED_STR "\0min speed"
#else
#define MIN_SPEED_STR
#endif

    VERSION_INFO_INDEX,
#define VERSION_INFO_STR "\0sw version"

} services_str_t;

#define SERVICES_STR FUEL_CONSTANT_STR VSS_CONSTANT_STR TOTAL_TRIP_STR VOLTAGE_ADJUST_STR SETTINGS_BITS_STR TEMP_SENSOR_STR SERVICE_COUNTERS_STR MIN_SPEED_STR VERSION_INFO_STR

#define DAY_OF_WEEK_STR "\0sunday\0monday\0tuesday\0wednesday\0thursday\0friday\0saturday"
#define MONTH_STR "\0jan\0feb\0mar\0apr\0may\0jun\0jul\0aug\0sep\0oct\0nov\0dec"

#define TEMP_SENSOR "t.sensor"

#define TEMP_SENSOR "t.sensor"

#ifndef __AVR
#define PROGMEM
#endif

PROGMEM const char symbols_str[] = SYMBOLS_STR;
PROGMEM const char trips_str[] = TRIPS_STR;

PROGMEM const char version_str[] = VERSION_STRING;

PROGMEM const char empty_string[] = EMPTY_STRING;
PROGMEM const char no_time_string[] = NO_TIME_STRING;
PROGMEM const char trip_string[] = TRIP_STRING;
PROGMEM const char onoff_string[] = ONOFF_STRING;
PROGMEM const char time_correction[] = TIME_CORRECTION;
PROGMEM const char reset_string[] = RESET_STRING;
PROGMEM const char voltage_string[] = VOLTAGE_STRING;
PROGMEM const char continuous_data_string[] = CONTINUOUS_DATA_STRING;
#ifdef SIMPLE_ACCELERATION_MEASUREMENT
PROGMEM const char accel_meas_string[] = ACCEL_MEAS_SIMPLE_STRING;
#else
PROGMEM const char accel_meas_string[] = ACCEL_MEAS_STRING;
PROGMEM const char accel_meas_timing_string[] = ACCEL_MEAS_STRING_TIMING;
#endif
PROGMEM const char accel_meas_wait_string[] = ACCEL_MEAS_WAIT_STRING;
PROGMEM const char timeout_string[] = TIMEOUT_STRING;
#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
PROGMEM const char warning_str[] = WARNING_STR;
#endif
PROGMEM const char service_menu_title[] = CONFIG_MENU_TITLE;
PROGMEM const char service_menu_str[] = SERVICES_STR;
PROGMEM const char service_counters[] = SERVICE_COUNTERS;

PROGMEM const char settings_bits[] = SETTINGS_BITS;
#ifdef TEMPERATURE_SUPPORT
PROGMEM const char temp_sensors[] = TEMP_SENSORS;
#ifdef DS18B20_CONFIG_EXT
PROGMEM const char temp_no_sensors[] = TEMP_NO_SENSORS;
PROGMEM const char temp_sensor[] = TEMP_SENSOR;
#endif
#endif
PROGMEM const char day_of_week_str[] = DAY_OF_WEEK_STR;
PROGMEM const char month_str[] = MONTH_STR;

#ifdef JOURNAL_SUPPORT
const char journal_mark_str[] = JOURNAL_MARK;
PROGMEM const char journal_viewer_str[] = JOURNAL_VIEWER;
PROGMEM const char journal_viewer_items_str[] = JOURNAL_VIEWER_ITEMS;
PROGMEM const char journal_viewer_no_items_str[] = JOURNAL_VIEWER_NO_ITEMS;
#endif

#endif	/* LOCALE_H */

