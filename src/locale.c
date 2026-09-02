#include "locale.h"

PROGMEM const char symbols_array[] = SYMBOLS_ARRAY;
PROGMEM const char trips_array[] = TRIPS_ARRAY;

PROGMEM const char version_string[] = VERSION;

PROGMEM const char empty_string[] = EMPTY;
PROGMEM const char no_time_string[] = NO_TIME;
PROGMEM const char trip_string[] = TRIP;
PROGMEM const char on_off_array[] = ON_OFF_ARRAY;
PROGMEM const char time_correction_string[] = TIME_CORRECTION;
PROGMEM const char reset_string[] = RESET_DATA;

#ifdef MIN_MAX_VOLTAGES_SUPPORT
PROGMEM const char voltage_string[] = VOLTAGE;
#endif

#ifdef CONTINUOUS_DATA_SUPPORT
PROGMEM const char continuous_data_string[] = CONTINUOUS_DATA;
#endif

#ifdef EXTENDED_ACCELERATION_MEASUREMENT
PROGMEM const char accel_meas_array[] = ACCEL_MEAS_ARRAY;
PROGMEM const char accel_meas_timing_string[] = ACCEL_MEAS_TIMING;
#else
PROGMEM const char accel_meas_string[] = ACCEL_MEAS_SIMPLE;
#endif

PROGMEM const char accel_meas_wait_string[] = ACCEL_MEAS_WAIT;
PROGMEM const char timeout_string[] = TIMEOUT;

#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
PROGMEM const char warning_string[] = WARNING;
#endif

PROGMEM const char config_menu_title_string[] = CONFIG_MENU_TITLE;
PROGMEM const char config_menu_array[] = CONFIG_MENU_ARRAY;
PROGMEM const char service_counters_array[] = SERVICE_COUNTERS_ARRAY;

PROGMEM const char settings_bits_array[] = SETTINGS_BITS_ARRAY;

#ifdef TEMPERATURE_SUPPORT
PROGMEM const char temp_sensors_array[] = TEMP_SENSORS_ARRAY;
#ifdef DS18B20_CONFIG_EXT
PROGMEM const char temp_no_sensors[] = TEMP_NO_SENSORS;
PROGMEM const char temp_sensor_string[] = TEMP_SENSOR;
#endif
#endif

PROGMEM const char day_of_week_array[] = DAY_OF_WEEK_ARRAY;
PROGMEM const char month_array[] = MONTH_ARRAY;

#ifdef JOURNAL_SUPPORT
PROGMEM const char journal_viewer_string[] = JOURNAL_VIEWER;
PROGMEM const char journal_viewer_items_array[] = JOURNAL_VIEWER_ITEMS_ARRAY;
PROGMEM const char journal_viewer_no_items_string[] = JOURNAL_VIEWER_NO_ITEMS;
#endif

