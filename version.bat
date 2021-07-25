@echo off

for /f "delims=" %%a in ('wmic path win32_localtime get dayofweek /format:list ') do for /f "delims=" %%d in ("%%a") do set %%d

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
echo #define VERSION_MINUTE_BCD 0x%time:~3,2% >>include\version.h
echo #define VERSION_HOUR_BCD 0x%time:~0,2% >>include\version.h
echo #define VERSION_DAY_OF_WEEK_BCD 0x0%dayofweek% >>include\version.h
echo #define VERSION_DAY_OF_MONTH_BCD 0x%date:~0,2% >>include\version.h
echo #define VERSION_MONTH_BCD 0x%date:~3,2% >>include\version.h
echo #define VERSION_YEAR_BCD 0x%date:~8,2% >>include\version.h
echo. >>include\version.h
echo #endif	/* VERSION_H */ >>include\version.h


