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

unsigned char get_day_of_week_text(char*, unsigned char);
char* get_month_text(unsigned char);

#endif	/* DS1307_H */

