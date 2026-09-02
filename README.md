# volksroutenrechner-ng

*[English](#volksroutenrechner-ng) | [Русский](#volksroutenrechner-ng-1)*

Firmware for a DIY automotive trip computer, built for **PIC** and **AVR**
microcontrollers. The project is written in C and the same source tree is
compiled for several different target MCUs thanks to an extensive set of
compile-time switches (`config.h`, `hw_pic.h` / `hw_avr.h`).

The trip computer measures vehicle speed, engine RPM, fuel consumption,
on-board (battery) voltage and temperature, keeps several mileage counters,
stores settings and a trip journal in EEPROM, and displays everything on a
16×2 text LCD.

## Features

- **Speed and mileage** — a main odometer plus three independent trip
  counters: Trip A, Trip B (can be used as a monthly counter) and Trip C
  (can be used as a daily counter), each automatically accumulating fuel
  consumption and travel time.
- **Fuel consumption** — instantaneous and average consumption (L/100km and
  L/h), optional filtering (smoothing) of the instantaneous value, and a
  continuous speed/consumption data-logging mode (`CONTINUOUS_DATA_SUPPORT`)
  for more accurate statistics.
- **Engine RPM (tachometer)** — RPM calculated from coil/injector pulses,
  with support for paired (wasted-spark/sequential-pair) injection.
- **Acceleration timing** — measures acceleration time over several speed
  ranges (0–100, 0–60, 60–100, 80–120 km/h).
- **Temperature** — up to three external DS18B20 sensors (outside/cabin/
  engine) over 1-Wire, and/or the DS3231's built-in temperature sensor.
- **Real-time clock** — DS3231 (I2C), with automatic day-of-week
  calculation.
- **Service counters** — up to 4 independent maintenance
  mileage/time counters with a warning when a service interval is
  approaching.
- **Trip journal** — trip and acceleration-run history stored in an
  external I2C EEPROM (24LC16) or in the AVR's internal EEPROM.
- **Display** — a character 16×2 LCD, either classic HD44780-compatible
  (4-bit bus or via a PCF8574 I2C backpack), or a graphical SSD1322 (OLED)
  panel emulating a 16×2 character display buffer.
- **Controls** — 2 or 3 buttons, optionally a rotary encoder with a button
  and/or analog buttons on a single ADC channel; audible feedback for
  button presses/alarms via a piezo buzzer.
- **Power monitoring** — on-board voltage measurement (including min/max),
  a threshold for detecting ignition on/off, and safe data flushing on
  power loss.
- **Settings storage** — all settings, trip counters and calibration
  constants (odometer pulse ratio, flow-meter constant, voltage ADC
  correction) are stored in EEPROM.

The exact feature set is configured per target controller via macros in
`include/config.h` (e.g. lower-end PIC models are built without the trip
journal, fuel tank support, continuous data logging, etc. — see below).

## Supported hardware

The firmware builds for the following microcontrollers (see `MCU` in
`build/Makefile`):

| MCU (`MCU=...`) | Family       | Build notes                                            |
|------------------|--------------|----------------------------------------------------------|
| `pic16f876a`      | PIC16F876A   | "legacy" hardware, simplified ADC, no trip journal        |
| `pic16f1936`      | PIC16F1936   | "legacy" hardware                                          |
| `pic16f1938`      | PIC16F1938   | "legacy" hardware, default target MCU                      |
| `pic18f242`       | PIC18F242    | "legacy" hardware, no trip journal                          |
| `pic18f252`       | PIC18F252    | "legacy" hardware                                           |
| `atmega168p`      | ATmega168P   | 4-bit 16x2 LCD, no encoder, no service counters, no fuel tank |
| `atmega328p`      | ATmega328P   | full feature set; also buildable as `ARDUINO=1`             |

Building for `pic16f876a`/`pic16f1936`/`pic16f1938`/`pic18f242`/`pic18f252`
requires the Microchip **XC8** toolchain; building for `atmega168p`/
`atmega328p` requires **avr-gcc**.

### Peripherals

- LCD: HD44780 16×2 (4-bit or I2C via PCF8574), or an SSD1322 OLED
  (emulating a 16×2 character display).
- Clock: DS3231 over I2C.
- Temperature sensors: DS18B20 over 1-Wire (up to 3).
- External EEPROM for the trip journal: 24LC16 over I2C (or the AVR's
  internal EEPROM when `JOURNAL_EEPROM_INTERNAL` is set).
- ADC inputs: on-board voltage, fuel level sensor, optionally analog
  buttons.
- Pulse inputs: vehicle speed (VSS), engine RPM/injector pulses, rotary
  encoder (optional).
- Piezo buzzer for audible feedback.

## Project layout

```
volksroutenrechner-ng/
├── build/
│   ├── Makefile         # builds all supported MCU targets
│   └── version.cmd       # generates include/version.h (build date/time)
├── include/               # headers
│   ├── config.h           # feature set selection per target MCU
│   ├── core.h              # shared data structures, constants, EEPROM layout
│   ├── hw.h, hw_avr.h, hw_pic.h    # HAL and per-MCU settings
│   ├── main.h              # main-menu / configuration screen structures
│   ├── locale.h             # UI strings and display characters
│   ├── lcd.h, lcd_ssd1322.h  # display drivers
│   ├── ds3231.h, ds18b20.h   # RTC and temperature sensor drivers
│   ├── i2c.h, i2c_eeprom.h, onewire.h, spi.h   # buses and external EEPROM
│   ├── journal.h, journal_eeprom.h, eeprom.h   # trip journal and EEPROM layout
│   ├── utils.h              # helper functions
│   └── fonts/                # fonts for the graphical display
└── src/                    # implementation (.c files match the headers above)
    ├── main.c               # screen logic, menus, button handling (main file)
    ├── core.c               # timer/interrupt handlers, parameter calculation
    ├── hw_avr.c, hw_pic.c    # peripheral init for the specific MCU
    └── ...
```

## Building

Requires GNU Make and the matching toolchain:

- for PIC: Microchip **XC8** (`XC8-TOOLCHAIN` variable);
- for AVR: **avr-gcc** (`AVR-GCC-TOOLCHAIN` variable).

```sh
cd build

# build for a specific MCU (default is MCU=pic16f1938)
make XC8-TOOLCHAIN=/path/to/xc8 MCU=pic16f1938

# build for ATmega328P
make AVR-GCC-TOOLCHAIN=/path/to/avr-gcc-toolchain MCU=atmega328p

# build for ATmega328P to run behind an Arduino bootloader
make AVR-GCC-TOOLCHAIN=/path/to/avr-gcc-toolchain MCU=atmega328p ARDUINO=1

# release build for all target MCUs of both toolchains at once
make XC8-TOOLCHAIN=/path/to/xc8 AVR-GCC-TOOLCHAIN=/path/to/avr-gcc-toolchain release
```

Build artifacts (`.hex`, plus `.eep`, `.elf`, `.lss` and a memory map for
AVR) are placed in `../dist/<TARGET>/<production|debug>/`. You can also
pass `OUTDIR=/path` to have the resulting `.hex`/`.eep` files copied there
as `volksroutenrechner-ng.<mcu>[_arduino].hex`.

Useful build variables:

- `DEBUG=1` — debug build (`-Og`, `DEBUG` macro defined);
- `EEPROM=1` — additionally extract an EEPROM data image (used for
  programming the EEPROM or for Proteus simulation, `../proteus`
  directory);
- `VERSION=0` — don't regenerate `include/version.h` before building (by
  default the version is the build date/time, see `build/version.cmd`).

