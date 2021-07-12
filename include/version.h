#ifndef VERSION_H 
#define	VERSION_H 
 
#include "hw.h" 
#ifndef __AVR_ATMEGA 
#define PROGMEM 
#endif 
  
PROGMEM const char version_str[] = "13:34 12.07.2021"; 
 
#endif	/* VERSION_H */ 
