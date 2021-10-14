#include "ds1307.h"
#include "utils.h"
#include "i2c.h"
#include "version.h"

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
        // fallback time
        time->minute = VERSION_MINUTE_BCD;
        time->hour = VERSION_HOUR_BCD;
        time->day_of_week = VERSION_DAY_OF_WEEK_BCD;
        time->day = VERSION_DAY_OF_MONTH_BCD;
        time->month = VERSION_MONTH_BCD;
        time->year = VERSION_YEAR_BCD;
        set_ds_time(time);
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

void set_day_of_week(ds_time* time) {
    uint8_t dow;
    uint8_t mArr[12] = {6, 2, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

    uint8_t tYear = bcd8_to_bin(time->year);
    uint8_t tMonth = bcd8_to_bin(time->month);
    dow = tYear;
    dow += tYear / 4;
    dow += bcd8_to_bin(time->day);
    dow += mArr[tMonth - 1];
    if (((tYear % 4) == 0) && (tMonth < 3))
        dow -= 1;
    while (dow >= 7)
        dow -= 7;
    time->day_of_week = dow + 1;
}

