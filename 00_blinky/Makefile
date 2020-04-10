# Makefile for ESP8266 projects

# Output directors to store intermediate compiled files
# relative to the project directory
BUILD_BASE	= build
FW_BASE		= firmware
DUMP_BASE       = dump

# base directory for the compiler
XTENSA_TOOLS_ROOT ?= $(HOME)/x-tools/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(HOME)/x-tools/sdk

# esptool.py path and port
ESPTOOL		?= esptool.py
ESPPORT		?= /dev/ttyUSB0

# the baud rate is limited to 115200 when esptool.py establishes the
# initial connection, higher speeds are only used for data transfers
ESPTOOL_BAUD    ?= 115200

##############################################
# My ESP8266 AT+RST output:                  #
##############################################
#   SPI Speed      : 40MHz                   #
#   SPI Mode       : QIO                     #
#   SPI Flash Size & Map: 8Mbit(512KB+512KB) #
##############################################
#
# In qio mode, two additional GPIOs (9 and 10) are used for SPI flash communications
# If flash mode is set to dio then these pins are available for other purposes
# see: https://github.com/espressif/esptool#flash-mode---flash_mode--fm
#      https://github.com/espressif/esptool/wiki/SPI-Flash-Modes
#
# The default --flash_size parameter is detect, which tries to
# autodetect size based on SPI flash ID. If detection fails,
# a warning is printed and a default value of 4MB (4 megabytes) is used
# see: https://github.com/espressif/esptool#flash-size---flash_size--fs
#      https://github.com/espressif/esptool#esp8266-and-flash-size
ESPTOOL_FLASH_OPTIONS = --flash_freq 40m --flash_mode qio --flash_size 8m

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
MODULES		= driver user
EXTRA_INCDIR    = include

# libraries used in this project, mainly provided by the SDK
LIBS		= c gcc pp phy net80211 lwip wpa main # hal

# compiler flags using during compilation of source files
CFLAGS		= -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH
# -std=gnu90

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# we create two different files for uploading into the flash
# these are the names and options to generate them
FW_FILE_1_ADDR	= 0x00000
FW_FILE_2_ADDR	= 0x10000

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc



####
#### no user configurable options below here
####
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))

INCDIR	        := $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

FW_FILE_1	:= $(addprefix $(FW_BASE)/,$(FW_FILE_1_ADDR).bin)
FW_FILE_2	:= $(addprefix $(FW_BASE)/,$(FW_FILE_2_ADDR).bin)

DUMP_FILE_1	:= $(addprefix $(DUMP_BASE)/,$(FW_FILE_1_ADDR).bin)
DUMP_FILE_2	:= $(addprefix $(DUMP_BASE)/,$(FW_FILE_1_ADDR).bin)

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS) -c $$< -o $$@
endef

.PHONY: all checkdirs flash clean

all: checkdirs $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)

$(FW_BASE)/%.bin: $(TARGET_OUT) | $(FW_BASE)
	$(vecho) "FW $(FW_BASE)/"
	$(Q) $(ESPTOOL) elf2image -o $(FW_BASE)/ $(TARGET_OUT)

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

$(FW_BASE):
	$(Q) mkdir -p $@

$(DUMP_BASE):
	$(Q) mkdir -p $@

flash: $(FW_FILE_1) $(FW_FILE_2)
	$(ESPTOOL) --port $(ESPPORT) --baud $(ESPTOOL_BAUD) write_flash $(ESPTOOL_FLASH_OPTIONS) $(FW_FILE_1_ADDR) $(FW_FILE_1) $(FW_FILE_2_ADDR) $(FW_FILE_2)

read: $(DUMP_BASE)
	$(ESPTOOL) --port $(ESPPORT) --baud $(ESPTOOL_BAUD) read_flash $(FW_FILE_1_ADDR) $(FW_FILE_2_ADDR) $(DUMP_FILE_1)

clean:
	$(Q) rm -rf $(FW_BASE) $(DUMP_BASE) $(BUILD_BASE)

flashid:
	$(ESPTOOL) --port $(ESPPORT) flash_id

readmac:
	$(ESPTOOL) --port $(ESPPORT) read_mac

imageinfo: $(FW_FILE_1)
	$(ESPTOOL) image_info $(FW_FILE_1)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
