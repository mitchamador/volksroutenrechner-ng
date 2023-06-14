MCU := $(or $(MCU),pic16f1938)
FIRMWARE := $(or $(FIRMWARE),1)

ifeq ($(MCU),pic16f876a)
 DFP_DIR := ${XC8-TOOLCHAIN}/packs/PIC16Fxxx_DFP/1.2.33
 MP_PROCESSOR_OPTION := 16F876A
else ifeq ($(MCU),pic16f1936)
 DFP_DIR := ${XC8-TOOLCHAIN}/packs/PIC12-16F1xxx_DFP/1.2.63
 MP_PROCESSOR_OPTION := 16F1936
else ifeq ($(MCU),pic16f1938)
 DFP_DIR := ${XC8-TOOLCHAIN}/packs/PIC12-16F1xxx_DFP/1.2.63
 MP_PROCESSOR_OPTION := 16F1938
else ifeq ($(MCU),pic18f252)
 DFP_DIR := ${XC8-TOOLCHAIN}/packs/PIC18Fxxxx_DFP/1.2.26
 MP_PROCESSOR_OPTION := 18F252
else ifeq ($(MCU),pic18f242)
 DFP_DIR := ${XC8-TOOLCHAIN}/packs/PIC18Fxxxx_DFP/1.2.26
 MP_PROCESSOR_OPTION := 18F242
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

ifneq (,$(filter pic16f% pic18f%,$(MCU)))
  CC_DIR=${XC8-TOOLCHAIN}/xc8/bin
  CC=${XC8-TOOLCHAIN}/xc8/bin/xc8-cc
  FIXDEPS=${XC8-TOOLCHAIN}/xc8/bin/fixDeps
  AVR_OBJCOPY=${XC8-TOOLCHAIN}/xc8/bin/avr-objcopy
ifneq (,$(findstring pic16f,$(MCU)))
  XC8_OPTIONS=-mno-osccal -mno-resetbits -mno-save-resetbits -mno-stackcall -mstack=compiled:auto:auto
else
  XC8_OPTIONS=-mdefault-config-bits -memi=wordwrite -merrata=none -mstack=compiled:auto:auto:auto
endif

else
  CC=${AVR-GCC-TOOLCHAIN}/bin/avr-gcc
  AVR_OBJCOPY=${AVR-GCC-TOOLCHAIN}/bin/avr-objcopy
  AVR_OBJDUMP=${AVR-GCC-TOOLCHAIN}/bin/avr-objdump
  AVR_SIZE=${AVR-GCC-TOOLCHAIN}/bin/avr-size
  TARGET=${MP_PROCESSOR_OPTION}${TARGET_SUFFIX}
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp  

ifneq (,$(filter pic16f% pic18f%,$(MCU)))
  TARGET=PIC${MP_PROCESSOR_OPTION}_legacy
endif

OUTPUTNAME=volksroutenrechner-ng
DEBUGGABLE_SUFFIX=elf
OUTPUT_SUFFIX=hex
ifeq ($(DEBUG),1)
IMAGE_TYPE=debug
else
IMAGE_TYPE=production
endif
OBJECTDIR=${TARGET}
DISTDIR=../dist/${TARGET}
OUTDIR_FIRMWARE=../firmware

SRCS = src/core.c src/hw-pic.c src/hw-avr.c src/main.c src/ds18b20.c src/ds3231.c src/i2c.c src/lcd.c src/utils.c src/onewire.c src/i2c-eeprom.c src/journal.c src/lcd_ssd1322.c
ifneq (,$(filter pic16f% pic18f%,$(MCU)))
  OBJS = $(SRCS:%.c=${OBJECTDIR}/${IMAGE_TYPE}/%.p1)
  DEPS := $(OBJS:.p1=.d)
else
  OBJS = $(SRCS:%.c=${OBJECTDIR}/${IMAGE_TYPE}/%.o)
  DEPS := $(OBJS:.o=.d)
