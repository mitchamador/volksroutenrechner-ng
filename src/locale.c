#include "main.h"
#include "locale.h"
#include "version.h"
#ifndef __AVR_ATMEGA
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
PROGMEM const char warning_str[] = WARNING_STR; 

PROGMEM const char service_menu_title[] = CONFIG_MENU_TITLE;
PROGMEM const char service_menu_str[] = SERVICES_STR;
PROGMEM const char service_counters[] = SERVICE_COUNTERS;

PROGMEM const char settings_bits[] = SETTINGS_BITS;

PROGMEM const char temp_sensors[] = TEMP_SENSORS;

PROGMEM const char day_of_week_str[] = DAY_OF_WEEK_STR;
PROGMEM const char month_str[] = MONTH_STR;
