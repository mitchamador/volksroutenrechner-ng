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
SOURCEFILES_QUOTED_IF_SPACED=src/ds18b20.c src/ds1307.c src/hw-pic.c src/i2c-pic.c src/lcd.c src/main.c src/utils.c src/hw-avr.c src/i2c-avr.c src/locale.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/src/ds18b20.o ${OBJECTDIR}/src/ds1307.o ${OBJECTDIR}/src/hw-pic.o ${OBJECTDIR}/src/i2c-pic.o ${OBJECTDIR}/src/lcd.o ${OBJECTDIR}/src/main.o ${OBJECTDIR}/src/utils.o ${OBJECTDIR}/src/hw-avr.o ${OBJECTDIR}/src/i2c-avr.o ${OBJECTDIR}/src/locale.o
POSSIBLE_DEPFILES=${OBJECTDIR}/src/ds18b20.o.d ${OBJECTDIR}/src/ds1307.o.d ${OBJECTDIR}/src/hw-pic.o.d ${OBJECTDIR}/src/i2c-pic.o.d ${OBJECTDIR}/src/lcd.o.d ${OBJECTDIR}/src/main.o.d ${OBJECTDIR}/src/utils.o.d ${OBJECTDIR}/src/hw-avr.o.d ${OBJECTDIR}/src/i2c-avr.o.d ${OBJECTDIR}/src/locale.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/src/ds18b20.o ${OBJECTDIR}/src/ds1307.o ${OBJECTDIR}/src/hw-pic.o ${OBJECTDIR}/src/i2c-pic.o ${OBJECTDIR}/src/lcd.o ${OBJECTDIR}/src/main.o ${OBJECTDIR}/src/utils.o ${OBJECTDIR}/src/hw-avr.o ${OBJECTDIR}/src/i2c-avr.o ${OBJECTDIR}/src/locale.o

# Source Files
SOURCEFILES=src/ds18b20.c src/ds1307.c src/hw-pic.c src/i2c-pic.c src/lcd.c src/main.c src/utils.c src/hw-avr.c src/i2c-avr.c src/locale.c

# Pack Options 
PACK_COMPILER_OPTIONS=-I "${DFP_DIR}/include"
PACK_COMMON_OPTIONS=-B "${DFP_DIR}/gcc/dev/atmega328p"



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
Device=ATmega328P
ProjectDir="D:\_dev_\volksroutenrechner-ng"
ProjectName=volksroutenrechner-ng
ConfName=ATMega328P
ImagePath="dist\ATMega328P\${IMAGE_TYPE}\volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ImageDir="dist\ATMega328P\${IMAGE_TYPE}"
ImageName="volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IsDebug="true"
else
IsDebug="false"
endif

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-ATMega328P.mk dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
	@echo "--------------------------------------"
	@echo "User defined post-build step: [${MP_CC_DIR}\avr-size -C ${ImageDir}\${ProjectName}.${IMAGE_TYPE}.elf]"
	@${MP_CC_DIR}\avr-size -C ${ImageDir}\${ProjectName}.${IMAGE_TYPE}.elf
	@echo "--------------------------------------"

