#ifndef LOCALE_H
#define	LOCALE_H

#include "version.h"

#ifndef __AVR
#define PROGMEM
#endif

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

typedef enum {
    TRIPS_POS_CURR=1,
    TRIPS_POS_DAY,
    TRIPS_POS_A,
    TRIPS_POS_B,
} trips_name_t;

typedef enum {
    FUEL_CONSTANT_INDEX = 1,
    VSS_CONSTANT_INDEX,
    TOTAL_TRIP_INDEX,
    VOLTAGE_ADJUST_INDEX,
    SETTINGS_BITS_INDEX,
#if defined(DS18B20_CONFIG)
    TEMP_SENSOR_INDEX,
#endif
#if defined(SERVICE_COUNTERS_CHECKS_SUPPORT)
    SERVICE_COUNTERS_INDEX,
#endif
#if defined(MIN_SPEED_CONFIG)
    MIN_SPEED_INDEX,
#endif
    VERSION_INFO_INDEX,
} services_str_t;

#define EMPTY                           "----"
#define NO_TIME                         "-----'--"

#define ACCEL_MEAS_ARRAY                "\0 0-100\0 0-60\0 60-100\0 80-120"

#if !defined(LOCALE_RUSSIAN)

// english locale

#define SYMBOLS_ARRAY {                           \
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

#define TRIPS_ARRAY                     "\0cur\0day\0A\0B"

#define TRIP                            "trip "

#define ON_OFF_ARRAY                    "\0 off\0  on"

#define TIME_CORRECTION                 "time correction?"

#define RESET_DATA                      "reset?"

#define VOLTAGE                         "voltage"

#ifdef FUEL_TANK_SUPPORT
#define CONTINUOUS_DATA                 "cont./fuel"
#else
#define CONTINUOUS_DATA                 "cont. data"
#endif

#define ACCEL_MEAS_TIMING               "timing"

#define ACCEL_MEAS_SIMPLE               "0-100 timing"

#define ACCEL_MEAS_WAIT                 "wait for start"

#define TIMEOUT                         "timeout" 

#define WARNING                         "warning"

#define CONFIG_MENU_TITLE               "config menu"

#define SERVICE_COUNTERS_ARRAY          "\0engine hours\0engine oil\0gearbox oil\0air filter\0spark plugs"

#define SETTING_PAIRPAR_INJ             "\0pair/par inj"

#ifdef JOURNAL_SUPPORT
#define SETTING_TRIPB_MONTH             "\0trip B month"
#endif

#define SETTING_TRIPC_DAY               "\0trip C day"

#ifdef TEMPERATURE_SUPPORT
#define SETTING_INOUT_TEMP              "\0in/out temp"
#endif

#ifdef DS3231_TEMP
#define SETTING_DS3231_TEMP             "\0ds3231 temp"
#endif

#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
#define SETTING_SERVICE_ALARM           "\0serv alarm"
#endif

#ifdef SOUND_SUPPORT
#define SETTING_KEY_SOUND               "\0key sound"
#endif

#ifdef FUEL_TANK_SUPPORT
#define SETTING_ADC_FUEL_NORMALIZE      "\0adc fuel nor"
#endif

#ifdef ENCODER_SUPPORT
#define SETTING_ENCODER                 "\0encoder"
#endif

#define TEMP_SENSORS_ARRAY              "\0---\0out\0in\0 eng\0"

#define TEMP_NO_SENSORS                 "no sensors found"

#define JOURNAL_VIEWER                  "journal viewer"

#define JOURNAL_VIEWER_ITEMS_ARRAY      "\0current trip\0trip A\0trip B\0accel logger"

#define JOURNAL_VIEWER_NO_ITEMS         "no items"

#define CONFIG_MENU_FUEL_CONSTANT       "\0fuel constant"

#define CONFIG_MENU_VSS_CONSTANT        "\0vss constant"

#define CONFIG_MENU_TOTAL_TRIP          "\0total trip"

#define CONFIG_MENU_VOLTAGE_ADJUST      "\0voltage adjust"

#define CONFIG_MENU_SETTINGS_BITS       "\0settings bits"

#if defined(DS18B20_CONFIG)
#define CONFIG_MENU_TEMP_SENSORS        "\0temp sensors"
#endif

#if defined(SERVICE_COUNTERS_CHECKS_SUPPORT)
#define CONFIG_MENU_SERVICE_COUNTERS    "\0service cntrs"
#endif