`make clean` cleans intermediate files for the current `MCU`; `make
clobber` cleans them for every supported MCU at once.

## Configuration

The set of supported features for each MCU is controlled by defines in
`include/config.h` (e.g. `NO_JOURNAL`, `NO_FUEL_TANK`,
`NO_SERVICE_COUNTERS`, `NO_ENCODER`, `LCD_1602_I2C`,
`ADC_BUTTONS_SUPPORT`, etc.) — lower-end / older controllers are built with
a reduced feature set due to memory constraints. Calibration constants
(odometer pulse ratio, flow-meter constant, voltage ADC correction) are
stored in EEPROM and configured from the on-device configuration screen.

## Status

Licensing and authorship are not specified in the source files — please
check with the repository author before republishing or redistributing the
firmware.

---

# volksroutenrechner-ng

Прошивка автомобильного маршрутного бортового компьютера (trip computer) для
самодельных конструкций на базе микроконтроллеров **PIC** и **AVR**. Проект
написан на C и собирается под несколько целевых МК одним и тем же исходным
кодом за счёт развитой системы условной компиляции (`config.h`, `hw_pic.h` /
`hw_avr.h`).

Бортовой компьютер измеряет скорость, обороты двигателя, расход топлива,
напряжение бортовой сети и температуру, ведёт несколько счётчиков пробега,
хранит настройки и журнал поездок в EEPROM и выводит всё это на текстовый
ЖК-дисплей 16×2.

