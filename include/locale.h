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
    POS_MAXS,        
    POS_MS,        
    POS_NONE,
} symbols_t;

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
        /* POS_MAXS=11 */ 0xFF, '<',              \
        /* POS_MAXS=12 */ 0xFF, 'm', 's',         \
        /* POS_NONE=13 */ 0xFF,                   \
        0xFF,                                     \
}

typedef enum {
    TRIPS_POS_CURR=1,
    TRIPS_POS_DAY,
    TRIPS_POS_A,
    TRIPS_POS_B,
} trips_name_t;

#define TRIPS_STR "\0cur\0day\0A\0B"

#define EMPTY_STRING "----";
#define NO_TIME_STRING "-----'--";
#define TRIP_STRING "trip ";
#define ONOFF_STRING "\0 off\0  on";
#define TIME_CORRECTION "time correction?";
#define RESET_STRING "reset?"; 

#define ACCEL_MEAS_STRING "\0 0-100\0 0-60\0 60-100\0 80-120"; 
#define ACCEL_MEAS_STRING_TIMING "timing"; 
#define ACCEL_MEAS_SIMPLE_STRING "0-100 timing"; 
#define ACCEL_MEAS_WAIT_STRING "wait for start"; 
#define TIMEOUT_STRING "timeout"; 

#define WARNING_STR "WARNING"; 

#define CONFIG_MENU_TITLE "CONFIG MENU";
#define SERVICE_COUNTERS "\0engine hours\0engine oil\0gearbox oil\0air filter\0spark plugs";

// xc8 going crazy with pic16f876a target with length of this string (1347 error)
#define SETTINGS_BITS "\0pair/par inj\0skip temp\0key sound\0serv alarm\0dummy\0mh rpm\0daily trip\0in/out temp\0ds3231 temp\0\0\0\0\0\0\0";

#define TEMP_SENSORS "\0---\0out\0in\0eng";
#define TEMP_NO_SENSORS "no sensors found"

typedef enum {
    FUEL_CONSTANT_INDEX=1,        
    VSS_CONSTANT_INDEX,        
    TOTAL_TRIP_INDEX,        
    VOLTAGE_ADJUST_INDEX,        
    SETTINGS_BITS_INDEX,        
    TEMP_SENSOR_INDEX,
    SERVICE_COUNTERS_INDEX,        
    MIN_SPEED_INDEX,        
    VERSION_INFO_INDEX,        
} services_str_t;

#define SERVICES_STR                                                   \
        /*FUEL_CONSTANT_INDEX*/     "\0FUEL constant"                  \
        /*VSS_CONSTANT_INDEX*/      "\0VSS constant"                   \
        /*TOTAL_TRIP_INDEX*/        "\0total trip"                     \
        /*VOLTAGE_ADJUST_INDEX*/    "\0voltage adjust"                 \
        /*SETTINGS_BITS_INDEX*/     "\0settings bits"                  \
        /*TEMP_SENSOR_INDEX*/       "\0temp sensors"                   \
        /*SERVICE_COUNTERS_INDEX*/  "\0service cntrs"                  \
        /*MIN_SPEED_INDEX*/         "\0min speed"                      \
        /*VERSION_INFO_INDEX*/      "\0sw version"                     \

#define DAY_OF_WEEK_STR "\0sunday\0monday\0tuesday\0wednesday\0thursday\0friday\0saturday";
#define MONTH_STR "\0jan\0feb\0mar\0apr\0may\0jun\0jul\0aug\0sep\0oct\0nov\0dec";

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
#endif
#endif
PROGMEM const char day_of_week_str[] = DAY_OF_WEEK_STR;
PROGMEM const char month_str[] = MONTH_STR;

#endif	/* LOCALE_H */

