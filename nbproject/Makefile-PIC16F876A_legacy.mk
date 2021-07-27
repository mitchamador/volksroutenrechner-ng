#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-PIC16F876A_legacy.mk)" "nbproject/Makefile-local-PIC16F876A_legacy.mk"
include nbproject/Makefile-local-PIC16F876A_legacy.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=PIC16F876A_legacy
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=src/ds18b20.c src/ds1307.c src/hw-pic.c src/i2c-pic.c src/lcd.c src/main.c src/utils.c src/hw-avr.c src/i2c-avr.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/src/ds18b20.p1 ${OBJECTDIR}/src/ds1307.p1 ${OBJECTDIR}/src/hw-pic.p1 ${OBJECTDIR}/src/i2c-pic.p1 ${OBJECTDIR}/src/lcd.p1 ${OBJECTDIR}/src/main.p1 ${OBJECTDIR}/src/utils.p1 ${OBJECTDIR}/src/hw-avr.p1 ${OBJECTDIR}/src/i2c-avr.p1
POSSIBLE_DEPFILES=${OBJECTDIR}/src/ds18b20.p1.d ${OBJECTDIR}/src/ds1307.p1.d ${OBJECTDIR}/src/hw-pic.p1.d ${OBJECTDIR}/src/i2c-pic.p1.d ${OBJECTDIR}/src/lcd.p1.d ${OBJECTDIR}/src/main.p1.d ${OBJECTDIR}/src/utils.p1.d ${OBJECTDIR}/src/hw-avr.p1.d ${OBJECTDIR}/src/i2c-avr.p1.d

# Object Files
OBJECTFILES=${OBJECTDIR}/src/ds18b20.p1 ${OBJECTDIR}/src/ds1307.p1 ${OBJECTDIR}/src/hw-pic.p1 ${OBJECTDIR}/src/i2c-pic.p1 ${OBJECTDIR}/src/lcd.p1 ${OBJECTDIR}/src/main.p1 ${OBJECTDIR}/src/utils.p1 ${OBJECTDIR}/src/hw-avr.p1 ${OBJECTDIR}/src/i2c-avr.p1

# Source Files
SOURCEFILES=src/ds18b20.c src/ds1307.c src/hw-pic.c src/i2c-pic.c src/lcd.c src/main.c src/utils.c src/hw-avr.c src/i2c-avr.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

# The following macros may be used in the pre and post step lines
_/_=\\
ShExtension=.bat
Device=PIC16F876A
ProjectDir="D:\_dev_\volksroutenrechner-ng"
ProjectName=volksroutenrechner-ng
ConfName=PIC16F876A_legacy
ImagePath="dist\PIC16F876A_legacy\${IMAGE_TYPE}\volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ImageDir="dist\PIC16F876A_legacy\${IMAGE_TYPE}"
ImageName="volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IsDebug="true"
else
IsDebug="false"
endif

.build-conf:  .pre ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-PIC16F876A_legacy.mk dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
	@echo "--------------------------------------"
	@echo "User defined post-build step: [if ${IsDebug} == "true" ( ${MP_CC_DIR}\avr-objcopy.exe -b 0 -i 2 -O binary -I elf32-little -j eeprom_data ${ImagePath} proteus\eeprom.bin )]"
	@if ${IsDebug} == "true" ( ${MP_CC_DIR}\avr-objcopy.exe -b 0 -i 2 -O binary -I elf32-little -j eeprom_data ${ImagePath} proteus\eeprom.bin )
	@echo "--------------------------------------"

MP_PROCESSOR_OPTION=16F876A
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/src/ds18b20.p1: src/ds18b20.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds18b20.p1.d 
	@${RM} ${OBJECTDIR}/src/ds18b20.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/ds18b20.p1 src/ds18b20.c 
	@-${MV} ${OBJECTDIR}/src/ds18b20.d ${OBJECTDIR}/src/ds18b20.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/ds18b20.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/ds1307.p1: src/ds1307.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds1307.p1.d 
	@${RM} ${OBJECTDIR}/src/ds1307.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/ds1307.p1 src/ds1307.c 
	@-${MV} ${OBJECTDIR}/src/ds1307.d ${OBJECTDIR}/src/ds1307.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/ds1307.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/hw-pic.p1: src/hw-pic.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-pic.p1.d 
	@${RM} ${OBJECTDIR}/src/hw-pic.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/hw-pic.p1 src/hw-pic.c 
	@-${MV} ${OBJECTDIR}/src/hw-pic.d ${OBJECTDIR}/src/hw-pic.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/hw-pic.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/i2c-pic.p1: src/i2c-pic.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-pic.p1.d 
	@${RM} ${OBJECTDIR}/src/i2c-pic.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/i2c-pic.p1 src/i2c-pic.c 
	@-${MV} ${OBJECTDIR}/src/i2c-pic.d ${OBJECTDIR}/src/i2c-pic.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/i2c-pic.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/lcd.p1: src/lcd.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/lcd.p1.d 
	@${RM} ${OBJECTDIR}/src/lcd.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/lcd.p1 src/lcd.c 
	@-${MV} ${OBJECTDIR}/src/lcd.d ${OBJECTDIR}/src/lcd.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/lcd.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/main.p1: src/main.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/main.p1.d 
	@${RM} ${OBJECTDIR}/src/main.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/main.p1 src/main.c 
	@-${MV} ${OBJECTDIR}/src/main.d ${OBJECTDIR}/src/main.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/main.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/utils.p1: src/utils.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/utils.p1.d 
	@${RM} ${OBJECTDIR}/src/utils.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/utils.p1 src/utils.c 
	@-${MV} ${OBJECTDIR}/src/utils.d ${OBJECTDIR}/src/utils.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/utils.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/hw-avr.p1: src/hw-avr.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-avr.p1.d 
	@${RM} ${OBJECTDIR}/src/hw-avr.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/hw-avr.p1 src/hw-avr.c 
	@-${MV} ${OBJECTDIR}/src/hw-avr.d ${OBJECTDIR}/src/hw-avr.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/hw-avr.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/i2c-avr.p1: src/i2c-avr.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-avr.p1.d 
	@${RM} ${OBJECTDIR}/src/i2c-avr.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/i2c-avr.p1 src/i2c-avr.c 
	@-${MV} ${OBJECTDIR}/src/i2c-avr.d ${OBJECTDIR}/src/i2c-avr.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/i2c-avr.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
