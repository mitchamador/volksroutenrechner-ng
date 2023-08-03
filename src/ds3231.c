#include "ds3231.h"
#include "core.h"
#include "utils.h"
#include "ds18b20.h"
#include "version.h"

// time
#if defined(_16F876A)
__bank2 ds_time time;
#else
ds_time time;
#endif

void DS3231_write_reg(unsigned char reg, unsigned char data) {
    if (I2C_Master_Start(0xD0) == ACK) {
        I2C_Master_Write(reg);
        I2C_Master_Write(data);
    }
    I2C_Master_Stop();
}

void DS3231_time_read(ds_time* time) {
    time->flags.is_valid = 0;

    if (I2C_Master_Start(0xD0) == ACK) {
        I2C_Master_Write(0x00); // start reading from 0x00 - seconds
        I2C_Master_RepeatedStart(0xD1);
        time->flags.is_valid = (I2C_Read_Byte(ACK) & 0x80) == 0; // CH for DS1307
        time->minute = I2C_Read_Byte(ACK);
        time->hour = I2C_Read_Byte(ACK);
        time->day_of_week = I2C_Read_Byte(ACK);
        time->day = I2C_Read_Byte(ACK);
        time->month = I2C_Read_Byte(ACK);
        time->year = I2C_Read_Byte(NACK);
    }
    I2C_Master_Stop();

    if (time->flags.is_valid == 0 || time->year == 0) {
        // fallback time
#if defined(FALLBACK_TIME_VERSION)
        time->minute = VERSION_MINUTE_BCD;
        time->hour = VERSION_HOUR_BCD;
        time->day_of_week = VERSION_DAY_OF_WEEK_BCD;
        time->day = VERSION_DAY_OF_MONTH_BCD;
        time->month = VERSION_MONTH_BCD;
        time->year = VERSION_YEAR_BCD;
#else
        time->minute = trips.tripC_time.minute;
        time->hour = trips.tripC_time.hour;
        time->day = trips.tripC_time.day;
        time->month = trips.tripC_time.month;
        time->year = trips.tripC_time.year;
        time->day_of_week = trips.tripC_time_dow;
#endif

        DS3231_time_write(time);
#if !defined(_16F876A)
        DS3231_write_reg(DS3231_REG_STATUS, 0); // clears OSF for DS3231
#endif
    }
}

void DS3231_time_write(ds_time* time) {
	if (I2C_Master_Start(0xD0) == ACK) {
        I2C_Master_Write(0x00); // start writing from 0x00 - seconds (also clears CH for DS1307)
        I2C_Master_Write(0);
        I2C_Master_Write(time->minute);
        I2C_Master_Write(time->hour);
        I2C_Master_Write(time->day_of_week);
        I2C_Master_Write(time->day);
        I2C_Master_Write(time->month);
        I2C_Master_Write(time->year);
    }
    I2C_Master_Stop();
}

#ifdef DS3231_TEMP
void DS3231_temp_read(uint16_t* raw_temp_value) {
    if (I2C_Master_Start(0xD0) == ACK) {
        I2C_Master_Write(DS3231_REG_TEMP); // start reading from 0x11 - temp
        I2C_Master_RepeatedStart(0xD1);
        uint8_t msb = I2C_Read_Byte(ACK);
        uint8_t lsb = I2C_Read_Byte(NACK);
        if ((lsb & ~0xC0) != 0) {
            *raw_temp_value = DS18B20_TEMP_NONE;
        } else {
            int16_t _raw = msb << 8 | lsb;
            *raw_temp_value = (uint16_t) (_raw >> 4);
        }
    }
    I2C_Master_Stop();
}

#endif
