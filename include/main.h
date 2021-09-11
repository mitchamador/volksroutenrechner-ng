#ifndef MAIN_H
#define	MAIN_H

#include "hw.h"
#include "lcd.h"

typedef struct {
    uint8_t day;
    uint8_t month;
    uint8_t year;
} service_time_t;

typedef struct {
    uint16_t counter;
    uint8_t limit;
    service_time_t time;
    uint8_t dummy[2]; // fill to 8 bytes size
} srv_t;

typedef struct {
    uint32_t counter;
    uint32_t counter_rpm;
    uint16_t limit;
} srv_mh_t;

typedef union {
    // a structure with 8 single bit bit-field objects, overlapping the union member "byte"
    uint8_t byte;

    struct {
        unsigned b0 : 1;
        unsigned alt_buttons : 1;
        unsigned mh_rpm : 1;
        unsigned fast_refresh : 1;
        unsigned service_alarm : 1;
        unsigned key_sound : 1;
        unsigned skip_temp_screen : 1;
        unsigned dual_injection : 1;
    };
} settings_u;

typedef struct {
    uint16_t odo;
    uint16_t odo_temp;
    uint8_t fuel_tmp1, fuel_tmp2;
    uint16_t fuel;
    uint32_t time;
} trip_t;

typedef struct {
    uint8_t minute, hour, day, month, year;
} trip_time_t;

// service counters limits

typedef struct {
    // main odometer
    uint32_t odo;
    uint16_t odo_temp;

    uint16_t odo_const;
    uint8_t fuel_const;
    uint8_t vcc_const;

    settings_u settings;

    // param counter for main screen
    uint8_t selected_param1;
    // param counter for tripC screen
    uint8_t selected_param2;

    // min speed for drive mode
    uint8_t min_speed;

    uint8_t dummy[2];

} config_t;

typedef struct {
    srv_mh_t mh;
    srv_t srv[4];
} services_t;

typedef struct {
    trip_t tripA, tripB, tripC;
    trip_time_t tripC_time;
    uint16_t tripC_max_speed;
} trips_t;

#define EEPROM_DS18B20_ADDRESS (((sizeof(config_t) - 1) / 8 + 1) * 8) + (((sizeof(trips_t) - 1) / 8 + 1) * 8) + (((sizeof(services_t) - 1) / 8 + 1) * 8)
#define EEPROM_CUSTOM_CHARS_ADDRESS (EEPROM_DS18B20_ADDRESS + 8 * 3)

typedef void (*screen_func) (void);

typedef struct {
    screen_func screen;
} screen_item_t;

typedef struct service_screen_item_t service_screen_item_t;
typedef void (*service_screen_func) (service_screen_item_t *);

struct service_screen_item_t {
    uint8_t str_index;
    service_screen_func screen;
};

#endif	/* MAIN_H */

