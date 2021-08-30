#include "ds1307.h"
#include "i2c.h"
#include "version.h"

// fallback time
ds_time _time = {
    VERSION_MINUTE_BCD,
    VERSION_HOUR_BCD,
    VERSION_DAY_OF_WEEK_BCD,
    VERSION_DAY_OF_MONTH_BCD,
    VERSION_MONTH_BCD,
    VERSION_YEAR_BCD
};

void get_ds_time(ds_time* time) {
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x00); // start reading from 0x00 - seconds
    I2C_Master_RepeatedStart();
    I2C_Master_Write(0xD1);
    unsigned char seconds = I2C_Read_Byte(ACK);
    time->minute = I2C_Read_Byte(ACK);
    time->hour = I2C_Read_Byte(ACK);
    time->day_of_week = I2C_Read_Byte(ACK);
    time->day = I2C_Read_Byte(ACK);
    time->month = I2C_Read_Byte(ACK);
    time->year = I2C_Read_Byte(NACK);
    I2C_Master_Stop();
    if ((seconds & 0x80) != 0) {
        set_ds_time(&_time);
    }
}

void set_ds_time(ds_time* time) {
    I2C_Master_Start();
	I2C_Master_Write(0xD0);
	I2C_Master_Write(0x00);
	I2C_Master_Write(0x00); // seconds
	I2C_Master_Write(time->minute);
	I2C_Master_Write(time->hour);
	I2C_Master_Write(time->day_of_week);
	I2C_Master_Write(time->day);
	I2C_Master_Write(time->month);
	I2C_Master_Write(time->year);
    I2C_Master_Stop();
}