#if defined(MIN_SPEED_CONFIG)
#define CONFIG_MENU_MIN_SPEED           "\0min speed"
#endif

#define CONFIG_MENU_VERSION_INFO        "\0sw version"

#define DAY_OF_WEEK_ARRAY               "\0sunday\0monday\0tuesday\0wednesday\0thursday\0friday\0saturday"

#define MONTH_ARRAY                     "\0jan\0feb\0mar\0apr\0may\0jun\0jul\0aug\0sep\0oct\0nov\0dec"

#define TEMP_SENSOR                     "t.sensor"

#else /* LOCALE_RUSSIAN */

// 1602 russian locale

#define SYMBOLS_ARRAY     {                       \
        /* POS_KMH=1   */ 0xFF, 0x00, 0x01,       \
        /* POS_OMIN=2  */ 0xFF, 0x02, 0x03,       \
        /* POS_LKM=3   */ 0xFF, 0x04, 0x05,       \
        /* POS_LH=4    */ 0xFF, 0x06, 0x07,       \
        /* POS_HOUR=5  */ 0xFF, 0xC0,             \
        /* POS_LITR=6  */ 0xFF, 0xBB,             \
        /* POS_VOLT=7  */ 0xFF, 0xB3,             \
        /* POS_KM=8    */ 0xFF, 0xBA, 0xBC,       \
        /* POS_SEC=9   */ 0xFF, 0x63,             \
        /* POS_CELS=10 */ 0xFF, 0xDF,             \
        /* POS_MIN=11  */ 0xFF, '>',              \
        /* POS_MAX=12  */ 0xFF, '<',              \
        /* POS_MS=13   */ 0xFF, 0xBC, 0x63,       \
        /* POS_NONE=14 */ 0xFF,                   \
        0xFF,                                     \
}

// " тек дн А Б"
#define TRIPS_ARRAY                     "\0\xBF\x65\xBA\0\xE3\xBD\0A\0B"

// "проб "
#define TRIP                            "\xBE\x70\x6F\xB2. "

// "выкл  вкл"
#define ON_OFF_ARRAY                    "\0\xB3\xC3\xBA\xBB\0 \xB3\xBA\xBB"

// "корр. времени?"
#define TIME_CORRECTION                 "\xBA\x6F\x70\x70. \xB3\x70\x65\xBC\x65\xBD\xB8?"

// "сброс?"
#define RESET_DATA                      "\x63\xB2\x70\x6F\x63?"

// "напряж."
#define VOLTAGE                         "\xBD\x61\xBE\x70\xC7\xB6."

#ifdef FUEL_TANK_SUPPORT
// "длит./топл."
#define CONTINUOUS_DATA                 "\xE3\xBB\xB8\xBF./\xBF\x6F\xBE\xBB."
#else
// "длит. данные"
#define CONTINUOUS_DATA                 "\xE3\xBB\xB8\xBF. \xE3\x61\xBD\xBD\xC3\x65"
#endif

// "разгон"
#define ACCEL_MEAS_TIMING               "\x70\x61\xB7\xB4\x6F\xBD"

// "разгон 0-100"
#define ACCEL_MEAS_SIMPLE               "\x70\x61\xB7\xB4\x6F\xBD 0-100"

// "ожид. старта"
#define ACCEL_MEAS_WAIT                 "\x6F\xB6\xB8\xE3. \x63\xBF\x61\x70\xBF\x61"

// "таймаут"
#define TIMEOUT                         "\xBF\x61\xB9\xBC\x61\x79\xBF" 

// "внимание"
#define WARNING                         "\xB3\xBD\xB8\xBC\x61\xBD\xB8\x65"

// "настройки"
#define CONFIG_MENU_TITLE               "\xBD\x61\x63\xBF\x70\x6F\xB9\xBA\xB8"

// " моточасы масло двиг. масло кпп возд. фильтр свечи"
#define SERVICE_COUNTERS_ARRAY          "\0\xBC\x6F\xBF\x6F\xC0\x61\x63\xC3\0\xBC\x61\x63\xBB\x6F \xE3\xB3\xB8\xB4.\0\xBC\x61\x63\xBB\x6F \xBA\xBE\xBE\0\xB3\x6F\xB7\xE3. \xE4\xB8\xBB\xC4\xBF\x70\0\x63\xB3\x65\xC0\xB8"

