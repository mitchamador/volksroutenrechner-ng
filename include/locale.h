#ifndef LOCALE_H
#define	LOCALE_H

#define HOUR_SYMBOL 'h'
#define LITRE_SYMBOL 'l'
#define VOLT_SYMBOL 'V'
#define KM1_SYMBOL 'k'
#define KM2_SYMBOL 'm'
#define SECONDS_SYMBOL 's'
#define CELSIUS_SYMBOL 0xDF
#define MAX_SPEED_SYMBOL '<'

// custom lcd characters values
#define _kmh0   0x00
#define _kmh1	0x01
#define _omin0	0x02
#define _omin1	0x03
#define _lkm0	0x04
#define _lkm1	0x05
#define _lh0	0x06
#define _lh1    0x07

#define EMPTY_STRING "----";
#define NO_TIME_STRING "-----'--";
#define TRIP_STRING "trip ";
#define ONOFF_STRING "\0 OFF\0  ON";
#define TIME_CORRECTION "time correction?";
#define RESET_STRING "reset?"; 
#define SPEED100_STRING "0-100 timing"; 
#define SPEED100_WAIT_STRING "wait for start"; 
#define TIMEOUT_STRING "timeout"; 
#define WARNING_STR "WARNING"; 

#define SERVICE_MENU_TITLE "SERVICE MENU";
#define SERVICE_COUNTERS "\0engine hours\0engine oil\0gearbox oil\0air filter\0spark plugs";

#define SETTINGS_BITS "\0dual inj\0skip temp\0key sound\0serv alarm\0fast refresh\0alt buttons\0\0";

#define TEMP_SENSORS "\0---\0out\0in\0eng";

#define FUEL_CONSTANT_STR "FUEL constant";
#define VSS_CONSTANT_STR "VSS constant";
#define TOTAL_TRIP_STR "total trip";
#define VOLTAGE_ADJUST_STR "voltage adjust";
#define SETTINGS_BITS_STR "settings bits";
#define TEMP_SENSOR_STR "temp sensors";
#define SERVICE_COUNTERS_STR "service cntrs";
#define MIN_SPEED_STR "min speed";
#define VERSION_INFO_STR "sw version";

#define DAY_OF_WEEK_STR "\0sunday\0monday\0tuesday\0wednesday\0thursday\0friday\0saturday";
#define MONTH_STR "\0jan\0feb\0mar\0apr\0may\0jun\0jul\0aug\0sep\0oct\0nov\0dec";

#define DATA_KMH_0  0x05, 0x06, 0x05, 0x00, 0x00, 0x00, 0x01, 0x02
#define DATA_KMH_1  0x0A, 0x15, 0x15, 0x00, 0x10, 0x05, 0x07, 0x01
#define DATA_OMIN_0 0x07, 0x05, 0x07, 0x00, 0x00, 0x01, 0x02, 0x00
#define DATA_OMIN_1 0x00, 0x00, 0x08, 0x10, 0x00, 0x0A, 0x15, 0x15
#define DATA_L100_0 0x0C, 0x14, 0x14, 0x01, 0x02, 0x05, 0x01, 0x01
#define DATA_L100_1 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x15, 0x1F
#define DATA_LH_0   0x03, 0x05, 0x05, 0x00, 0x00, 0x01, 0x02, 0x00
#define DATA_LH_1   0x00, 0x00, 0x08, 0x10, 0x00, 0x14, 0x1C, 0x04

extern const char version_str[]; 

extern const char empty_string[];
extern const char no_time_string[];
extern const char trip_string[];
extern const char onoff_string[];
extern const char time_correction[];
extern const char reset_string[]; 
extern const char speed100_string[]; 
extern const char speed100_wait_string[]; 
extern const char timeout_string[]; 
extern const char warning_str[]; 

extern const char service_menu_title[];
extern const char service_counters[];

extern const char settings_bits[];

extern const char temp_sensors[];

extern const char fuel_constant_str[];
extern const char vss_constant_str[];
extern const char total_trip_str[];
extern const char voltage_adjust_str[];
extern const char settings_bits_str[];
extern const char temp_sensor_str[];
extern const char service_counters_str[];
extern const char min_speed_str[];
extern const char version_info_str[];

extern const char day_of_week_str[];
extern const char month_str[];

#endif	/* LOCALE_H */

