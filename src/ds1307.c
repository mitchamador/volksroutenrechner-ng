#include "ds1307.h"
#include "i2c.h"
#include "version.h"

void get_ds_time(ds_time* time) {
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x00); // start reading from 0x01 - minutes
    I2C_Master_RepeatedStart();
    I2C_Master_Write(0xD1);
    unsigned char seconds = I2C_Read_Byte_ACK();
    time->minute = I2C_Read_Byte_ACK();
    time->hour = I2C_Read_Byte_ACK();
    time->day_of_week = I2C_Read_Byte_ACK();
    time->day = I2C_Read_Byte_ACK();
    time->month = I2C_Read_Byte_ACK();
    time->year = I2C_Read_Byte_NACK();
    I2C_Master_Stop();
    if ((seconds & 0x80) != 0) {
        I2C_Master_Start();
        I2C_Master_Write(0xD0);
        I2C_Master_Write(0x00);
        I2C_Master_Write(0x00);
        I2C_Master_Write(VERSION_MINUTE_BCD);
        I2C_Master_Write(VERSION_HOUR_BCD);
        I2C_Master_Write(VERSION_DAY_OF_WEEK_BCD);
        I2C_Master_Write(VERSION_DAY_OF_MONTH_BCD);
        I2C_Master_Write(VERSION_MONTH_BCD);
        I2C_Master_Write(VERSION_YEAR_BCD);
        I2C_Master_Stop();
    }
}

void set_ds_time(ds_time* time) {
    I2C_Master_Start();
	I2C_Master_Write(0xD0);
	I2C_Master_Write(0x00);
	I2C_Master_Write(0); // seconds
	I2C_Master_Write(time->minute);
	I2C_Master_Write(time->hour);
	I2C_Master_Write(time->day_of_week);
	I2C_Master_Write(time->day);
	I2C_Master_Write(time->month);
	I2C_Master_Write(time->year);
    I2C_Master_Stop();
}