// "пар. впрыск"
#define SETTING_PAIRPAR_INJ             "\0\xBE\x61\x70. \xB3\xBE\x70\xC3\x63\xBA"

// "проб. B мес."
#ifdef JOURNAL_SUPPORT
#define SETTING_TRIPB_MONTH             "\0\xBE\x70\x6F\xB2. B \xBC\x65\x63."
#endif

// "проб. C дн."
#define SETTING_TRIPC_DAY               "\0\xBE\x70\x6F\xB2. C \xE3\xBD."

#ifdef TEMPERATURE_SUPPORT
// "вн/нар темп."
#define SETTING_INOUT_TEMP              "\0\xB3\xBD/\xBD\x61\x70 \xBF\x65\xBC\xBE"
#endif

#ifdef DS3231_TEMP
// "ds3231 темп"
#define SETTING_DS3231_TEMP             "\0ds3231 \xBF\x65\xBC\xBE"
#endif

#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
// "звук серв."
#define SETTING_SERVICE_ALARM           "\0\xB7\xB3\x79\xBA \x63\x65\x70\xB3."
#endif

#ifdef SOUND_SUPPORT
// "звук кнопок"
#define SETTING_KEY_SOUND               "\0\xB7\xB3\x79\xBA \xBA\xBD\x6F\xBE\x6F\xBA"
#endif

#ifdef FUEL_TANK_SUPPORT
// ""
#define SETTING_ADC_FUEL_NORMALIZE      "\0adc fuel nor"
#endif

// "\0---\0нар\0вн\0  дв\0"
#define TEMP_SENSORS_ARRAY              "\0---\0\xBD\x61\x70\0\xB3\xBD\0  \xE3\xB3\0"

// "датч. не найдены"
#define TEMP_NO_SENSORS                 "\xE3\x61\xBF\xC0. \xBD\x65 \xBD\x61\xB9\xE3\x65\xBD\xC3"

// "просмотр журнала"
#define JOURNAL_VIEWER                  "\xBE\x70\x6F\x63\xBC\x6F\xBF\x70 \xB6\x79\x70\xBD\x61\xBB\x61"

// " текущий проб. проб. A проб. B изм. разгона"
#define JOURNAL_VIEWER_ITEMS_ARRAY      "\0\xBF\x65\xBA\x79\xE6\xB8\xB9 \xBE\x70\x6F\xB2.\0\xBE\x70\x6F\xB2. A\0\xBE\x70\x6F\xB2. B\0\xB8\xB7\xBC. \x70\x61\xB7\xB4\x6F\xBD\x61"

// "нет записей"
#define JOURNAL_VIEWER_NO_ITEMS         "\xBD\x65\xBF \xB7\x61\xBE\xB8\x63\x65\xB9"

// "конст. топл."
#define CONFIG_MENU_FUEL_CONSTANT       "\0\xBA\x6F\xBD\x63\xBF. \xBF\x6F\xBE\xBB."

// "конст. скор."
#define CONFIG_MENU_VSS_CONSTANT        "\0\xBA\x6F\xBD\x63\xBF. \x63\xBA\x6F\x70."

// "общий пробег"
#define CONFIG_MENU_TOTAL_TRIP          "\0\x6F\xB2\xE6\xB8\xB9 \xBE\x70\x6F\xB2\x65\xB4"

// "настр. напряж."
#define CONFIG_MENU_VOLTAGE_ADJUST      "\0\xBD\x61\x63\xBF\x70. \xBD\x61\xBE\x70\xC7\xB6."

// "биты настроек"
#define CONFIG_MENU_SETTINGS_BITS       "\0\xB2\xB8\xBF\xC3 \xBD\x61\x63\xBF\x70\x6F\x65\xBA"

#if defined(DS18B20_CONFIG)
// "темп. датчики"
#define CONFIG_MENU_TEMP_SENSORS        "\0\xBF\x65\xBC\xBE. \xE3\x61\xBF\xC0\xB8\xBA\xB8"
#endif

#if defined(SERVICE_COUNTERS_CHECKS_SUPPORT)
// "серв. интерв."
#define CONFIG_MENU_SERVICE_COUNTERS    "\0\x63\x65\x70\xB3. \xB8\xBD\xBF\x65\x70\xB3."
#endif

