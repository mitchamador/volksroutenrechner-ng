#include "onewire.h"
#include "utils.h"
#include <string.h>

void onewire_write_bit(uint8_t value) {
    value = value & 0x01;
    if (value) {
        // Write '1' bit
        ONEWIRE_OUTPUT;
        ONEWIRE_CLEAR;
        delay_us(5);
        ONEWIRE_INPUT;
        delay_us(60);
    } else {
        // Write '0' bit
        ONEWIRE_OUTPUT;
        ONEWIRE_CLEAR;
        delay_us(70);
        ONEWIRE_INPUT;
        delay_us(2);
    }
}

__bit onewire_read_bit(void) {
    unsigned char result;

    ONEWIRE_OUTPUT;
    ONEWIRE_CLEAR;
    delay_us(1);
    ONEWIRE_INPUT;
    delay_us(5);
    result = ONEWIRE_GET;
    delay_us(55);
    return (__bit) result;

}

__bit onewire_start() {
    unsigned char result;

    ONEWIRE_OUTPUT;
    ONEWIRE_CLEAR;
    delay_us(480);
    ONEWIRE_INPUT;
    delay_us(70);
    result = !ONEWIRE_GET;
    delay_us(410);
    return (__bit) result;
}

uint8_t onewire_read_byte(void) {
    unsigned char result = 0;

    for (unsigned char loop = 0; loop < 8; loop++) {
        // shift the result to get it ready for the next bit
        result >>= 1;

        // if result is one, then set MS bit
        if (onewire_read_bit())
            result |= 0x80;
    }
    return result;
}

void onewire_write_byte(uint8_t value) {
    // Loop to write each bit in the byte, LS-bit first
    for (unsigned char loop = 0; loop < 8; loop++) {
        onewire_write_bit(value & 0x01);

        // shift the data byte for the next bit
        value >>= 1;
    }
}

uint8_t last_discrepancy = 0;
__bit last_device_fl = 0;
uint8_t _rom[8] = {0,0,0,0,0,0,0,0};

__bit onewire_search() {

    uint8_t id_bit_number;
    uint8_t last_zero;
    static __bit search_result;
    uint8_t id_bit, cmp_id_bit;

    uint8_t rom_byte_mask, search_direction;
    
    // initialize for search
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_mask = 1;
    search_result = 0;
    
    uint8_t *buf = (uint8_t *) _rom;

    // if the last call was not the last one
    if (!last_device_fl) {
        // 1-Wire reset
        if (!onewire_start()) {
            // reset the search
            last_discrepancy = 0;
            last_device_fl = 0;
            return 0;
        }

        // issue the search command
        onewire_write_byte(0xF0); // NORMAL SEARCH

        uint8_t _b = 64;
        // loop to do the search
        while (_b-- > 0) {
            // read a bit and its complement
            id_bit = onewire_read_bit();
            cmp_id_bit = onewire_read_bit();

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1)) {
                break;
            } else {
                // all devices coupled have 0 or 1
                if (id_bit != cmp_id_bit) {
                    search_direction = id_bit; // bit write value for search
                } else {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < last_discrepancy) {
                        search_direction = ((*buf & rom_byte_mask) > 0);
                    } else {
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == last_discrepancy);
                    }
                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0) {
                        last_zero = id_bit_number;
                    }
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if (search_direction == 1)
                    *buf |= rom_byte_mask;
                else
                    *buf &= ~rom_byte_mask;

                // serial number search direction write bit
                onewire_write_bit(search_direction);

                // increment the byte counter id_bit_number
                // and shift the mask rom_byte_mask
                id_bit_number++;
                rom_byte_mask <<= 1;

                // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                if (rom_byte_mask == 0) {
                    buf++;
                    rom_byte_mask = 1;
                }
            }
        } // loop until through all ROM bytes 0-7

        // if the search was successful then
        if (id_bit_number > 64) {
            // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
            last_discrepancy = last_zero;

            // check for last device
            if (last_discrepancy == 0) {
                last_device_fl = 1;
            }
            search_result = 1;
        }
    }

    if (search_result == 0) {
        // reset the search
        last_discrepancy = 0;
        last_device_fl = 0;
        _memset(_rom, 0x00, 8);
    }

    return search_result;
}

uint8_t onewire_search_devices(uint8_t *ptr, int8_t max_devices) {
    uint8_t num_devices = 0;
    while (num_devices < max_devices && onewire_search() != 0) {
        uint8_t *_rom_ptr = (uint8_t *) _rom;
        uint8_t c = 8;
        while (c-- != 0) {
            *ptr++ = *_rom_ptr++;
        }
        num_devices++;
    }
    return num_devices;
}

