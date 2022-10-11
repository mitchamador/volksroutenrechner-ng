#include "journal.h"
#include "stdbool.h"
#include "string.h"

#if defined(JOURNAL_SUPPORT)

flag_t journal_support;

journal_header_t journal_header = {
    // journal_type_pos_t
    {
        {0xFF, J_EEPROM_TRIPC_COUNT},
        {0xFF, J_EEPROM_TRIPA_COUNT},
        {0xFF, J_EEPROM_TRIPB_COUNT},
        {0xFF, J_EEPROM_ACCEL_COUNT}
    },
};

const char journal_mark[] = JOURNAL_MARK;

// item buffer
unsigned char item[sizeof (journal_trip_item_t) >= sizeof (journal_accel_item_t) ? sizeof (journal_trip_item_t) : sizeof (journal_accel_item_t)];

unsigned char* journal_read_item(journal_reader_t* jr, uint8_t journal_type) {
    while (1) {
        // read item from eeprom
        int8_t _index;
        if (jr->item_num == 0) {
            _index = (int8_t) jr->item_current;
        } else {
            _index = (int8_t) (jr->item_current + jr->item_max - jr->item_num);
            if (jr->item_num <= jr->item_current) {
                _index -= jr->item_max;
            }
        }

        uint8_t _size = journal_type == 3 ? sizeof (journal_accel_item_t) : sizeof (journal_trip_item_t);

        JOURNAL_read_eeprom_block((unsigned char *) &item, journal_find_eeaddr(journal_type, _index), _size);

        // check, if item is valid. if not, read first item
        if (item[0] == JOURNAL_ITEM_OK || jr->item_num == 0) {
            break;
        }
        jr->item_num = 0;
    };
    return &item[0];
}

void journal_update_header() {
    JOURNAL_write_eeprom_block((unsigned char *) &journal_header, J_EEPROM_MARK_POS + 8, sizeof (journal_header));
}

flag_t journal_check_eeprom() {
    // check mark
    bool init_fl = true;
    unsigned char buf[8];

    while (1) {
        JOURNAL_read_eeprom_block((unsigned char *) &buf, J_EEPROM_MARK_POS, 8);
        if (memcmp(&buf, &journal_mark, (sizeof (journal_mark) - 1) <= 8 ? (sizeof (journal_mark) - 1) : 8) != 0) {
            if (init_fl) {
                // save init values on first attempt
                memcpy(&buf, &journal_mark, (sizeof (journal_mark) - 1) <= 8 ? (sizeof (journal_mark) - 1) : 8);
                JOURNAL_write_eeprom_block((unsigned char *) &buf, J_EEPROM_MARK_POS, 8);

                read_ds_time();
                fill_trip_time(&journal_header.time_trip_c);
                fill_trip_time(&journal_header.time_trip_a);
                fill_trip_time(&journal_header.time_trip_b);

                journal_update_header();
            } else {
                // set flag for no journal eeprom
                journal_support = 0;
                break;
            }
            init_fl = false;
        } else {
            // set flag for journal support
            journal_support = 1;
            break;
        }
    }
    if (journal_support != 0) {
        // read journal header
        JOURNAL_read_eeprom_block((unsigned char *) &journal_header, J_EEPROM_MARK_POS + 8, sizeof (journal_header));
    }
    return journal_support;
}

uint16_t journal_find_eeaddr(uint8_t index, int8_t item_index) {
    uint16_t eeaddr = J_EEPROM_DATA;
    for (uint8_t i = 0; i < index; i++) {
        eeaddr += sizeof (journal_trip_item_t) * journal_header.journal_type_pos[i].max;
    }
    journal_type_pos_t *lcd_cursor_position = &journal_header.journal_type_pos[index];
    if (item_index == -1) {
        if (++lcd_cursor_position->current >= lcd_cursor_position->max) {
            lcd_cursor_position->current = 0;
        }
        item_index = (int8_t) lcd_cursor_position->current;
    }
    return eeaddr + (uint16_t) ((index == 3 ? sizeof (journal_accel_item_t) : sizeof (journal_trip_item_t)) * ((uint8_t) item_index));
}

void journal_save_trip(trip_t *trip) {
    if (journal_support == 0) return;

    uint8_t index;
    trip_time_t *trip_time;

    journal_trip_item_t trip_item;

    if (trip == &trips.tripC) {
        index = 0;
        trip_item.start_time = journal_header.time_trip_c;
        trip_time = &journal_header.time_trip_c;
    } else if (trip == &trips.tripA) {
        index = 1;
        trip_item.start_time = journal_header.time_trip_a;
        trip_time = &journal_header.time_trip_a;
    } else if (trip == &trips.tripB) {
        index = 2;
        trip_item.start_time = journal_header.time_trip_b;
        trip_time = &journal_header.time_trip_b;
    } else {
        return;
    }

    uint16_t odo = get_trip_odometer(trip);

    // skip if zero distance
    if (odo != 0) {
        trip_item.status = JOURNAL_ITEM_OK;
        memcpy(&trip_item.trip, trip, sizeof (trip_t));
        // save trip item
        JOURNAL_write_eeprom_block((unsigned char *) &trip_item, journal_find_eeaddr(index, -1), sizeof (journal_trip_item_t));
    }

    // update header time
    read_ds_time();
    fill_trip_time(trip_time);

    // update header
    journal_update_header();

}

void journal_save_accel(uint8_t index) {
    if (journal_support == 0) return;

    journal_accel_item_t accel_item;

    accel_item.status = JOURNAL_ITEM_OK;
    accel_item.time = accel_meas_timer;

#ifdef EXTENDED_ACCELERATION_MEASUREMENT
    switch (index) {
        case 0:
            accel_item.lower = ACCEL_MEAS_LOWER_0;
            accel_item.upper = ACCEL_MEAS_UPPER_0;
            break;
        case 1:
            accel_item.lower = ACCEL_MEAS_LOWER_1;
            accel_item.upper = ACCEL_MEAS_UPPER_1;
            break;
        case 2:
            accel_item.lower = ACCEL_MEAS_LOWER_2;
            accel_item.upper = ACCEL_MEAS_UPPER_2;
            break;
        case 3:
            accel_item.lower = ACCEL_MEAS_LOWER_3;
            accel_item.upper = ACCEL_MEAS_UPPER_3;
            break;
    }
#else
    accel_item.lower = 0;
    accel_item.upper = 100;
#endif

    read_ds_time();
    fill_trip_time(&accel_item.start_time);

    // save accel item
    JOURNAL_write_eeprom_block((unsigned char *) &accel_item, journal_find_eeaddr(3, -1), sizeof (journal_accel_item_t));

    // update header
    journal_update_header();
}

#endif