endif

VPATH = ..

-include $(DEPS)

.PHONY: usage build release clobber clean version

usage:
	$(info usage:)
	$(info make XC8-TOOLCHAIN=/path/to/xc8-toolchain or AVR-GCC-TOOLCHAIN=/path/to/avr-gcc-toolchain MCU=TARGET_CPU)
	$(info TARGET_CPU is one of pic16F876a, pic16f1936, pic16f1938 (default), pic18f242, pic18f252, atmega168p, atmega328p)
	$(info creating release:)
	$(info make XC8-TOOLCHAIN=/path/to/xc8-toolchain AVR-GCC-TOOLCHAIN=/path/to/avr-gcc-toolchain release)

build: $(TARGET)
ifneq ($(OUTDIR),)
	${CP} ${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${OUTPUT_SUFFIX} ${OUTDIR}/${OUTPUTNAME}.${MCU}${TARGET_SUFFIX}.${OUTPUT_SUFFIX}
ifeq (,$(filter pic16f% pic18f%,$(MCU)))
	${CP} ${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.eep ${OUTDIR}/${OUTPUTNAME}.${MCU}${TARGET_SUFFIX}.eep
endif
endif
ifeq ($(EEPROM),1)
ifneq (,$(filter pic16f876a pic16f1938,$(MCU)))
	${AVR_OBJCOPY} -b 0 -i 2 -O binary -I elf32-little -j eeprom_data ${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX} ../proteus/eeprom_$(MP_PROCESSOR_OPTION).bin
else
ifneq (,$(filter pic18f252,$(MCU)))
	${AVR_OBJCOPY} -b 0 -i 1 -O binary -I elf32-little -j eeprom_data ${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX} ../proteus/eeprom_$(MP_PROCESSOR_OPTION).bin
else
ifneq (,$(filter atmega%,$(MCU)))
ifneq ($(ARDUINO),1)
	${AVR_OBJCOPY} -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O binary ${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX} ../proteus/eeprom_$(MP_PROCESSOR_OPTION).bin
endif
endif
endif
endif
endif

release: clobber version
ifneq ($(OUTDIR),)
	$(eval OUTDIR_FIRMWARE=${OUTDIR})
endif
ifdef XC8-TOOLCHAIN
	MCU=pic16f876a $(MAKE) -f $(firstword $(MAKEFILE_LIST)) CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} OUTDIR=${OUTDIR_FIRMWARE} build
#	MCU=pic16f1936 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} OUTDIR=${OUTDIR_FIRMWARE} build
	MCU=pic16f1938 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} OUTDIR=${OUTDIR_FIRMWARE} build
#	MCU=pic18f242 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} OUTDIR=${OUTDIR_FIRMWARE} build
	MCU=pic18f252 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} OUTDIR=${OUTDIR_FIRMWARE} build
endif
ifdef AVR-GCC-TOOLCHAIN
#	MCU=atmega168p $(MAKE) -f $(firstword $(MAKEFILE_LIST)) CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} OUTDIR=${OUTDIR_FIRMWARE} build
	MCU=atmega328p $(MAKE) -f $(firstword $(MAKEFILE_LIST)) CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} OUTDIR=${OUTDIR_FIRMWARE} build
#	MCU=atmega328p ARDUINO=1 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} OUTDIR=${OUTDIR_FIRMWARE} build
endif

clobber:
	$(info $(MAKEFILE_LIST))
	MCU=pic16f876a $(MAKE) -f $(firstword $(MAKEFILE_LIST)) clean
#	MCU=pic16f1936 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) clean
	MCU=pic16f1938 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) clean
#	MCU=pic18f242 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) clean
	MCU=pic18f252 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) clean
#	MCU=atmega168p $(MAKE) -f $(firstword $(MAKEFILE_LIST)) clean
	MCU=atmega328p $(MAKE) -f $(firstword $(MAKEFILE_LIST)) clean
