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
ifeq "$(wildcard nbproject/Makefile-local-ATMega328P.mk)" "nbproject/Makefile-local-ATMega328P.mk"
include nbproject/Makefile-local-ATMega328P.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=ATMega328P
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
COMPARISON_BUILD=
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
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/src/ds18b20.o ${OBJECTDIR}/src/ds1307.o ${OBJECTDIR}/src/hw-pic.o ${OBJECTDIR}/src/i2c-pic.o ${OBJECTDIR}/src/lcd.o ${OBJECTDIR}/src/main.o ${OBJECTDIR}/src/utils.o ${OBJECTDIR}/src/hw-avr.o ${OBJECTDIR}/src/i2c-avr.o
POSSIBLE_DEPFILES=${OBJECTDIR}/src/ds18b20.o.d ${OBJECTDIR}/src/ds1307.o.d ${OBJECTDIR}/src/hw-pic.o.d ${OBJECTDIR}/src/i2c-pic.o.d ${OBJECTDIR}/src/lcd.o.d ${OBJECTDIR}/src/main.o.d ${OBJECTDIR}/src/utils.o.d ${OBJECTDIR}/src/hw-avr.o.d ${OBJECTDIR}/src/i2c-avr.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/src/ds18b20.o ${OBJECTDIR}/src/ds1307.o ${OBJECTDIR}/src/hw-pic.o ${OBJECTDIR}/src/i2c-pic.o ${OBJECTDIR}/src/lcd.o ${OBJECTDIR}/src/main.o ${OBJECTDIR}/src/utils.o ${OBJECTDIR}/src/hw-avr.o ${OBJECTDIR}/src/i2c-avr.o

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

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-ATMega328P.mk dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=ATmega328P
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/src/ds18b20.o: src/ds18b20.c  .generated_files/flags/ATMega328P/3d70246683cccc557395eea7331658ebf4245326 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds18b20.o.d 
	@${RM} ${OBJECTDIR}/src/ds18b20.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1 -g -DDEBUG  -gdwarf-2  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/ds18b20.o.d" -MT "${OBJECTDIR}/src/ds18b20.o.d" -MT ${OBJECTDIR}/src/ds18b20.o -o ${OBJECTDIR}/src/ds18b20.o src/ds18b20.c 
	
${OBJECTDIR}/src/ds1307.o: src/ds1307.c  .generated_files/flags/ATMega328P/3c6a667c14cae33361f612178cc0b30b5cc948ad .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds1307.o.d 
	@${RM} ${OBJECTDIR}/src/ds1307.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1 -g -DDEBUG  -gdwarf-2  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/ds1307.o.d" -MT "${OBJECTDIR}/src/ds1307.o.d" -MT ${OBJECTDIR}/src/ds1307.o -o ${OBJECTDIR}/src/ds1307.o src/ds1307.c 
	
${OBJECTDIR}/src/hw-pic.o: src/hw-pic.c  .generated_files/flags/ATMega328P/edf299ec7f3fa0ac5861d82d6e2f1fbae9305cf2 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-pic.o.d 
	@${RM} ${OBJECTDIR}/src/hw-pic.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1 -g -DDEBUG  -gdwarf-2  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/hw-pic.o.d" -MT "${OBJECTDIR}/src/hw-pic.o.d" -MT ${OBJECTDIR}/src/hw-pic.o -o ${OBJECTDIR}/src/hw-pic.o src/hw-pic.c 
	
${OBJECTDIR}/src/i2c-pic.o: src/i2c-pic.c  .generated_files/flags/ATMega328P/87cc0fe4793a756e859d8e767682f7163b839d55 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-pic.o.d 
	@${RM} ${OBJECTDIR}/src/i2c-pic.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1 -g -DDEBUG  -gdwarf-2  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/i2c-pic.o.d" -MT "${OBJECTDIR}/src/i2c-pic.o.d" -MT ${OBJECTDIR}/src/i2c-pic.o -o ${OBJECTDIR}/src/i2c-pic.o src/i2c-pic.c 
	