else
${OBJECTDIR}/src/ds18b20.p1: src/ds18b20.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds18b20.p1.d 
	@${RM} ${OBJECTDIR}/src/ds18b20.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/ds18b20.p1 src/ds18b20.c 
	@-${MV} ${OBJECTDIR}/src/ds18b20.d ${OBJECTDIR}/src/ds18b20.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/ds18b20.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/ds1307.p1: src/ds1307.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds1307.p1.d 
	@${RM} ${OBJECTDIR}/src/ds1307.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/ds1307.p1 src/ds1307.c 
	@-${MV} ${OBJECTDIR}/src/ds1307.d ${OBJECTDIR}/src/ds1307.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/ds1307.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/hw-pic.p1: src/hw-pic.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-pic.p1.d 
	@${RM} ${OBJECTDIR}/src/hw-pic.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/hw-pic.p1 src/hw-pic.c 
	@-${MV} ${OBJECTDIR}/src/hw-pic.d ${OBJECTDIR}/src/hw-pic.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/hw-pic.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/i2c-pic.p1: src/i2c-pic.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-pic.p1.d 
	@${RM} ${OBJECTDIR}/src/i2c-pic.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/i2c-pic.p1 src/i2c-pic.c 
	@-${MV} ${OBJECTDIR}/src/i2c-pic.d ${OBJECTDIR}/src/i2c-pic.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/i2c-pic.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/lcd.p1: src/lcd.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/lcd.p1.d 
	@${RM} ${OBJECTDIR}/src/lcd.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/lcd.p1 src/lcd.c 
	@-${MV} ${OBJECTDIR}/src/lcd.d ${OBJECTDIR}/src/lcd.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/lcd.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/main.p1: src/main.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/main.p1.d 
	@${RM} ${OBJECTDIR}/src/main.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/main.p1 src/main.c 
	@-${MV} ${OBJECTDIR}/src/main.d ${OBJECTDIR}/src/main.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/main.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/utils.p1: src/utils.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/utils.p1.d 
	@${RM} ${OBJECTDIR}/src/utils.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/utils.p1 src/utils.c 
	@-${MV} ${OBJECTDIR}/src/utils.d ${OBJECTDIR}/src/utils.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/utils.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/hw-avr.p1: src/hw-avr.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-avr.p1.d 
	@${RM} ${OBJECTDIR}/src/hw-avr.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/hw-avr.p1 src/hw-avr.c 
	@-${MV} ${OBJECTDIR}/src/hw-avr.d ${OBJECTDIR}/src/hw-avr.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/hw-avr.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/src/i2c-avr.p1: src/i2c-avr.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-avr.p1.d 
	@${RM} ${OBJECTDIR}/src/i2c-avr.p1 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall $(COMPARISON_BUILD)  -std=c99 -gcoff -mstack=compiled:auto:auto     -o ${OBJECTDIR}/src/i2c-avr.p1 src/i2c-avr.c 
	@-${MV} ${OBJECTDIR}/src/i2c-avr.d ${OBJECTDIR}/src/i2c-avr.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/src/i2c-avr.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -Wl,-Map=dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.map  -D__DEBUG=1  -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -Wl,--defsym=__MPLAB_BUILD=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall -std=c99 -gcoff -mstack=compiled:auto:auto        $(COMPARISON_BUILD) -Wl,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -o dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	@${RM} dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.hex 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -Wl,-Map=dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.map  -DXPRJ_PIC16F876A_legacy=$(CND_CONF)  -Wl,--defsym=__MPLAB_BUILD=1   -mdfp="${DFP_DIR}/xc8"  -fno-short-double -fno-short-float -Os -fasmfile -maddrqual=require -DHW_LEGACY -xassembler-with-cpp -I"include" -mwarn=-3 -Wa,-a -msummary=-psect,-class,+mem,-hex,-file  -ginhx32 -Wl,--data-init -mkeep-startup -mno-osccal -mno-resetbits -mno-save-resetbits -mno-download -mno-stackcall -std=c99 -gcoff -mstack=compiled:auto:auto     $(COMPARISON_BUILD) -Wl,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -o dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	
endif

.pre:
	@echo "--------------------------------------"
	@echo "User defined pre-build step: [if ${IsDebug} == "false" ( ${ProjectDir}\version.bat )]"
	@if ${IsDebug} == "false" ( ${ProjectDir}\version.bat )
	@echo "--------------------------------------"

# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/PIC16F876A_legacy
	${RM} -r dist/PIC16F876A_legacy

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
