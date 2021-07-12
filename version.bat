@echo off
echo #ifndef VERSION_H >include\version.h
echo #define	VERSION_H >>include\version.h
echo. >>include\version.h
echo #include "hw.h" >>include\version.h
echo #ifndef __AVR_ATMEGA >>include\version.h
echo #define PROGMEM >>include\version.h
echo #endif >>include\version.h
echo. >>include\version.h 
echo PROGMEM const char version_str[] = "%time:~0,5% %date%"; >>include\version.h
echo. >>include\version.h
echo #endif	/* VERSION_H */ >>include\version.h