## Возможности

- **Скорость и пробег** — основной одометр, а также три отдельных счётчика
  поездок: Trip A, Trip B (может использоваться как месячный счётчик) и
  Trip C (может использоваться как суточный счётчик), с автоматическим
  накоплением расхода топлива и времени в пути по каждому.
- **Расход топлива** — мгновенный и средний расход (л/100км и л/ч), опциональная
  фильтрация (усреднение) мгновенного расхода, а также режим непрерывного
  сбора данных скорость/расход (`CONTINUOUS_DATA_SUPPORT`) для более точной
  статистики.
- **Обороты двигателя (тахометр)** — расчёт RPM по импульсам с катушки/форсунки,
  с поддержкой парного (парно-последовательного) впрыска.
- **Замер разгона** — измерение времени разгона в нескольких диапазонах
  скорости (0–100, 0–60, 60–100, 80–120 км/ч).
- **Температура** — до трёх внешних датчиков DS18B20 (улица/салон/двигатель)
  по шине 1-Wire и/или встроенный датчик температуры DS3231.
- **Часы реального времени** — на базе DS3231 (I2C), с автоматическим
  определением дня недели.
- **Счётчики ТО (service counters)** — до 4 независимых счётчиков
  межсервисного пробега/времени с предупреждением о приближении срока
  обслуживания.
- **Журнал поездок (trip journal)** — сохранение истории поездок и заездов на
  разгон во внешней I2C EEPROM (24LC16) либо во внутренней EEPROM AVR.
- **Индикация** — символьный ЖК 16×2: как классический HD44780-совместимый
  (4-битная шина или через I2C-переходник PCF8574), так и графический
  SSD1322 (OLED), эмулирующий буфер 16×2 символьного дисплея.
- **Управление** — кнопки (2 или 3 шт.), опционально энкодер с кнопкой и/или
  аналоговые кнопки на одном канале АЦП; звуковая индикация нажатий/аварий
  (пьезодинамик).
- **Контроль питания** — измерение напряжения бортсети (в т.ч. мин/макс),
  порог для определения включения/выключения зажигания и корректного
  завершения записи данных при пропадании питания.
- **Хранение настроек** — все настройки, счётчики поездок и калибровочные
  константы (постоянные одометра, расходомера, АЦП напряжения) хранятся в
  EEPROM.

Конкретный набор функций конфигурируется под целевой контроллер через
макросы в `include/config.h` (например, младшие модели PIC собираются без
журнала поездок, без бака/непрерывных данных и т.д. — см. ниже).

## Поддерживаемое железо

Прошивка собирается под следующие микроконтроллеры (см. `MCU` в
`build/Makefile`):

| MCU (`MCU=...`)   | Семейство      | Особенности сборки                                   |
|--------------------|----------------|-------------------------------------------------------|
| `pic16f876a`        | PIC16F876A     | "legacy" железо, упрощённый АЦП, без журнала           |
| `pic16f1936`        | PIC16F1936     | "legacy" железо                                        |
| `pic16f1938`        | PIC16F1938     | "legacy" железо, целевой МК по умолчанию                |
| `pic18f242`         | PIC18F242      | "legacy" железо, без журнала                            |
| `pic18f252`         | PIC18F252      | "legacy" железо                                         |
| `atmega168p`        | ATmega168P     | ЖК 1602 4-бит, без энкодера, без счётчиков ТО, без бака |
| `atmega328p`        | ATmega328P     | полный набор функций; есть сборка `ARDUINO=1`           |

Сборка `pic16f876a`/`pic16f1936`/`pic16f1938`/`pic18f242`/`pic18f252` требует
Microchip **XC8** toolchain, сборка `atmega168p`/`atmega328p` — **avr-gcc**.

### Периферия

- ЖК-дисплей: HD44780 1602 (4-бит либо I2C через PCF8574) или SSD1322 OLED
  (эмуляция 1602).
- Часы: DS3231 по I2C.
- Датчики температуры: DS18B20 по 1-Wire (до 3 шт.).
- Внешняя EEPROM для журнала поездок: 24LC16 по I2C (либо внутренняя EEPROM
  AVR при `JOURNAL_EEPROM_INTERNAL`).
- Входные каналы АЦП: напряжение бортсети, уровень топлива (датчик уровня),
  опционально — аналоговые кнопки.
