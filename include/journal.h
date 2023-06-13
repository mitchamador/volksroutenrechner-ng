#ifndef JOURNAL_H
#define	JOURNAL_H

#include "core.h"
#include "i2c-eeprom.h"

// 8 symbols
#define JOURNAL_MARK                    "JOURTRIP"

typedef struct {
    uint8_t item_current;
    uint8_t item_max;
    uint8_t item_num;
    uint8_t item_prev;
} journal_reader_t;

typedef struct {
    uint8_t status; // 1 byte
    trip_time_t start_time; // 5 byte
    union { // 12 bytes
        trip_t trip; 
        print_trip_t ptrip;
    };
} journal_trip_item_t; // 18 bytes total

typedef struct {
    uint8_t status; // 1 byte
    trip_time_t start_time; // 5 byte
    uint8_t lower; // 1 byte
    uint8_t upper; // 1 byte
    uint16_t time; // 2 byte
} journal_accel_item_t; // 10 bytes total

typedef struct {
    uint8_t current;
    uint8_t max;
} journal_type_pos_t;

typedef union {
    uint8_t byte;

    struct {
        uint8_t dummy : 8;
    };
} journal_header_flags_u;

typedef struct {
    journal_type_pos_t journal_type_pos[4]; // 8
    trip_time_t time_trip_c; // 5
    trip_time_t time_trip_a; // 5
    trip_time_t time_trip_b; // 5
    journal_header_flags_u flags; // 1
} journal_header_t; // 24

#ifdef JOURNAL_EEPROM_INTERNAL

// internal eeprom for atmega328p (768 bytes)

#define JOURNAL_read_eeprom_block(p, ee_addr, length) HW_read_eeprom_block(p, ee_addr, length);
#define JOURNAL_write_eeprom_block(p, ee_addr, length) HW_write_eeprom_block(p, ee_addr, length);

#define J_EEPROM_START 256
#define J_EEPROM_LENGTH 768

// 768 - 32 = 736 bytes for data

// (20 + 12 + 6) * 18 + 5 * 10 = 734
#define J_EEPROM_TRIPC_COUNT 20
#define J_EEPROM_TRIPA_COUNT 12
#define J_EEPROM_TRIPB_COUNT 6
#define J_EEPROM_ACCEL_COUNT 5

#else

// i2c eeprom 24lc16 ((2048-32) / 16 bytes per block = 126 data blocks)

#define JOURNAL_read_eeprom_block(p, ee_addr, length) I2C_read_eeprom_block(p, ee_addr, length);
#define JOURNAL_write_eeprom_block(p, ee_addr, length) I2C_write_eeprom_block(p, ee_addr, length);

#define J_EEPROM_START 0
#define J_EEPROM_LENGTH 2048

// 2048 - 32 = 2016 bytes for data

// (64 + 30 + 12) * 18 + 10 * 10 = 2008
#define J_EEPROM_TRIPC_COUNT 64
#define J_EEPROM_TRIPA_COUNT 30
#define J_EEPROM_TRIPB_COUNT 12
#define J_EEPROM_ACCEL_COUNT 10

#endif

#define J_EEPROM_MARK_POS J_EEPROM_START
#define J_EEPROM_DATA J_EEPROM_START + 32

#define JOURNAL_ITEM_V1 0xA5
#define JOURNAL_ITEM_V2 0xAA

extern flag_t journal_support;

extern journal_header_t journal_header;

unsigned char* journal_read_item(journal_reader_t* jr, uint8_t journal_type);
void journal_update_header(void);
flag_t journal_check_eeprom(void);
uint16_t journal_find_eeaddr(uint8_t index, int8_t item_index);
void journal_save_trip(trip_t *trip);
void journal_save_accel(uint8_t index);


#endif	/* JOURNAL_H */

