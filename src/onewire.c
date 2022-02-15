#include "onewire.h"

#if 0
__bit onewire_start() {
    ONEWIRE_CLEAR; // send reset pulse to the DS18B20 sensor
    ONEWIRE_OUTPUT; // configure DS18B20_PIN pin as output
    delay_us(500); // wait 500 us

    ONEWIRE_INPUT; // configure DS18B20_PIN pin as input
    delay_us(100); // wait 100 us to read the DS18B20 sensor response

    if (ONEWIRE_GET == 0) {
        delay_us(400); // wait 400 us
        return 1; // DS18B20 sensor is present
    }

    return 0; // connection error
}

void onewire_write_bit(uint8_t value) {
    ONEWIRE_CLEAR;
    ONEWIRE_OUTPUT; // configure DS18B20_PIN pin as output
    delay_us(2); // wait 2 us

    ONEWIRE_VALUE((__bit) value);
    delay_us(80); // wait 80 us

    ONEWIRE_INPUT; // configure DS18B20_PIN pin as input
    delay_us(2); // wait 2 us
}

__bit onewire_read_bit(void) {
    static __bit value;

    ONEWIRE_CLEAR;
    ONEWIRE_OUTPUT; // configure DS18B20_PIN pin as output
    delay_us(2);

    ONEWIRE_INPUT; // configure DS18B20_PIN pin as input
    delay_us(5); // wait 5 us

    value = ONEWIRE_GET; // read and store DS18B20 state
    delay_us(100); // wait 100 us

    return value;
}

void onewire_write_byte(uint8_t value) {
    for (uint8_t i = 0; i < 8; i++)
        onewire_write_bit(value >> i);
}

uint8_t onewire_read_byte(void) {
    uint8_t value = 0;

    for (uint8_t i = 0; i < 8; i++)
        value |= onewire_read_bit() << i;

    return value;
}
#else

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
    ONEWIRE_OUTPUT;
    ONEWIRE_CLEAR;
    delay_us(480);
    ONEWIRE_INPUT;
    delay_us(60);
    if (ONEWIRE_GET == 0) {
        delay_us(100);
        return 1;
    }
    return 0;
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
#endif

