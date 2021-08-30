#ifndef DS1307_H
#define	DS1307_H

#include "hw.h"

typedef struct {
    uint8_t minute,hour,day_of_week,day,month,year;
} ds_time;

void get_ds_time(ds_time*);
void set_ds_time(ds_time*);

#endif	/* DS1307_H */