- Датчики импульсов: скорость (VSS), обороты двигателя/форсунки, энкодер
  (опционально).
- Пьезодинамик для звуковой индикации.

## Структура проекта

```
volksroutenrechner-ng/
├── build/
│   ├── Makefile        # сборка под все поддерживаемые MCU
│   └── version.cmd      # генерация include/version.h (дата/время сборки)
├── include/              # заголовочные файлы
│   ├── config.h          # выбор набора функций под целевой MCU
│   ├── core.h             # общие структуры данных, константы, EEPROM-раскладка
│   ├── hw.h, hw_avr.h, hw_pic.h   # HAL и настройки конкретного MCU
│   ├── main.h             # структуры экранов главного меню/конфигурации
│   ├── locale.h            # строки интерфейса и символы дисплея
│   ├── lcd.h, lcd_ssd1322.h # драйверы дисплеев
│   ├── ds3231.h, ds18b20.h  # драйверы RTC и датчиков температуры
│   ├── i2c.h, i2c-eeprom.h, onewire.h, spi.h  # шины и внешняя EEPROM
│   ├── journal.h, eeprom-journal.h, eeprom.h  # журнал поездок и раскладка EEPROM
│   ├── utils.h            # вспомогательные функции
│   └── fonts/              # шрифты для графического дисплея
└── src/                   # реализация (файлы .c соответствуют заголовкам выше)
    ├── main.c              # логика экранов, меню, обработка кнопок (основной файл)
    ├── core.c              # обработчики таймеров/прерываний, расчёт параметров
    ├── hw_avr.c, hw_pic.c   # инициализация периферии под конкретный MCU
    └── ...
```

## Сборка

Требуется GNU Make и соответствующий тулчейн:

- для PIC: Microchip **XC8** (переменная `XC8-TOOLCHAIN`);
- для AVR: **avr-gcc** (переменная `AVR-GCC-TOOLCHAIN`).

```sh
cd build

# сборка под конкретный MCU (по умолчанию MCU=pic16f1938)
make XC8-TOOLCHAIN=/path/to/xc8 MCU=pic16f1938

# сборка под ATmega328P
make AVR-GCC-TOOLCHAIN=/path/to/avr-gcc-toolchain MCU=atmega328p

# сборка под ATmega328P в связке с Arduino-бутлоадером
make AVR-GCC-TOOLCHAIN=/path/to/avr-gcc-toolchain MCU=atmega328p ARDUINO=1

# релизная сборка сразу под все целевые MCU обоих тулчейнов
make XC8-TOOLCHAIN=/path/to/xc8 AVR-GCC-TOOLCHAIN=/path/to/avr-gcc-toolchain release
```

Результаты сборки (`.hex`, для AVR также `.eep`, `.elf`, `.lss`, карта памяти)
кладутся в `../dist/<TARGET>/<production|debug>/`. Дополнительно можно указать
`OUTDIR=/путь` — тогда итоговые `.hex`/`.eep` файлы будут скопированы туда с
именем `volksroutenrechner-ng.<mcu>[_arduino].hex`.

Полезные переменные сборки:

- `DEBUG=1` — отладочная сборка (`-Og`, макрос `DEBUG`);
- `EEPROM=1` — дополнительно извлечь образ данных EEPROM (используется для
  прошивки EEPROM или для симуляции в Proteus, каталог `../proteus`);
- `VERSION=0` — не перегенерировать `include/version.h` перед сборкой
  (по умолчанию версия — дата и время сборки, см. `build/version.cmd`).

`make clean` очищает промежуточные файлы для текущего `MCU`, `make clobber` —
для всех поддерживаемых MCU сразу.

## Конфигурация

Набор поддерживаемых возможностей для каждого MCU задаётся дефайнами в
`include/config.h` (например `NO_JOURNAL`, `NO_FUEL_TANK`,
`NO_SERVICE_COUNTERS`, `NO_ENCODER`, `LCD_1602_I2C`, `ADC_BUTTONS_SUPPORT` и
т.д.) — младшие/более старые контроллеры собираются с урезанным набором
функций из-за ограничений по памяти. Калибровочные константы (передаточное
число одометра, константа расходомера, поправка АЦП по напряжению) хранятся
в EEPROM и настраиваются через экран конфигурации на самом устройстве.

## Статус

Лицензия и авторство в исходных файлах не указаны — при необходимости
уточните их у автора репозитория, прежде чем публиковать или распространять
прошивку.

