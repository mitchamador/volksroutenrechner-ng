:; if [ -z 0 ]; then
  @echo off
  goto :WINDOWS
fi

dayofweek=$(date +%w)
if [ $dayofweek == 0 ]; then dayofweek=7; fi

echo \#ifndef VERSION_H >../include/version.h
echo \#define VERSION_H >>../include/version.h
echo >>../include/version.h
echo \#define VERSION \"$(date +"%H:%M %d.%m.%Y")\" >>../include/version.h
echo >>../include/version.h
echo \#define VERSION_MINUTE_BCD 0x$(date +%M) >>../include/version.h
echo \#define VERSION_HOUR_BCD 0x$(date +%H) >>../include/version.h
echo \#define VERSION_DAY_OF_WEEK_BCD 0x0${dayofweek} >>../include/version.h
echo \#define VERSION_DAY_OF_MONTH_BCD 0x$(date +%d) >>../include/version.h
echo \#define VERSION_MONTH_BCD 0x$(date +%m) >>../include/version.h
echo \#define VERSION_YEAR_BCD 0x$(date +%y) >>../include/version.h
echo \#define VERSION_YEAR $(date +%y) >>../include/version.h
echo >>../include/version.h
echo \#endif \/\* VERSION_H \*\/ >>../include/version.h

# bash stuff
exit

:WINDOWS

@echo off

for /f "delims=" %%a in ('wmic path win32_localtime get dayofweek /format:list ') do for /f "delims=" %%d in ("%%a") do set %%d

echo #ifndef VERSION_H >..\include\version.h
echo #define VERSION_H >>..\include\version.h
echo. >>..\include\version.h
set _time=%TIME: =0%
if %dayofweek%==0 (set /a "dayofweek=7")
echo #define VERSION "%_time:~0,5% %date%" >>..\include\version.h
echo. >>..\include\version.h
echo #define VERSION_MINUTE_BCD 0x%_time:~3,2% >>..\include\version.h
echo #define VERSION_HOUR_BCD 0x%_time:~0,2% >>..\include\version.h
echo #define VERSION_DAY_OF_WEEK_BCD 0x0%dayofweek% >>..\include\version.h
echo #define VERSION_DAY_OF_MONTH_BCD 0x%date:~0,2% >>..\include\version.h
echo #define VERSION_MONTH_BCD 0x%date:~3,2% >>..\include\version.h
echo #define VERSION_YEAR_BCD 0x%date:~8,2% >>..\include\version.h
echo #define VERSION_YEAR %date:~8,2% >>..\include\version.h
echo. >>..\include\version.h
echo #endif	/* VERSION_H */ >>..\include\version.h

:: windows stuff