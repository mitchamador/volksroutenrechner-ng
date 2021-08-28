#
#  There exist several targets which are by default empty and which can be 
#  used for execution of your targets. These targets are usually executed 
#  before and after some main targets. They are: 
#
#     .build-pre:              called before 'build' target
#     .build-post:             called after 'build' target
#     .clean-pre:              called before 'clean' target
#     .clean-post:             called after 'clean' target
#     .clobber-pre:            called before 'clobber' target
#     .clobber-post:           called after 'clobber' target
#     .all-pre:                called before 'all' target
#     .all-post:               called after 'all' target
#     .help-pre:               called before 'help' target
#     .help-post:              called after 'help' target
#
#  Targets beginning with '.' are not intended to be called on their own.
#
#  Main targets can be executed directly, and they are:
#  
#     build                    build a specific configuration
#     clean                    remove built files from a configuration
#     clobber                  remove all built files
#     all                      build all configurations
#     help                     print help mesage
#  
#  Targets .build-impl, .clean-impl, .clobber-impl, .all-impl, and
#  .help-impl are implemented in nbproject/makefile-impl.mk.
#
#  Available make variables:
#
#     CND_BASEDIR                base directory for relative paths
#     CND_DISTDIR                default top distribution directory (build artifacts)
#     CND_BUILDDIR               default top build directory (object files, ...)
#     CONF                       name of current configuration
#     CND_ARTIFACT_DIR_${CONF}   directory of build artifact (current configuration)
#     CND_ARTIFACT_NAME_${CONF}  name of build artifact (current configuration)
#     CND_ARTIFACT_PATH_${CONF}  path to build artifact (current configuration)
#     CND_PACKAGE_DIR_${CONF}    directory of package (current configuration)
#     CND_PACKAGE_NAME_${CONF}   name of package (current configuration)
#     CND_PACKAGE_PATH_${CONF}   path to package (current configuration)
#
# NOCDDL


# Environment 
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib


# build
build: .build-post

.build-pre:
# Add your pre 'build' code here...

.build-post: .build-impl
# Add your post 'build' code here...
ifneq (${EEPROM},)
.build-post: .eeprom_${CONF}
endif
ifneq (${RELEASE},)
.build-post: .release_${CONF}
endif

# clean
clean: .clean-post

.clean-pre:
# Add your pre 'clean' code here...
# WARNING: the IDE does not call this target since it takes a long time to
# simply run make. Instead, the IDE removes the configuration directories
# under build and dist directly without calling make.
# This target is left here so people can do a clean when running a clean
# outside the IDE.

.clean-post: .clean-impl
# Add your post 'clean' code here...


# clobber
clobber: .clobber-post

.clobber-pre:
# Add your pre 'clobber' code here...

.clobber-post: .clobber-impl
# Add your post 'clobber' code here...


# all
all: .all-post

.all-pre:
# Add your pre 'all' code here...

.all-post: .all-impl
# Add your post 'all' code here...


# help
help: .help-post

.help-pre:
# Add your pre 'help' code here...

.help-post: .help-impl
# Add your post 'help' code here...

# include project implementation makefile
include nbproject/Makefile-impl.mk

# include project make variables
include nbproject/Makefile-variables.mk

include nbproject/Makefile-local-${CONF}.mk

.eeprom_PIC16F876A_legacy:
	${MP_CC_DIR}/avr-objcopy -b 0 -i 2 -O binary -I elf32-little -j eeprom_data ${CND_ARTIFACT_DIR_${CONF}}/${PROJECTNAME}.production.elf proteus/eeprom_16F876A.bin

.release_PIC16F876A_legacy:
	${CP} ${CND_ARTIFACT_PATH_${CONF}} firmware/${PROJECTNAME}.pic16f876a.hex

.eeprom_ATMega328P:
	${MP_CC_DIR}/avr-objcopy -O binary -I elf32-little -j .eeprom ${CND_ARTIFACT_DIR_${CONF}}/${PROJECTNAME}.production.elf proteus/eeprom_ATmega328P.bin

.release_ATMega328P:
	${CP} ${CND_ARTIFACT_PATH_${CONF}} firmware/${PROJECTNAME}.atmega328p.hex
	${CP} ${CND_ARTIFACT_DIR_${CONF}}/${PROJECTNAME}.production.eep firmware/${PROJECTNAME}.atmega328p.eep

version:
	@version.cmd

eeprom:
	${MAKE} EEPROM=1 all

release: version
	${MAKE} RELEASE=1 clobber all
