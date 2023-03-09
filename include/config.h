#ifndef CONFIG_H
#define	CONFIG_H

#if defined(__DEBUG) || defined(DEBUG)
#define _DEBUG_
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__)

#if defined(__AVR_ATmega168P__)
// 1602 lcd 4bit
#define LCD_LEGACY
// no encoder support
#define NO_ENCODER
// disable service counters' configuration and checking
#define NO_SERVICE_COUNTERS_CHECKS
// no fuel tank support
#define NO_FUEL_TANK
#elif defined(ARDUINO)
// 1602 lcd i2c
#define LCD_1602_I2C
// adc buttons connected to PC0/ADC0
#define ADC_BUTTONS
// save default eeprom in progmem
#define PROGMEM_EEPROM
// use internal eeprom for trip journal
#define JOURNAL_EEPROM_INTERNAL
#else
// 1602 lcd 4bit
#define LCD_LEGACY
// use internal eeprom for trip journal
#define JOURNAL_EEPROM_INTERNAL
#endif

#elif defined(_16F876A) || defined(_16F1936) || defined (_16F1938) || defined(_18F252)  || defined(_18F242)
// legacy hardware
#define HW_LEGACY
// no encoder on pic mcu
#define NO_ENCODER

#if defined(_16F876A)
// simple adc handler
#define SIMPLE_ADC
// skip oled lcd reset sequence (though works ok without it after power up with EH1602 REV.J)
//#define NO_LCD_OLED_RESET
// simple checking time difference (decrease memory usage)
#define SIMPLE_TRIPC_TIME_CHECK
// auto calculate day of week
//#define NO_AUTO_DAY_OF_WEEK
// min speed settings
//#define NO_MIN_SPEED_CONFIG
// speed 0-100 measurement only
#define NO_EXTENDED_ACCELERATION_MEASUREMENT
// disable all service counters' support
//#define NO_SERVICE_COUNTERS
// disable service counters' configuration and checking
#define NO_SERVICE_COUNTERS_CHECKS
// journal trip
#define NO_JOURNAL
// disable temperature support
//#define NO_TEMPERATURE
// use ds18b20 temp sensors
//#define NO_DS18B20
// use temp sensor from ds3231
#define NO_DS3231_TEMP
// ds18b20 configuration
//#define NO_DS18B20_CONFIG
// show temp for sensors' configuration
//#define DS18B20_CONFIG_SHOW_TEMP
// extended ds18b20 configuration
#define NO_DS18B20_CONFIG_EXT
// support for prev key for legacy hw
#define NO_KEY3
// voltage min/max
//#define NO_MIN_MAX_VOLTAGES
// no fuel tank support
#define NO_FUEL_TANK
// no instant fuel averaging
#define NO_INSTANT_FUEL_AVERAGE
#endif /* _16F876A */

#if defined(_16F1936)
// simple adc handler
#define SIMPLE_ADC
// speed 0-100 measurement only
//#define NO_EXTENDED_ACCELERATION_MEASUREMENT
// disable all service counters' support
//#define NO_SERVICE_COUNTERS
// disable service counters' configuration and checking
#define NO_SERVICE_COUNTERS_CHECKS
// journal trip
#define NO_JOURNAL
// disable temperature support
//#define NO_TEMPERATURE
// ds18b20 configuration
//#define NO_DS18B20
// use temp sensor from ds3231
//#define NO_DS3231_TEMP
// use ds18b20 temp sensors
//#define NO_DS18B20_CONFIG
// extended ds18b20 configuration
#define NO_DS18B20_CONFIG_EXT
// show temp for sensors' configuration
#define DS18B20_CONFIG_SHOW_TEMP
// use temp sensor from ds3231
#define NO_KEY3
// voltage min/max
//#define NO_MIN_MAX_VOLTAGES
// no fuel tank support
#define NO_FUEL_TANK
// no instant fuel averaging
//#define NO_INSTANT_FUEL_AVERAGE
#endif /* _16F1936 */