MP_PROCESSOR_OPTION=ATmega328P
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
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/src/ds18b20.o: src/ds18b20.c  .generated_files/flags/ATMega328P/a5c02c14329f6a707c39c14fd9b30f18bb73a281 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds18b20.o.d 
	@${RM} ${OBJECTDIR}/src/ds18b20.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/ds18b20.o.d" -MT "${OBJECTDIR}/src/ds18b20.o.d" -MT ${OBJECTDIR}/src/ds18b20.o  -o ${OBJECTDIR}/src/ds18b20.o src/ds18b20.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/ds1307.o: src/ds1307.c  .generated_files/flags/ATMega328P/485c9fc4ee66021a25e2661c9a9252bd9f97e6b9 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds1307.o.d 
	@${RM} ${OBJECTDIR}/src/ds1307.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/ds1307.o.d" -MT "${OBJECTDIR}/src/ds1307.o.d" -MT ${OBJECTDIR}/src/ds1307.o  -o ${OBJECTDIR}/src/ds1307.o src/ds1307.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/hw-pic.o: src/hw-pic.c  .generated_files/flags/ATMega328P/14ef8e5884e96f5a80e4437229966756ad1302f7 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-pic.o.d 
	@${RM} ${OBJECTDIR}/src/hw-pic.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/hw-pic.o.d" -MT "${OBJECTDIR}/src/hw-pic.o.d" -MT ${OBJECTDIR}/src/hw-pic.o  -o ${OBJECTDIR}/src/hw-pic.o src/hw-pic.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/i2c-pic.o: src/i2c-pic.c  .generated_files/flags/ATMega328P/56fd00ceae19378c847df7c818a736b0086837ef .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-pic.o.d 
	@${RM} ${OBJECTDIR}/src/i2c-pic.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/i2c-pic.o.d" -MT "${OBJECTDIR}/src/i2c-pic.o.d" -MT ${OBJECTDIR}/src/i2c-pic.o  -o ${OBJECTDIR}/src/i2c-pic.o src/i2c-pic.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/lcd.o: src/lcd.c  .generated_files/flags/ATMega328P/18ad6cb94e81e8055b2e29a05daf3659bcf5092a .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/lcd.o.d 
	@${RM} ${OBJECTDIR}/src/lcd.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/lcd.o.d" -MT "${OBJECTDIR}/src/lcd.o.d" -MT ${OBJECTDIR}/src/lcd.o  -o ${OBJECTDIR}/src/lcd.o src/lcd.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/main.o: src/main.c  .generated_files/flags/ATMega328P/1ff0fef13268de1066a9419c4a83f07b8bbf6cb8 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/main.o.d 
	@${RM} ${OBJECTDIR}/src/main.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/main.o.d" -MT "${OBJECTDIR}/src/main.o.d" -MT ${OBJECTDIR}/src/main.o  -o ${OBJECTDIR}/src/main.o src/main.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/utils.o: src/utils.c  .generated_files/flags/ATMega328P/f5fb6cbd2cc8a89e0935c2827f6da1d7c2aa61c4 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/utils.o.d 
	@${RM} ${OBJECTDIR}/src/utils.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/utils.o.d" -MT "${OBJECTDIR}/src/utils.o.d" -MT ${OBJECTDIR}/src/utils.o  -o ${OBJECTDIR}/src/utils.o src/utils.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/hw-avr.o: src/hw-avr.c  .generated_files/flags/ATMega328P/c2e33d982b695bb54cda881b0670b9e95fd7f0b9 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-avr.o.d 
	@${RM} ${OBJECTDIR}/src/hw-avr.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/hw-avr.o.d" -MT "${OBJECTDIR}/src/hw-avr.o.d" -MT ${OBJECTDIR}/src/hw-avr.o  -o ${OBJECTDIR}/src/hw-avr.o src/hw-avr.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/i2c-avr.o: src/i2c-avr.c  .generated_files/flags/ATMega328P/4e850b51a05349f136fbc35e0f64f1254fbe312e .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-avr.o.d 
	@${RM} ${OBJECTDIR}/src/i2c-avr.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/i2c-avr.o.d" -MT "${OBJECTDIR}/src/i2c-avr.o.d" -MT ${OBJECTDIR}/src/i2c-avr.o  -o ${OBJECTDIR}/src/i2c-avr.o src/i2c-avr.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/locale.o: src/locale.c  .generated_files/flags/ATMega328P/58f9a9b844bd5fa6f1559affd77f13791157c881 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/locale.o.d 
	@${RM} ${OBJECTDIR}/src/locale.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/locale.o.d" -MT "${OBJECTDIR}/src/locale.o.d" -MT ${OBJECTDIR}/src/locale.o  -o ${OBJECTDIR}/src/locale.o src/locale.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
