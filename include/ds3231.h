#ifndef DS1307_H
#define	DS1307_H

#include "hw.h"
#include <stdint.h>

#define DS3231_REG_CTRL     0x0E
#define DS3231_REG_STATUS   0x0F
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

// time
#if defined(_16F876A)
__bank2 extern ds_time time;
#else
extern ds_time time;
#endif

void DS3231_write_reg(uint8_t reg, uint8_t data);
void DS3231_time_read(ds_time*);
void DS3231_time_write(ds_time*);
void DS3231_temp_read(uint16_t*);

#define DS3231_temp_start() DS3231_write_reg(DS3231_REG_CTRL, DS3231_CTRL_DEFAULT | DS3231_CTRL_CONV)

#endif	/* DS1307_H */