#	MCU=atmega328p ARDUINO=1 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) clean

$(TARGET): $(OBJS)
	@${MKDIR} ${DISTDIR}/${IMAGE_TYPE} 
ifneq (,$(filter pic16f% pic18f%,$(MCU)))
	${CC} $(LDFLAGS) -mcpu=$(MP_PROCESSOR_OPTION) -Wl,-Map=${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.map  -DXPRJ_$(TARGET)=$(CND_CONF)  -Wl,--defsym=__MPLAB_BUILD=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -xassembler-with-cpp -I"../include" -mwarn=-3 -Wa,-a -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-download $(XC8_OPTIONS) -std=c99 -gcoff $(COMPARISON_BUILD) -Wl,--memorysummary,${DISTDIR}/${IMAGE_TYPE}//memoryfile.xml -o ${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${OUTPUT_SUFFIX}  ${OBJS}      
else
	${CC} $(LDFLAGS) -gdwarf-2 -Wl,-Map="${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.map" -o ${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX} -mmcu=$(MCU) -Os -Wl,--gc-sections -flto -fuse-linker-plugin ${OBJS} 
	${AVR_OBJCOPY} -O ihex -R .eeprom "${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX}" "${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${OUTPUT_SUFFIX}"
	${AVR_OBJCOPY} -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex "${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX}" "${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.eep" || exit 0 
	${AVR_OBJDUMP} -h -S "${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX}" >"${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.lss"
	${AVR_SIZE} -C --mcu=$(MCU) "${DISTDIR}/${IMAGE_TYPE}/${OUTPUTNAME}.production.${DEBUGGABLE_SUFFIX}"
endif

${OBJECTDIR}/${IMAGE_TYPE}/%.o : %.c
	@${MKDIR} "${OBJECTDIR}/${IMAGE_TYPE}/src"
ifeq ($(DEBUG),1)
	${CC} $(CPPFLAGS) -o $@ -mmcu=$(MCU) -std=gnu11 -fno-fat-lto-objects -MD -Og -DDEBUG -ffunction-sections -fdata-sections -flto -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums $(MP_EXTRA_CC_PRE) -I../include -I../src -c $<
else
	${CC} $(CPPFLAGS) -o $@ -mmcu=$(MCU) -std=gnu11 -fno-fat-lto-objects -MD -Os -ffunction-sections -fdata-sections -flto -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -mcall-prologues $(MP_EXTRA_CC_PRE) -I../include -I../src -c $<
endif

${OBJECTDIR}/${IMAGE_TYPE}/%.p1 : %.c
	@${MKDIR} "${OBJECTDIR}/${IMAGE_TYPE}/src"
ifeq ($(DEBUG),1)
	${CC} $(CPPFLAGS) -mcpu=$(MP_PROCESSOR_OPTION) -c -mdfp="${DFP_DIR}/xc8"  -D__DEBUG -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -xassembler-with-cpp -I"../include" -mwarn=-3 -Wa,-a -DXPRJ_$(TARGET)=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-download $(XC8_OPTIONS) $(COMPARISON_BUILD)  -std=c99 -gcoff -o $@ -c $<
else
	${CC} $(CPPFLAGS) -mcpu=$(MP_PROCESSOR_OPTION) -c -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -xassembler-with-cpp -I"../include" -mwarn=-3 -Wa,-a -DXPRJ_$(TARGET)=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-download $(XC8_OPTIONS) $(COMPARISON_BUILD)  -std=c99 -gcoff -o $@ -c $<
endif
	@${FIXDEPS} $(patsubst %.p1,%.d,$@) $(SILENT) -rsi ${CC_DIR}../ 

clean:
	@rm -rf $(OBJECTDIR)
	@rm -rf $(DISTDIR)

version:
ifneq ($(VERSION),0)
	@$(shell sh version.cmd)
endif
