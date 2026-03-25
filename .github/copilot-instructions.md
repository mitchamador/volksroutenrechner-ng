# Copilot Instructions for Volksroutenrechner-NG

## Project Overview
A car trip computer firmware supporting multiple embedded platforms. Tracks trip data (distance, fuel, time, temperature) via sensor inputs and displays on LCD with persistent EEPROM storage.

## Architecture

### Hardware Abstraction Layer
The codebase uses **conditional compilation + macro abstraction** for multi-platform support:
- **hw.h** defines platform-agnostic interface (timers, ADC, GPIO, EEPROM)
- Platform implementations: **hw-avr.h** (ATMega328P) and **hw-pic.h** (legacy PIC16/PIC18)
- All MCU-specific details (timers, interrupts, port numbers) hidden in platform headers
- Build flags (platformio.ini) determine which hw-*.h is included

**When modifying hardware control**: Always use HW_* macros. Never directly access registers in non-hw-*.h files.

### Core Components

| Component | Purpose | Files |
|-----------|---------|-------|
| **core** | Trip data structures, timing constants, trip logic | core.h/c |
| **main** | UI screens, event loop, trip recording | main.c, main.h |
| **journal** | Trip history EEPROM storage (internal or i2c) | journal.h/c, i2c-eeprom.h/c |
| **i2c** | I2C bus (hardware or software) | i2c.h/c |
| **RTC** | Date/time from DS3231 chip | ds3231.h/c |
| **Temperature** | DS18B20 one-wire sensors | ds18b20.h/c, onewire.h/c |
| **LCD** | Dual driver support: 1602 (I2C) or SSD1322 (SPI) | lcd.h/c, lcd_ssd1322.h/c |
| **Sensors** | RPM, speed pulses (via timers), fuel level (ADC), buttons (analog/digital) | (timer ISRs in hw*.h, ADC in main.c) |

### Data Flow: How It Works
1. **Main timer ISR** (10ms): Samples all sensors via ADC/GPIO, computes RPM/speed
2. **Trip logic** (core.c): Updates trip distance, fuel consumption, avg speeds
3. **Screen refresh**: main.c renders trip data to LCD buffer
4. **Storage**: Trip data saved to EEPROM; journal records individual trips for history

## Key Development Patterns & Conventions

### 1. Platform-Specific Configuration
`include/config.h` uses nested #ifdef blocks per MCU + features. Example:
```c
#if defined(__AVR_ATmega328P__)
#define LCD_1602  // or LCD_SSD1322 variant
#define ENCODER_SUPPORT
#define NO_FUEL_TANK  // strip features for memory
```
- **Never edit main logic to match config flags.** Instead, add #ifdef guards in headers.
- Each platformio.ini environment passes build_flags that set these macros.

### 2. EEPROM Layout: Memory-Conscious Design
- **ATMega328P internal**: 768 bytes → header (24) + 20 trip-c records (18 each) + 12 trip-a + 6 trip-b + 5 accel
- **i2c external (24lc16)**: 2KB → relaxed limits, ~126 data blocks
- See journal.h for exact limits. **Math:_ trip_record = 18 bytes, accel = 10 bytes.

### 3. Macro Abstraction for Cross-Platform
Examples in hw-avr.h:
```c
#define HW_key2_pressed()        ((PINC & _BV(PINC0)) == 0)
#define HW_adc_read()            (ADCW)
#define HW_start_main_timer()    TCCR1B = ...
```
**Policy**: All GPIO/timer/interrupt control → HW_* macros. This keeps platform-specific logic contained.

### 4. Screen & Menu System
main.c uses **function pointers in arrays**:
```c
screen_item_t items_main[] = {
    { .screen = screen_main, .page.index = SCREEN_INDEX_MAIN, ... },
    { .screen = screen_trip, .page.index = SCREEN_INDEX_TRIP_C, ... },
};
```
New screen? Add struct to array, implement handler function, update enum.

### 5. Time-Critical vs. Non-Critical Code
- **Time-critical** (IRQ context): Main timer (~10ms), sensor sampling, trip accumulation → must be fast
- **Non-critical** (event loop): UI rendering, EEPROM writes → deferred to main loop
- **Golden rule**: Minimize work in ISR. Set flags (`timeout_timer1 = 0`, `screen_refresh = 1`), handle in main.

## Build & Development Workflows

### PlatformIO (Recommended)
```bash
pio run -e atmega328p                           # Build for ATMega
pio run -e avr-ssd1322                          # Build SSD1322 variant
pio run --environment atmega328p --target upload # Upload via USBASP
```

### VSCode Tasks
- **PlatformIO: Build** task (default) → configurable in Tasks menu
- Clean: `pio clean`

### Legacy Support
- PIC16F1938, PIC16F876A, PIC18F252 in nbproject/ + Makefile (not actively using PlatformIO for legacy)
- Modern development focuses on ATMega328P with conditional flags for legacy variants

## Testing & Validation

### Proteus Simulation
- `proteus/mk-atmega328p.pdsprj` — primary simulation file
- `proteus/mk-pic16f1938-legacy.pdsprj` and others for legacy hardware
- Useful for verifying timing, sensor signal processing before hardware test

### Version Management
- **Manual version.h**: Hour:Minute DAY.MONTH.YEAR (e.g., "19:55 09.01.2025")
- Also stores BCD values for RTC display
- Update version.h before releases

## Quick Reference: File Dependencies

```
main.c (entry)
├── core.h (trip logic, timing)
├── lcd.h / lcd_ssd1322.h (display)
├── hw.h → hw-avr.h | hw-pic.h (platform)
├── journal.h → i2c-eeprom.h (async storage)
├── ds3231.h (RTC via I2C)
├── ds18b20.h + onewire.h (temp via 1-wire)
├── i2c.h (bus control)
└── utils.h (string/format utils)
```

## Common Pitfalls & Tips

- **Conditional compilation bugs**: Test all variants (`-e atmega328p`, `-e arduino`, `-e atmega328p-ssd1322`). A missing #ifdef can silently break legacy builds.
- **Memory pressure**: ATMega328P has ~2KB RAM. Check stack/heap in final build. `fpack-struct -fshort-enums` flags shrink struct sizes.
- **EEPROM wear**: Trip journal is write-heavy. External i2c EEPROM preferred for high-mileage use.
- **RTC sync**: DS3231 can be set via config menu; time persists across power loss.
- **Sensor tuning**: Fuel constant, VSS constant (odometer), and RPM constants in config— calibration critical for accuracy.

## When Adding Features

1. **Hardware control?** Add HW_* macro to appropriate hw-*.h
2. **New sensor?** Extend core.h trip struct + main.c sampling logic
3. **New screen?** Add to items_main[] array + implement handler in main.c
4. **Multi-platform support?** Wrap in #ifdef per target: `#if defined(__AVR_ATmega328P__) || defined(_16F1938)`
5. **Low memory?** Use #define NO_* flags in config.h to strip features selectively
