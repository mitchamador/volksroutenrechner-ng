#ifndef LOCALE_H
#define	LOCALE_H

#include <hw.h>
#ifndef __AVR_ATMEGA
#define PROGMEM
#endif

#define HOUR_SYMBOL 'h'
#define LITRE_SYMBOL 'l'
#define VOLT_SYMBOL 'V'
#define KM1_SYMBOL 'k'
#define KM2_SYMBOL 'm'
#define SECONDS_SYMBOL 's'
#define CELSIUS_SYMBOL 0xDF
#define MAX_SPEED_SYMBOL '<'

// custom lcd characters values
#define _kmh0   0x00
#define _kmh1	0x01
#define _omin0	0x02
#define _omin1	0x03
#define _lkm0	0x04
#define _lkm1	0x05
#define _lh0	0x06
#define _lh1    0x07

PROGMEM const char empty_string[] = "----";
PROGMEM const char no_time_string[] = "-----'--";
PROGMEM const char trip_string[] = "trip ";
PROGMEM const char onoff_string[] = "\0 OFF\0  ON";
PROGMEM const char time_correction[] = "time correction?";
PROGMEM const char reset_string[] = "reset?"; 
PROGMEM const char speed100_string[] = "0-100 timing"; 
PROGMEM const char speed100_wait_string[] = "wait for start"; 
PROGMEM const char timeout_string[] = "timeout"; 
PROGMEM const char warning_str[] = "WARNING"; 

PROGMEM const char service_menu_title[] = "SERVICE MENU";
PROGMEM const char service_counters[] = "\0engine hours\0engine oil\0gearbox oil\0air filter\0spark plugs";

#ifdef HW_LEGACY
#define ALT_BUTTONS "\0"
#else
#define ALT_BUTTONS "\0alt buttons"
#endif
PROGMEM const char settings_bits[] = "\0dual inj\0skip temp\0key sound\0serv alarm\0fast refresh" ALT_BUTTONS "\0\0";

PROGMEM const char temp_sensors[] = "\0---\0out\0in\0eng";

PROGMEM const char fuel_constant_str[] = "FUEL constant";
PROGMEM const char vss_constant_str[] = "VSS constant";
PROGMEM const char total_trip_str[] = "total trip";
PROGMEM const char voltage_adjust_str[] = "voltage adjust";
PROGMEM const char settings_bits_str[] = "settings bits";
PROGMEM const char temp_sensor_str[] = "temp sensors";
PROGMEM const char service_counters_str[] = "service cntrs";
PROGMEM const char version_info_str[] = "sw version";

PROGMEM const char day_of_week_str[] = "\0sunday\0monday\0tuesday\0wednesday\0thursday\0friday\0saturday";
PROGMEM const char month_str[] = "\0jan\0feb\0mar\0apr\0may\0jun\0jul\0aug\0sep\0oct\0nov\0dec";

#define DATA_KMH_0  0x05, 0x06, 0x05, 0x00, 0x00, 0x00, 0x01, 0x02
#define DATA_KMH_1  0x0A, 0x15, 0x15, 0x00, 0x10, 0x05, 0x07, 0x01
#define DATA_OMIN_0 0x07, 0x05, 0x07, 0x00, 0x00, 0x01, 0x02, 0x00
#define DATA_OMIN_1 0x00, 0x00, 0x08, 0x10, 0x00, 0x0A, 0x15, 0x15
#define DATA_L100_0 0x0C, 0x14, 0x14, 0x01, 0x02, 0x05, 0x01, 0x01
#define DATA_L100_1 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x15, 0x1F
#define DATA_LH_0   0x03, 0x05, 0x05, 0x00, 0x00, 0x01, 0x02, 0x00
#define DATA_LH_1   0x00, 0x00, 0x08, 0x10, 0x00, 0x14, 0x1C, 0x04

#endif	/* LOCALE_H */

