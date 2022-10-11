#include "ds18b20.h"
#include "utils.h"

#ifdef DS18B20_TEMP

flag_t ds18b20_start_conversion() {
    if (!onewire_start()) // send start pulse
        return 0; // return 0 if error

    onewire_write_byte(0xCC); // send skip ROM command
    onewire_write_byte(0x44); // send start conversion command

    return 1;
}

flag_t ds18b20_read_rom(unsigned char *buf) {
    if (!onewire_start()) // send start pulse
        return 0; // return 0 if error

    onewire_write_byte(0x33); // send read ROM command
    unsigned char i;
    for (i = 0; i < 8; i++) {
        *buf++ = onewire_read_byte();
    }

    return 1; // OK --> return 1
}


flag_t ds18b20_read_temp_matchrom(unsigned char *tbuf, uint16_t *raw_temp_value) {
    unsigned char rom_buf[9];

    if (!onewire_start()) // send start pulse
        return 0; // return 0 if error

    onewire_write_byte(0x55); // send match ROM command

    unsigned char *ptr = tbuf;

    unsigned char i = 0;
    for (i = 0; i < 8; i++) {
        onewire_write_byte(*ptr++); // send address
    }

    onewire_write_byte(0xBE); // send read command

    ptr = rom_buf;
    for (int i = 0; i < 9; i++) {
        *ptr++ = onewire_read_byte();
    }

    *raw_temp_value = (((uint16_t) (rom_buf[1] << 8)) | rom_buf[0]);

    if (rom_buf[8] != crc8_dallas(rom_buf, 8)) {
        return 0; // ERROR --> return 0
    }

    return 1; // OK --> return 1
}

#endif