${OBJECTDIR}/src/lcd.o: src/lcd.c  .generated_files/flags/ATMega328P/4afb049ac1e86671bf8458b69fb4503170f23529 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/lcd.o.d 
	@${RM} ${OBJECTDIR}/src/lcd.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1 -g -DDEBUG  -gdwarf-2  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/lcd.o.d" -MT "${OBJECTDIR}/src/lcd.o.d" -MT ${OBJECTDIR}/src/lcd.o -o ${OBJECTDIR}/src/lcd.o src/lcd.c 
	
${OBJECTDIR}/src/main.o: src/main.c  .generated_files/flags/ATMega328P/fad1f5b1d59182531a92aa595f956114835c4351 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/main.o.d 
	@${RM} ${OBJECTDIR}/src/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1 -g -DDEBUG  -gdwarf-2  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/main.o.d" -MT "${OBJECTDIR}/src/main.o.d" -MT ${OBJECTDIR}/src/main.o -o ${OBJECTDIR}/src/main.o src/main.c 
	
${OBJECTDIR}/src/utils.o: src/utils.c  .generated_files/flags/ATMega328P/978bb2f7f8f3a634dc0b6eb529b142f840c4f3d9 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/utils.o.d 
	@${RM} ${OBJECTDIR}/src/utils.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1 -g -DDEBUG  -gdwarf-2  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/utils.o.d" -MT "${OBJECTDIR}/src/utils.o.d" -MT ${OBJECTDIR}/src/utils.o -o ${OBJECTDIR}/src/utils.o src/utils.c 
	
${OBJECTDIR}/src/hw-avr.o: src/hw-avr.c  .generated_files/flags/ATMega328P/ad70ae4393638ce8dc8539cde23ebd4d98b2c58f .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-avr.o.d 
	@${RM} ${OBJECTDIR}/src/hw-avr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1 -g -DDEBUG  -gdwarf-2  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/hw-avr.o.d" -MT "${OBJECTDIR}/src/hw-avr.o.d" -MT ${OBJECTDIR}/src/hw-avr.o -o ${OBJECTDIR}/src/hw-avr.o src/hw-avr.c 
	
${OBJECTDIR}/src/i2c-avr.o: src/i2c-avr.c  .generated_files/flags/ATMega328P/39fedb002de1c4d41d73d52a13f1dec51f514f59 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-avr.o.d 
	@${RM} ${OBJECTDIR}/src/i2c-avr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -D__DEBUG=1 -g -DDEBUG  -gdwarf-2  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/i2c-avr.o.d" -MT "${OBJECTDIR}/src/i2c-avr.o.d" -MT ${OBJECTDIR}/src/i2c-avr.o -o ${OBJECTDIR}/src/i2c-avr.o src/i2c-avr.c 
	
else
${OBJECTDIR}/src/ds18b20.o: src/ds18b20.c  .generated_files/flags/ATMega328P/9c00c04b7f58ddbb294a14a141ff485fdcf8378d .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds18b20.o.d 
	@${RM} ${OBJECTDIR}/src/ds18b20.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/ds18b20.o.d" -MT "${OBJECTDIR}/src/ds18b20.o.d" -MT ${OBJECTDIR}/src/ds18b20.o -o ${OBJECTDIR}/src/ds18b20.o src/ds18b20.c 
	
${OBJECTDIR}/src/ds1307.o: src/ds1307.c  .generated_files/flags/ATMega328P/b89422a573d3785cc93a73b7dccc0ad081cd9c26 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds1307.o.d 
	@${RM} ${OBJECTDIR}/src/ds1307.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/ds1307.o.d" -MT "${OBJECTDIR}/src/ds1307.o.d" -MT ${OBJECTDIR}/src/ds1307.o -o ${OBJECTDIR}/src/ds1307.o src/ds1307.c 
	
${OBJECTDIR}/src/hw-pic.o: src/hw-pic.c  .generated_files/flags/ATMega328P/fc8e9c29e718ed304e78066f3bd3967af9e8ffa3 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-pic.o.d 
	@${RM} ${OBJECTDIR}/src/hw-pic.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/hw-pic.o.d" -MT "${OBJECTDIR}/src/hw-pic.o.d" -MT ${OBJECTDIR}/src/hw-pic.o -o ${OBJECTDIR}/src/hw-pic.o src/hw-pic.c 
	
