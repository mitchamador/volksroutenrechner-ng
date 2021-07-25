#ifndef VERSION_H 
#define	VERSION_H 
 
#include "hw.h" 
#ifndef __AVR_ATMEGA 
#define PROGMEM 
#endif 
  
PROGMEM const char version_str[] = "12:17 25.07.2021"; 
 
#define VERSION_MINUTE_BCD 0x17 
#define VERSION_HOUR_BCD 0x12 
#define VERSION_DAY_OF_WEEK_BCD 0x00 
#define VERSION_DAY_OF_MONTH_BCD 0x25 
#define VERSION_MONTH_BCD 0x07 
#define VERSION_YEAR_BCD 0x21 
 
#endif	/* VERSION_H */ 
