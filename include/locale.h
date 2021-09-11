#ifndef LOCALE_H
#define	LOCALE_H

// custom lcd characters
// 0x00
#define DATA_KMH_0  0x05, 0x06, 0x05, 0x00, 0x00, 0x00, 0x01, 0x02
// 0x01
#define DATA_KMH_1  0x0A, 0x15, 0x15, 0x00, 0x10, 0x05, 0x07, 0x01
// 0x02
#define DATA_OMIN_0 0x07, 0x05, 0x07, 0x00, 0x00, 0x01, 0x02, 0x00
// 0x03
#define DATA_OMIN_1 0x00, 0x00, 0x08, 0x10, 0x00, 0x0A, 0x15, 0x15
// 0x04
#define DATA_L100_0 0x0C, 0x14, 0x14, 0x01, 0x02, 0x05, 0x01, 0x01
// 0x05
#define DATA_L100_1 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x15, 0x1F
// 0x06
#define DATA_LH_0   0x03, 0x05, 0x05, 0x00, 0x00, 0x01, 0x02, 0x00
// 0x07
#define DATA_LH_1   0x00, 0x00, 0x08, 0x10, 0x00, 0x14, 0x1C, 0x04

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
        /* POS_NONE=12 */ 0xFF,                   \
        0xFF,                                     \
}                                                 \

#define EMPTY_STRING "----";
#define NO_TIME_STRING "-----'--";
#define TRIP_STRING "trip ";
#define ONOFF_STRING "\0 off\0  on";
#define TIME_CORRECTION "time correction?";
#define RESET_STRING "reset?"; 
#define SPEED100_STRING "0-100 timing"; 
#define SPEED100_WAIT_STRING "wait for start"; 
#define TIMEOUT_STRING "timeout"; 
#define WARNING_STR "WARNING"; 

#define SERVICE_MENU_TITLE "SERVICE MENU";
#define SERVICE_COUNTERS "\0engine hours\0engine oil\0gearbox oil\0air filter\0spark plugs";

#define SETTINGS_BITS "\0dual inj\0skip temp\0key sound\0serv alarm\0fast refresh\0mh rpm\0alt buttons\0";

#define TEMP_SENSORS "\0---\0out\0in\0eng";

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

extern const char symbols_str[];

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
extern const char service_menu_str[];
extern const char service_counters[];

extern const char settings_bits[];

extern const char temp_sensors[];

extern const char day_of_week_str[];
extern const char month_str[];

#endif	/* LOCALE_H */

