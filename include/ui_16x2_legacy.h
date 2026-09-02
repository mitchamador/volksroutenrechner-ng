#ifndef UI_16X2_LEGACY_H
#define	UI_16X2_LEGACY_H

// Legacy 16x2 UI module: menus, screens, editors, and the whole size-optimized
// print_*/lcd_print_* family originally written for the pic16f876a. This is a
// straight extraction of the old main.c screen code -- content is unchanged,
// only the file boundary is new. Do not "clean up" definitions here without
// checking every #ifdef target still compiles, especially pic16f876a.
//
// Public surface used from outside this module:
//   - ui_16x2_legacy_update()             called once per main-loop iteration
//   - clear_trip()                        called from core.c's fill_misc_values/power_on
//   - print_warning_service_counters()     called from main()'s init sequence
//   - params / service_param               read/written by core.c's
//                                           set_params() and power_off() to
//                                           persist/restore UI navigation state

#include "core.h"

typedef union {
    uint8_t byte;
    struct {
        unsigned trip : 4;
        unsigned config_switch : 1;
        unsigned skip_key_handler : 1;
        unsigned drive_mode : 1;
        unsigned dummy : 1;
    };
} main_page_t;

typedef struct {
    void (*screen)(void);
    main_page_t page;
} screen_item_t;

typedef struct {
    const char *title;
    uint8_t index;
} config_page_t;

typedef struct {
    void (*screen)(void);
    config_page_t page;
} screen_config_item_t;

// ds18b20 temperatures
#define TEMP_NONE -1
#define TEMP_OUT 0
#define TEMP_IN 1
#define TEMP_ENGINE 2
#define TEMP_CONFIG 3

#define PRINT_TEMP_PARAM_HEADER       0x80
#define PRINT_TEMP_PARAM_FRACT        0x40
#define PRINT_TEMP_PARAM_NO_PLUS_SIGN 0x20
#define PRINT_TEMP_PARAM_DEG_SIGN     0x10
#define PRINT_TEMP_PARAM_MASK         0x0F

typedef struct {
    uint8_t *p;         // value pointer
    uint8_t min;        // min value
    uint8_t max;        // max value
    uint8_t pos;        // cursor position
} time_editor_item_t;

// UI navigation state (which sub-page/parameter is currently shown). This is
// UI-owned, but core.c's set_params()/power_off() read and write it directly
// to restore/persist it in config EEPROM -- see the comment above set_params()
// in core.c for why this crosses the module boundary.
typedef struct {
    uint8_t main;
    uint8_t main_add;
    uint8_t tmp;        // cleared on screen item change, used also as additional page index
} params_t;

extern params_t params;

#ifdef SERVICE_COUNTERS_SUPPORT
extern uint8_t service_param;
#endif

// Called once per main-loop iteration from main(). Contains the menu
// navigation/dispatch/refresh-timing logic that used to be the tail half of
// main()'s while(1) body.
void ui_16x2_legacy_update(void);

// Displays the service-counter warning on the LCD at startup. The check
// itself (check_service_counters()) is core logic and lives in core.c;
// this half just renders the result.
void print_warning_service_counters(uint8_t warn);

#endif	/* UI_16X2_LEGACY_H */
