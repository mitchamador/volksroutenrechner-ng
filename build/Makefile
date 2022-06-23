MCU := $(or $(MCU),pic16f1938)

ifeq ($(MCU),pic16f876a)
 DFP_DIR := ${XC8-TOOLCHAIN}/PIC16Fxxx_DFP/1.2.33
 MP_PROCESSOR_OPTION := 16F876A
else ifeq ($(MCU),pic16f1936)
 DFP_DIR := ${XC8-TOOLCHAIN}/PIC12-16F1xxx_DFP/1.2.63
 MP_PROCESSOR_OPTION := 16F1936
else ifeq ($(MCU),pic16f1938)
 DFP_DIR := ${XC8-TOOLCHAIN}/PIC12-16F1xxx_DFP/1.2.63
 MP_PROCESSOR_OPTION := 16F1938
else ifeq ($(MCU),atmega328p)
 MP_PROCESSOR_OPTION := ATMega328P
 ifeq ($(ARDUINO),1)
  MP_EXTRA_CC_PRE="-DARDUINO"
  TARGET_SUFFIX=_arduino
 endif
else ifeq ($(MCU),atmega168p)
 MP_PROCESSOR_OPTION := ATMega168P
else
 $(error MCU=${MCU} not supported)
endif

ifneq (,$(findstring pic16f,$(MCU)))
  MP_CC=${XC8-TOOLCHAIN}/xc8/v2.32/bin/xc8-cc
  AVR_OBJCOPY=${XC8-TOOLCHAIN}/xc8/v2.32/bin/avr-objcopy
else
  MP_CC=${AVR-GCC-TOOLCHAIN}/bin/avr-gcc
  AVR_OBJCOPY=${AVR-GCC-TOOLCHAIN}/bin/avr-objcopy
  TARGET=${MP_PROCESSOR_OPTION}${TARGET_SUFFIX}
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp  

ifneq (,$(findstring pic16f,$(MCU)))
  TARGET=PIC${MP_PROCESSOR_OPTION}_legacy
endif

OUTPUTNAME=volksroutenrechner-ng
DEBUGGABLE_SUFFIX=elf
OUTPUT_SUFFIX=hex
OBJECTDIR=${TARGET}

SRCS = src/onewire.c src/ds18b20.c src/ds1307.c src/hw-avr.c src/hw-pic.c src/i2c.c src/lcd.c src/locale.c src/main.c src/utils.c
ifneq (,$(findstring pic16f,$(MCU)))
  OBJS = $(SRCS:%.c=${OBJECTDIR}/%.p1)
else
  OBJS = $(SRCS:%.c=${OBJECTDIR}/%.o)
endif

VPATH = ..

.PHONY: usage build clean release

usage:
	$(info usage:)
	$(info make XC8-TOOLCHAIN=/path/to/xc8-toolchain or AVR-GCC-TOOLCHAIN=/path/to/avr-gcc-toolchain MCU=TARGET_CPU)
	$(info TARGET_CPU is one of pic16F876a, pic16f1936, pic16f1938 (default), atmega168p, atmega328p)
	$(info creating release:)
	$(info make XC8-TOOLCHAIN=/path/to/xc8-toolchain AVR-GCC-TOOLCHAIN=/path/to/avr-gcc-toolchain release)

build: $(TARGET)
	${CP} ${OBJECTDIR}/${OUTPUTNAME}.production.${OUTPUT_SUFFIX} ../firmware/${OUTPUTNAME}.${MCU}${TARGET_SUFFIX}.${OUTPUT_SUFFIX}
ifeq (,$(findstring pic16f,$(MCU)))
	${CP} ${OBJECTDIR}/${OUTPUTNAME}.production.eep ../firmware/${OUTPUTNAME}.${MCU}${TARGET_SUFFIX}.eep
endif
ifeq ($(EEPROM),1)
ifneq (,$(findstring pic16f,$(MCU)))
	${AVR_OBJCOPY} -b 0 -i 2 -O binary -I elf32-little -j eeprom_data ${OBJECTDIR}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX} ../proteus/eeprom_$(MP_PROCESSOR_OPTION).bin
else
ifneq ($(ARDUINO),1)
	${AVR_OBJCOPY} -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O binary ${OBJECTDIR}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX} ../proteus/eeprom_$(MP_PROCESSOR_OPTION).bin
endif
endif
endif

release: version
ifdef XC8-TOOLCHAIN
	MCU=pic16f876a $(MAKE) MP_EXTRA_CC_PRE="-DNO_SERVICE_COUNTERS_CHECKS" -f $(lastword $(MAKEFILE_LIST)) clean build
	MCU=pic16f1936 $(MAKE) -f $(lastword $(MAKEFILE_LIST)) clean build
	MCU=pic16f1938 $(MAKE) -f $(lastword $(MAKEFILE_LIST)) clean build
endif
ifdef AVR-GCC-TOOLCHAIN
	MCU=atmega328p $(MAKE) -f $(lastword $(MAKEFILE_LIST)) clean build
	MCU=atmega328p ARDUINO=1 $(MAKE) -f $(lastword $(MAKEFILE_LIST)) clean build
endif

$(TARGET): $(OBJS)
	@${MKDIR} ${OBJECTDIR} 
ifneq (,$(findstring pic16f,$(MCU)))
	${MP_CC} $(MP_EXTRA_LD_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -Wl,-Map=${OBJECTDIR}/${OUTPUTNAME}.production.map  -DXPRJ_$(TARGET)=$(CND_CONF)  -Wl,--defsym=__MPLAB_BUILD=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"../include" -mwarn=-3 -Wa,-a -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall -std=c99 -gcoff -mstack=compiled:auto:auto     $(COMPARISON_BUILD) -Wl,--memorysummary,${OBJECTDIR}/memoryfile.xml -o ${OBJECTDIR}/${OUTPUTNAME}.production.${OUTPUT_SUFFIX}  ${OBJS}      
else
	${MP_CC} -o ${OBJECTDIR}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX} -mmcu=$(MCU) -Os -Wl,--gc-sections -flto -fuse-linker-plugin ${OBJS} 
	${AVR_OBJCOPY} -O ihex -R .eeprom "${OBJECTDIR}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX}" "${OBJECTDIR}/${OUTPUTNAME}.production.${OUTPUT_SUFFIX}"
	${AVR_OBJCOPY} -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex "${OBJECTDIR}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX}" "${OBJECTDIR}/${OUTPUTNAME}.production.eep" || exit 0 
endif

${OBJECTDIR}/%.o : %.c
	@${MKDIR} "${OBJECTDIR}/src"
	${MP_CC} -o $@ -mmcu=$(MCU) -std=gnu11 -fno-fat-lto-objects -mmcu=atmega328p -Os -ffunction-sections -fdata-sections -flto $(MP_EXTRA_CC_PRE) -I../include -I../src -c $<

${OBJECTDIR}/%.p1 : %.c
	@${MKDIR} "${OBJECTDIR}/src"
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"../include" -mwarn=-3 -Wa,-a -DXPRJ_$(TARGET)=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o $@ -c $<

clean:
	@rm -rf $(OBJECTDIR)

version:
	@version.cmd