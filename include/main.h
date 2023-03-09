#ifndef MAIN_H
#define	MAIN_H

#include "core.h"


typedef union {
    uint8_t byte;
    struct {
        unsigned index : 4;
        unsigned config_switch : 1;
        unsigned skip_key_handler : 1;
        unsigned drive_mode : 1;
        unsigned skip : 1;
    };
} main_page_t;

typedef struct {
    void (*screen)(void);
    main_page_t page;
} screen_item_t;

typedef union {
    uint8_t byte;

    struct {
        unsigned title_string_index : 4;
        unsigned index : 4;
    };
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

#endif	/* MAIN_H */
