#ifndef DS1307_H
#define	DS1307_H

#include "hw.h"

#define DS3231_REG_CTRL     0x0E
#define DS3231_REG_TEMP     0x11

#define DS3231_CTRL_INTCN   (1 << 2)
#define DS3231_CTRL_RS1     (1 << 3)
#define DS3231_CTRL_RS2     (1 << 4)
#define DS3231_CTRL_CONV    (1 << 5)

#define DS3231_CTRL_DEFAULT (DS3231_CTRL_INTCN | DS3231_CTRL_RS1 | DS3231_CTRL_RS2)

typedef union {
    uint8_t byte;

    struct {
        uint8_t dummy : 7;
        uint8_t is_valid : 1;
    };

} time_flags_u;

typedef struct {
    uint8_t minute,hour,day_of_week,day,month,year;
    time_flags_u flags;
} ds_time;

void get_ds_time(ds_time*);
void set_ds_time(ds_time*);
void get_ds_temp(uint16_t*);
void start_ds_temp(void);

#endif	/* DS1307_H */

