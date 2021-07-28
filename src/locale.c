#include "locale.h"
#include "hw.h"
#include "version.h"
#ifndef __AVR_ATMEGA
#define PROGMEM
#endif

PROGMEM const char version_str[] = VERSION_STRING; 

PROGMEM const char empty_string[] = EMPTY_STRING;
PROGMEM const char no_time_string[] = NO_TIME_STRING;
PROGMEM const char trip_string[] = TRIP_STRING;
PROGMEM const char onoff_string[] = ONOFF_STRING;
PROGMEM const char time_correction[] = TIME_CORRECTION;
PROGMEM const char reset_string[] = RESET_STRING; 
PROGMEM const char speed100_string[] = SPEED100_STRING; 
PROGMEM const char speed100_wait_string[] = SPEED100_WAIT_STRING; 
PROGMEM const char timeout_string[] = TIMEOUT_STRING; 
PROGMEM const char warning_str[] = WARNING_STR; 

PROGMEM const char service_menu_title[] = SERVICE_MENU_TITLE;
PROGMEM const char service_counters[] = SERVICE_COUNTERS;

PROGMEM const char settings_bits[] = SETTINGS_BITS;

PROGMEM const char temp_sensors[] = TEMP_SENSORS;

PROGMEM const char fuel_constant_str[] = FUEL_CONSTANT_STR;
PROGMEM const char vss_constant_str[] = VSS_CONSTANT_STR;
PROGMEM const char total_trip_str[] = TOTAL_TRIP_STR;
PROGMEM const char voltage_adjust_str[] = VOLTAGE_ADJUST_STR;
PROGMEM const char settings_bits_str[] = SETTINGS_BITS_STR;
PROGMEM const char temp_sensor_str[] = TEMP_SENSOR_STR;
PROGMEM const char service_counters_str[] = SERVICE_COUNTERS_STR;
PROGMEM const char min_speed_str[] = MIN_SPEED_STR;
PROGMEM const char version_info_str[] = VERSION_INFO_STR;

PROGMEM const char day_of_week_str[] = DAY_OF_WEEK_STR;
PROGMEM const char month_str[] = MONTH_STR;
