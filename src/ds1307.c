#include "ds1307.h"
#include "i2c.h"

void get_ds_time(ds_time* time) {
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Write(0x00); // start reading from 0x01 - minutes
    I2C_Master_RepeatedStart();
    I2C_Master_Write(0xD1);
    unsigned char seconds = I2C_Read_Byte();
    if ((seconds & 0x80) == 0) {
        I2C_ACK();
        time->minute = I2C_Read_Byte();
        I2C_ACK();
        time->hour = I2C_Read_Byte();
        I2C_ACK();
        time->day_of_week = I2C_Read_Byte();
        I2C_ACK();
        time->day = I2C_Read_Byte();
        I2C_ACK();
        time->month = I2C_Read_Byte();
        I2C_ACK();
        time->year = I2C_Read_Byte();
    }
    I2C_NACK();
    I2C_Master_Stop();
    if ((seconds & 0x80) != 0) {
        I2C_Master_Start();
        I2C_Master_Write(0xD0);
        I2C_Master_Write(0x00);
        I2C_Master_Write(0);
        I2C_Master_Write(0);
        I2C_Master_Write(12);
        I2C_Master_Write(3);
        I2C_Master_Write(1);
        I2C_Master_Write(6);
        I2C_Master_Write(21);
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

