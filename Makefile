include config.mk

MODULES=app lib/lpc_chip_43xx lib/lpc_board_ciaa_edu_4337
DEFINES=CORE_M4 __USE_LPCOPEN __USE_NEWLIB

SRC=$(foreach m, $(MODULES), $(wildcard $(m)/src/*.c))
ASRC=$(foreach m, $(MODULES), $(wildcard $(m)/src/*.S))
OUT=out
INCLUDES=$(foreach m, $(MODULES), -I$(m)/inc)
_DEFINES=$(foreach m, $(DEFINES), -D$(m))
OBJECTS=$(addprefix $(OUT)/, $(SRC:.c=.o) $(ASRC:.S=.o))
DEPS=$(SRC:.c=.d)
LDSCRIPT=lib/ciaa_lpc4337.ld

ARCH_FLAGS=-mcpu=cortex-m4 -mthumb

TARGET=$(OUT)/$(APP).elf
TARGET_BIN=$(basename $(TARGET)).bin
TARGET_LST=$(basename $(TARGET)).lst
TARGET_MAP=$(basename $(TARGET)).map

ifeq ($(USE_FPU),y)
ARCH_FLAGS+=-mfloat-abi=hard -mfpu=fpv4-sp-d16
endif

CFLAGS=$(ARCH_FLAGS) $(INCLUDES) $(_DEFINES) -ggdb3 -O$(OPT) -ffunction-sections -fdata-sections
LDFLAGS=$(ARCH_FLAGS) -T$(LDSCRIPT) -nostartfiles -Wl,-gc-sections -Wl,-Map=$(TARGET_MAP) -Wl,--cref

ifeq ($(USE_NANO),y)
LDFLAGS+=--specs=nano.specs
endif

ifeq ($(SEMIHOST),y)
LDFLAGS+=--specs=rdimon.specs
endif

CROSS=arm-none-eabi-
CC=$(CROSS)gcc
AS=$(CROSS)gcc
LD=$(CROSS)gcc
SIZE=$(CROSS)size
LIST=$(CROSS)objdump -xdS
OBJCOPY=$(CROSS)objcopy
GDB=$(CROSS)gdb
OOCD=openocd

OOCD_SCRIPT?=lib/ciaa-nxp.cfg

ifeq ($(VERBOSE),y)
Q=
else
Q=@
endif

all: $(TARGET) $(TARGET_BIN) $(TARGET_LST) size

help:
	@echo $(OBJECTS)

-include $(DEPS)

$(OUT)/%.o: %.c
	@echo CC $<
	@mkdir -p $(dir $@)
	$(Q)$(CC) -MMD $(CFLAGS) -c -o $@ $<

$(OUT)/%.o: %.S
	@echo AS $<
	@mkdir -p $(dir $@)
	$(Q)$(AS) -MMD $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	@echo LD $@
	@mkdir -p $(dir $@)
	$(Q)$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

$(TARGET_BIN): $(TARGET)
	@echo BIN
	@mkdir -p $(dir $@)
	$(Q)$(OBJCOPY) -v -O binary $< $@

$(TARGET_LST): $(TARGET)
	@echo LIST
	@mkdir -p $(dir $@)
	$(Q)$(LIST) $< > $@

size: $(TARGET)
	$(Q)$(SIZE) $<

program: $(TARGET_BIN)
	@echo PROG
	$(Q)$(OOCD) -f $(OOCD_SCRIPT) \
		-c "init" \
		-c "halt 0" \
		-c "flash write_image erase unlock $< 0x1A000000 bin" \
		-c "reset run" \
		-c "shutdown" 2>&1

debug: $(TARGET_BIN)
	@echo DEBUG
	$(Q)$(OOCD) -f $(OOCD_SCRIPT) 2>&1
	
clean:
	@echo CLEAN
	$(Q)rm -fR $(OUT)

.PHONY: all size clean program
