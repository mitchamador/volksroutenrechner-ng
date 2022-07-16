#include "i2c-eeprom.h"
#include "string.h"

#if defined(JOURNAL_SUPPORT) && !defined(JOURNAL_EEPROM_INTERNAL)

void I2C_read_eeprom_block(unsigned char* p, uint16_t ee_addr, unsigned char length) {
    uint8_t wait_ms = 0;
    uint8_t ack = NACK;
    while (ack == NACK && wait_ms < WAIT_RESPONSE_MS) {
        I2C_Master_Start();
        ack = I2C_Master_Write(0xA0 | ((ee_addr >> 7) & 0x0E));
        if (ack == NACK) {
            delay_ms(1);
            wait_ms++;
        }
    }
    if (ack == ACK) {
        I2C_Master_Write(ee_addr & 0xFF);
        I2C_Master_RepeatedStart();
        I2C_Master_Write(0xA1 | ((ee_addr >> 7) & 0x0E));
        while (--length > 0) {
            *p++ = I2C_Read_Byte(ACK);
        }
        *p = I2C_Read_Byte(NACK);
    } else {
        _memset(p, 0xFF, length);
    }
    I2C_Master_Stop();
}

void I2C_write_eeprom_block(unsigned char* p, uint16_t ee_addr, unsigned char length) {
    do {
        uint8_t wait_ms = 0;
        uint8_t ack = NACK;
        while (ack == NACK && wait_ms < WAIT_RESPONSE_MS) {
            I2C_Master_Start();
            ack = I2C_Master_Write(0xA0 | ((ee_addr >> 7) & 0x0E));
            if (ack == NACK) {
                delay_ms(1);
                wait_ms++;
            }
        }
        if (ack == ACK) {
            I2C_Master_Write(ee_addr & 0xFF);
            while (length > 0) {
                I2C_Master_Write(*p++);
                length--;
                ee_addr++;
                if ((ee_addr & 0x0F) == 0) break;
            }
        } else {
            length = 0;
        }
        I2C_Master_Stop();
    } while (length > 0);
}

#endif