#if defined(MIN_SPEED_CONFIG)
// "мин. скорость"
#define CONFIG_MENU_MIN_SPEED           "\0\xBC\xB8\xBD. \x63\xBA\x6F\x70\x6F\x63\xBF\xC4"
#endif

// "версия по"
#define CONFIG_MENU_VERSION_INFO        "\0\xB3\x65\x70\x63\xB8\xC7 \xBE\x6F"

// " воскресенье понедельник вторник среда четверг пятница суббота"
#define DAY_OF_WEEK_ARRAY               "\x00\xB3\x6F\x63\xBA\x70\x65\x63\x65\xBD\xC4\x65\x00\xBE\x6F\xBD\x65\xE3\x65\xBB\xC4\xBD\xB8\xBA\x00\xB3\xBF\x6F\x70\xBD\xB8\xBA\x00\x63\x70\x65\xE3\x61\x00\xC0\x65\xBF\xB3\x65\x70\xB4\x00\xBE\xC7\xBF\xBD\xB8\xE5\x61\x00\x63\x79\xB2\xB2\x6F\xBF\x61"

// " янв фев мар апр май июн июл авг сен окт ноя дек"
#define MONTH_ARRAY                     "\x00\xC7\xBD\xB3\x00\xE4\x65\xB3\x00\xBC\x61\x70\x00\x61\xBE\x70\x00\xBC\x61\xB9\x00\xB8\xC6\xBD\x00\xB8\xC6\xBB\x00\x61\xB3\xB4\x00\x63\x65\xBD\x00\x6F\xBA\xBF\x00\xBD\x6F\xC7\x00\xE3\x65\xBA"

// "т.датчик"
#define TEMP_SENSOR                     "\xBF.\xE3\x61\xBF\xC0\xB8\xBA"

#endif /* LOCALE_RUSSIAN */

#ifndef CONFIG_MENU_TEMP_SENSORS
#define CONFIG_MENU_TEMP_SENSORS
#endif

#ifndef CONFIG_MENU_SERVICE_COUNTERS
#define CONFIG_MENU_SERVICE_COUNTERS
#endif

#ifndef CONFIG_MENU_MIN_SPEED
#define CONFIG_MENU_MIN_SPEED
#endif

#define CONFIG_MENU_ARRAY                \
            CONFIG_MENU_FUEL_CONSTANT    \
            CONFIG_MENU_VSS_CONSTANT     \
            CONFIG_MENU_TOTAL_TRIP       \
            CONFIG_MENU_VOLTAGE_ADJUST   \
            CONFIG_MENU_SETTINGS_BITS    \
            CONFIG_MENU_TEMP_SENSORS     \
            CONFIG_MENU_SERVICE_COUNTERS \
            CONFIG_MENU_MIN_SPEED        \
            CONFIG_MENU_VERSION_INFO     \

#ifndef SETTING_ENCODER
#define SETTING_ENCODER                 "\0"
#endif

#ifndef SETTING_ADC_FUEL_NORMALIZE
#define SETTING_ADC_FUEL_NORMALIZE      "\0"
#endif

#ifndef SETTING_KEY_SOUND
#define SETTING_KEY_SOUND               "\0"
#endif

#ifndef SETTING_SERVICE_ALARM
#define SETTING_SERVICE_ALARM           "\0"
#endif

#ifndef SETTING_DS3231_TEMP
#define SETTING_DS3231_TEMP             "\0"
#endif

#ifndef SETTING_INOUT_TEMP
#define SETTING_INOUT_TEMP              "\0"
#endif

#ifndef SETTING_TRIPB_MONTH
#define SETTING_TRIPB_MONTH             "\0"
#endif

#define SETTING_DUMMY                   "\0"

#define SETTINGS_BITS_ARRAY             \
            SETTING_PAIRPAR_INJ         \
            SETTING_DUMMY               \
            SETTING_KEY_SOUND           \
            SETTING_SERVICE_ALARM       \
            SETTING_DUMMY               \
            SETTING_TRIPB_MONTH         \
            SETTING_TRIPC_DAY           \
            SETTING_INOUT_TEMP          \
            SETTING_DS3231_TEMP         \
            SETTING_ADC_FUEL_NORMALIZE  \
            "\0"                        \
            "\0"                        \
            "\0"                        \
            "\0"                        \
            "\0"                        \
            SETTING_ENCODER             \

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

#endif	/* LOCALE_H */