${OBJECTDIR}/src/i2c-pic.o: src/i2c-pic.c  .generated_files/flags/ATMega328P/9c64dde93bc441c21e9c54551faf3d0e99338f5d .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-pic.o.d 
	@${RM} ${OBJECTDIR}/src/i2c-pic.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/i2c-pic.o.d" -MT "${OBJECTDIR}/src/i2c-pic.o.d" -MT ${OBJECTDIR}/src/i2c-pic.o -o ${OBJECTDIR}/src/i2c-pic.o src/i2c-pic.c 
	
${OBJECTDIR}/src/lcd.o: src/lcd.c  .generated_files/flags/ATMega328P/74abbcc00985ac94dbd8283ca8e22769bcf60313 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/lcd.o.d 
	@${RM} ${OBJECTDIR}/src/lcd.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/lcd.o.d" -MT "${OBJECTDIR}/src/lcd.o.d" -MT ${OBJECTDIR}/src/lcd.o -o ${OBJECTDIR}/src/lcd.o src/lcd.c 
	
${OBJECTDIR}/src/main.o: src/main.c  .generated_files/flags/ATMega328P/f51ac17ced7436b8124dce1787bf8e7b0c2f8020 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/main.o.d 
	@${RM} ${OBJECTDIR}/src/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/main.o.d" -MT "${OBJECTDIR}/src/main.o.d" -MT ${OBJECTDIR}/src/main.o -o ${OBJECTDIR}/src/main.o src/main.c 
	
${OBJECTDIR}/src/utils.o: src/utils.c  .generated_files/flags/ATMega328P/b346d35afdeab12d000c9e34b21a4f151622d9f5 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/utils.o.d 
	@${RM} ${OBJECTDIR}/src/utils.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/utils.o.d" -MT "${OBJECTDIR}/src/utils.o.d" -MT ${OBJECTDIR}/src/utils.o -o ${OBJECTDIR}/src/utils.o src/utils.c 
	
${OBJECTDIR}/src/hw-avr.o: src/hw-avr.c  .generated_files/flags/ATMega328P/ff017a8e8d9523f2cced41ec59a428f021d73ae9 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-avr.o.d 
	@${RM} ${OBJECTDIR}/src/hw-avr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/hw-avr.o.d" -MT "${OBJECTDIR}/src/hw-avr.o.d" -MT ${OBJECTDIR}/src/hw-avr.o -o ${OBJECTDIR}/src/hw-avr.o src/hw-avr.c 
	
${OBJECTDIR}/src/i2c-avr.o: src/i2c-avr.c  .generated_files/flags/ATMega328P/416799ec774460757d3e1ef889be30d90c4210b4 .generated_files/flags/ATMega328P/95dcf237f9529e21c59a51a986c56bf395bae4f3
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-avr.o.d 
	@${RM} ${OBJECTDIR}/src/i2c-avr.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -c  -x c -D__$(MP_PROCESSOR_OPTION)__   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -gdwarf-3     -MD -MP -MF "${OBJECTDIR}/src/i2c-avr.o.d" -MT "${OBJECTDIR}/src/i2c-avr.o.d" -MT ${OBJECTDIR}/src/i2c-avr.o -o ${OBJECTDIR}/src/i2c-avr.o src/i2c-avr.c 
	
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
	${MP_CC} $(MP_EXTRA_LD_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -Wl,-Map=dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.map  -D__DEBUG=1  -DXPRJ_ATMega328P=$(CND_CONF)  -Wl,--defsym=__MPLAB_BUILD=1   -mdfp="${DFP_DIR}/xc8"   -gdwarf-2 -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -gdwarf-3     $(COMPARISON_BUILD) -Wl,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -o dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  -o dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -Wl,--start-group  -Wl,-lm -Wl,--end-group  -Wl,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1
	@${RM} dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.hex 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mcpu=$(MP_PROCESSOR_OPTION) -Wl,-Map=dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.map  -DXPRJ_ATMega328P=$(CND_CONF)  -Wl,--defsym=__MPLAB_BUILD=1   -mdfp="${DFP_DIR}/xc8"  -Wl,--gc-sections -O1 -Og -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -I"include" -gdwarf-3     $(COMPARISON_BUILD) -Wl,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -o dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  -o dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -Wl,--start-group  -Wl,-lm -Wl,--end-group 
	${MP_CC_DIR}\\avr-objcopy -O ihex "dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}" "dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.hex"
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/ATMega328P
	${RM} -r dist/ATMega328P

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