#if defined(_18F242)
// simple adc handler
#define SIMPLE_ADC
// speed 0-100 measurement only
//#define NO_EXTENDED_ACCELERATION_MEASUREMENT
// disable all service counters' support
//#define NO_SERVICE_COUNTERS
// disable service counters' configuration and checking
#define NO_SERVICE_COUNTERS_CHECKS
// journal trip
//#define NO_JOURNAL
// disable temperature support
//#define NO_TEMPERATURE
// ds18b20 configuration
//#define NO_DS18B20
// use temp sensor from ds3231
#define NO_DS3231_TEMP
// use ds18b20 temp sensors
//#define NO_DS18B20_CONFIG
// extended ds18b20 configuration
#define NO_DS18B20_CONFIG_EXT
// show temp for sensors' configuration
#define DS18B20_CONFIG_SHOW_TEMP
// use temp sensor from ds3231
#define NO_KEY3
// voltage min/max
//#define NO_MIN_MAX_VOLTAGES
// no fuel tank support
#define NO_FUEL_TANK
// no instant fuel averaging
//#define NO_INSTANT_FUEL_AVERAGE

#endif /* _18F242 */

#if defined(_16F1938) || defined(_18F252)
// support for prev key for legacy hw
#define NO_KEY3
#endif /* _16F1938 */

#else
#error("configuration not supported")
#endif

// end of configuration

// disable temperature support
#if !defined(NO_TEMPERATURE)

// use temp sensor from ds3231
#ifndef NO_DS3231_TEMP
#define DS3231_TEMP
#endif

// disable ds18b20 support
#ifndef NO_DS18B20
#define DS18B20_TEMP
#endif

#ifdef DS18B20_TEMP
// disable ds18b20 config screen
#ifndef NO_DS18B20_CONFIG
#define DS18B20_CONFIG
#endif
#endif

#if defined(DS18B20_TEMP) || defined(DS3231_TEMP)
#define TEMPERATURE_SUPPORT
#endif

#ifdef DS18B20_CONFIG

// extended ds18b20 config (use onewire search)
#ifndef NO_DS18B20_CONFIG_EXT
#define DS18B20_CONFIG_EXT
#endif /* NO_DS18B20_CONFIG_EXT */

#endif /* DS18B20_CONFIG */

#endif /* !NO_TEMPERATURE */

// disable all service counters' support
#ifdef NO_SERVICE_COUNTERS
#ifndef NO_SERVICE_COUNTERS_CHECKS
#define NO_SERVICE_COUNTERS_CHECKS
#endif
#else
#define SERVICE_COUNTERS_SUPPORT
#endif

// disable service counters' configuration and checking
#ifndef NO_SERVICE_COUNTERS_CHECKS
#define SERVICE_COUNTERS_CHECKS_SUPPORT
#endif

// disable sound support
#ifndef NO_SOUND
#define SOUND_SUPPORT
#endif

#ifndef NO_JOURNAL
#define JOURNAL_SUPPORT
#endif

// auto calculate day of week
#ifndef NO_AUTO_DAY_OF_WEEK
#define AUTO_DAY_OF_WEEK
#endif

// min speed settings
#ifndef NO_MIN_SPEED_CONFIG
#define MIN_SPEED_CONFIG
#endif

// key3 support
#ifndef NO_KEY3
#define KEY3_SUPPORT
#endif

// encoder support
#ifndef NO_ENCODER
#define ENCODER_SUPPORT
#endif

// extended acceleration measurement
#ifndef NO_EXTENDED_ACCELERATION_MEASUREMENT
#define EXTENDED_ACCELERATION_MEASUREMENT
#endif

// fuel tank and continuous fuel/speed support
#ifndef NO_FUEL_TANK
//#define FUEL_TANK_SUPPORT
#define CONTINUOUS_DATA_SUPPORT
#endif

// no fuel tank support with simple adc handler
#if defined(SIMPLE_ADC) && defined(FUEL_TANK_SUPPORT)
#undef FUEL_TANK_SUPPORT
#endif

// voltages min/max 
#ifndef NO_MIN_MAX_VOLTAGES
#define MIN_MAX_VOLTAGES_SUPPORT
#endif

// instant fuel averaging
#ifndef NO_INSTANT_FUEL_AVERAGE
#define INSTANT_FUEL_AVERAGE_SUPPORT
#endif

#if defined(ENCODER_SUPPORT) && !defined(KEY3_SUPPORT)
#define KEY3_SUPPORT
#endif

#endif	/* CONFIG_H */

