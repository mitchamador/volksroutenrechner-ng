/* 
 * File:   ds1307.h
 * Author: victor
 *
 * Created on 30 мая 2021 г., 13:55
 */

#ifndef DS1307_H
#define	DS1307_H

#include "hw.h"
#include "utils.h"

typedef struct {
    unsigned char minute,hour,day_of_week,day,month,year;
} ds_time;

void get_ds_time(ds_time*);
void set_ds_time(ds_time*);

#endif	/* DS1307_H */

