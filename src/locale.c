#include "locale.h"
#include "hw.h"
#include "version.h"
#ifndef __AVR_ATMEGA
#define PROGMEM
#endif

PROGMEM const char symbols_str[] = SYMBOLS_STR;

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
PROGMEM const char service_menu_str[] = SERVICES_STR;
PROGMEM const char service_counters[] = SERVICE_COUNTERS;

PROGMEM const char settings_bits[] = SETTINGS_BITS;

PROGMEM const char temp_sensors[] = TEMP_SENSORS;

PROGMEM const char day_of_week_str[] = DAY_OF_WEEK_STR;
PROGMEM const char month_str[] = MONTH_STR;
