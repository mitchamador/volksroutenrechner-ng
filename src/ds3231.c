#include "ds3231.h"
#include "ds18b20.h"
#include "version.h"

// time
#if defined(_16F876A)
__bank2 ds_time time;
#else
ds_time time;
#endif

void DS3231_time_read(ds_time* time) {
    unsigned char seconds = 0x80;
    if (I2C_Master_Start(0xD0) == ACK) {
        I2C_Master_Write(0x00); // start reading from 0x00 - seconds
        I2C_Master_RepeatedStart(0xD1);
        seconds = I2C_Read_Byte(ACK);
        time->minute = I2C_Read_Byte(ACK);
        time->hour = I2C_Read_Byte(ACK);
        time->day_of_week = I2C_Read_Byte(ACK);
        time->day = I2C_Read_Byte(ACK);
        time->month = I2C_Read_Byte(ACK);
        time->year = I2C_Read_Byte(NACK);
        time->flags.is_valid = 1;
    }
    I2C_Master_Stop();
    if ((seconds & 0x80) != 0) {
        // fallback time
        time->minute = VERSION_MINUTE_BCD;
        time->hour = VERSION_HOUR_BCD;
        time->day_of_week = VERSION_DAY_OF_WEEK_BCD;
        time->day = VERSION_DAY_OF_MONTH_BCD;
        time->month = VERSION_MONTH_BCD;
        time->year = VERSION_YEAR_BCD;
        time->flags.is_valid = 0;
        DS3231_time_write(time);
    }
}

void DS3231_time_write(ds_time* time) {
	if (I2C_Master_Start(0xD0) == ACK) {
        I2C_Master_Write(0x00);
        I2C_Master_Write(0x00); // seconds
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
        if (msb != 0xFF) {
            *raw_temp_value = (((uint16_t) (msb << 4)) | (lsb >> 4));
        } else {
            *raw_temp_value = DS18B20_TEMP_NONE;
        }
    }
    I2C_Master_Stop();
}

void DS3231_temp_start() {
    if (I2C_Master_Start(0xD0) == ACK) {
        I2C_Master_Write(DS3231_REG_CTRL);                          // 0x0E - control register
        I2C_Master_Write(DS3231_CTRL_DEFAULT | DS3231_CTRL_CONV);   // start conversion
    }
    I2C_Master_Stop();
}
#endif