else
${OBJECTDIR}/src/ds18b20.o: src/ds18b20.c  .generated_files/flags/ATMega328P/b6bdef661f0d1bfd838d19d1050c31b7f6ec319e .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds18b20.o.d 
	@${RM} ${OBJECTDIR}/src/ds18b20.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/ds18b20.o.d" -MT "${OBJECTDIR}/src/ds18b20.o.d" -MT ${OBJECTDIR}/src/ds18b20.o  -o ${OBJECTDIR}/src/ds18b20.o src/ds18b20.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/ds1307.o: src/ds1307.c  .generated_files/flags/ATMega328P/4b8fb898862efe8dbf0bce9a4a5bb436b517a782 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/ds1307.o.d 
	@${RM} ${OBJECTDIR}/src/ds1307.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/ds1307.o.d" -MT "${OBJECTDIR}/src/ds1307.o.d" -MT ${OBJECTDIR}/src/ds1307.o  -o ${OBJECTDIR}/src/ds1307.o src/ds1307.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/hw-pic.o: src/hw-pic.c  .generated_files/flags/ATMega328P/41c2b36bfcd4c2257ba89b900b70c966c6ea65df .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-pic.o.d 
	@${RM} ${OBJECTDIR}/src/hw-pic.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/hw-pic.o.d" -MT "${OBJECTDIR}/src/hw-pic.o.d" -MT ${OBJECTDIR}/src/hw-pic.o  -o ${OBJECTDIR}/src/hw-pic.o src/hw-pic.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/i2c-pic.o: src/i2c-pic.c  .generated_files/flags/ATMega328P/2a0a3fe3947c7ff55c1249b4cb164906892f9e3a .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-pic.o.d 
	@${RM} ${OBJECTDIR}/src/i2c-pic.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/i2c-pic.o.d" -MT "${OBJECTDIR}/src/i2c-pic.o.d" -MT ${OBJECTDIR}/src/i2c-pic.o  -o ${OBJECTDIR}/src/i2c-pic.o src/i2c-pic.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/lcd.o: src/lcd.c  .generated_files/flags/ATMega328P/b533db5a07d2e567cc3e7c72cffc11b887d04114 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/lcd.o.d 
	@${RM} ${OBJECTDIR}/src/lcd.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/lcd.o.d" -MT "${OBJECTDIR}/src/lcd.o.d" -MT ${OBJECTDIR}/src/lcd.o  -o ${OBJECTDIR}/src/lcd.o src/lcd.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/main.o: src/main.c  .generated_files/flags/ATMega328P/4e4a1f4d2c0b07964200a464474c874d1fa1e034 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/main.o.d 
	@${RM} ${OBJECTDIR}/src/main.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/main.o.d" -MT "${OBJECTDIR}/src/main.o.d" -MT ${OBJECTDIR}/src/main.o  -o ${OBJECTDIR}/src/main.o src/main.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/utils.o: src/utils.c  .generated_files/flags/ATMega328P/18c0bf91514dd92452e4b4b81e54045d22d40b00 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/utils.o.d 
	@${RM} ${OBJECTDIR}/src/utils.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/utils.o.d" -MT "${OBJECTDIR}/src/utils.o.d" -MT ${OBJECTDIR}/src/utils.o  -o ${OBJECTDIR}/src/utils.o src/utils.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/hw-avr.o: src/hw-avr.c  .generated_files/flags/ATMega328P/7b6b6e017a8634b1027c56ce16746ee66d6c2c02 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/hw-avr.o.d 
	@${RM} ${OBJECTDIR}/src/hw-avr.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/hw-avr.o.d" -MT "${OBJECTDIR}/src/hw-avr.o.d" -MT ${OBJECTDIR}/src/hw-avr.o  -o ${OBJECTDIR}/src/hw-avr.o src/hw-avr.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/i2c-avr.o: src/i2c-avr.c  .generated_files/flags/ATMega328P/5804a8e0e2120dc5aba2c55bc04e65d12e02d931 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/i2c-avr.o.d 
	@${RM} ${OBJECTDIR}/src/i2c-avr.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/i2c-avr.o.d" -MT "${OBJECTDIR}/src/i2c-avr.o.d" -MT ${OBJECTDIR}/src/i2c-avr.o  -o ${OBJECTDIR}/src/i2c-avr.o src/i2c-avr.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/src/locale.o: src/locale.c  .generated_files/flags/ATMega328P/15363c51fec4d053ba608e3534c272fd6d881fe6 .generated_files/flags/ATMega328P/a9b356ce7035b08538a425db1a3c50a0afa6c2d6
	@${MKDIR} "${OBJECTDIR}/src" 
	@${RM} ${OBJECTDIR}/src/locale.o.d 
	@${RM} ${OBJECTDIR}/src/locale.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"include" -funsigned-char -funsigned-bitfields -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -MD -MP -MF "${OBJECTDIR}/src/locale.o.d" -MT "${OBJECTDIR}/src/locale.o.d" -MT ${OBJECTDIR}/src/locale.o  -o ${OBJECTDIR}/src/locale.o src/locale.c  -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD) 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mmcu=atmega328p ${PACK_COMMON_OPTIONS}   -gdwarf-2 -D__$(MP_PROCESSOR_OPTION)__  -Wl,-Map="dist\${CND_CONF}\${IMAGE_TYPE}\volksroutenrechner-ng.${IMAGE_TYPE}.map"    -o dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}      -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1 -Wl,--gc-sections -Wl,--start-group  -Wl,-lm -Wl,--end-group 
	
	${MP_CC_DIR}\\avr-objcopy -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex "dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}" "dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.eep" || exit 0
	
	
	
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mmcu=atmega328p ${PACK_COMMON_OPTIONS}  -D__$(MP_PROCESSOR_OPTION)__  -Wl,-Map="dist\${CND_CONF}\${IMAGE_TYPE}\volksroutenrechner-ng.${IMAGE_TYPE}.map"    -o dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}      -DXPRJ_ATMega328P=$(CND_CONF)  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION) -Wl,--gc-sections -Wl,--start-group  -Wl,-lm -Wl,--end-group 
	${MP_CC_DIR}\\avr-objcopy -O ihex "dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}" "dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.hex"
	${MP_CC_DIR}\\avr-objcopy -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex "dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}" "dist/${CND_CONF}/${IMAGE_TYPE}/volksroutenrechner-ng.${IMAGE_TYPE}.eep" || exit 0
	
	
	
	
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
