#include "ds18b20.h"

#ifdef DS18B20_SUPPORT

__bit ds18b20_start_conversion() {
    if (!onewire_start()) // send start pulse
        return 0; // return 0 if error

    onewire_write_byte(0xCC); // send skip ROM command
    onewire_write_byte(0x44); // send start conversion command

    return 1;
}

__bit ds18b20_read_rom(unsigned char *buf) {
    if (!onewire_start()) // send start pulse
        return 0; // return 0 if error

    onewire_write_byte(0x33); // send read ROM command
    unsigned char i;
    for (i = 0; i < 8; i++) {
        *buf++ = onewire_read_byte();
    }

    return 1; // OK --> return 1
}

__bit ds18b20_read_temp_matchrom(unsigned char *tbuf, uint16_t *raw_temp_value) {
    if (!onewire_start()) // send start pulse
        return 0; // return 0 if error

    onewire_write_byte(0x55); // send match ROM command

    unsigned char *ptr = tbuf;

    unsigned char i = 0;
    for (i = 0; i < 8; i++) {
        onewire_write_byte(*ptr++); // send address
    }

    onewire_write_byte(0xBE); // send read command

    ptr = tbuf;
    for (int i = 0; i < 9; i++) {
        *ptr++ = onewire_read_byte();
    }

    *raw_temp_value = (((uint16_t) (tbuf[1] << 8)) | tbuf[0]);

    if (tbuf[8] != ds18b20_crc(tbuf, 8)) {
        return 0; // ERROR --> return 0
    }

    return 1; // OK --> return 1
}

uint8_t ds18b20_crc(const uint8_t *addr, uint8_t len) {
    uint8_t crc = 0;
    while (len--) {
        uint8_t inbyte = *addr++;
        for (uint8_t i = 8; i; i--) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}

#endif