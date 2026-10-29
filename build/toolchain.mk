# toolchain.mk —— 与芯片无关的构建规则
# 前置:CPU_FLAGS | PROJ_DEFS | INCLUDES | C_SOURCES | ASM_SOURCES | LINKER  变量须在 include 前配置
CROSS     = arm-none-eabi-
CC        = $(CROSS)gcc
OBJCOPY   = $(CROSS)objcopy
SIZE      = $(CROSS)size
GDB       = $(CROSS)gdb
OPENOCD   = openocd

BUILD_DIR = objs
TARGET    = firmware

null  :=
space := $(null) $(null)
$(info $(space)$(space)BOARD   $(BOARD)($(BOARD_NAME)))

DEFS   = $(CHIP_DEFS) $(BOARD_DEFS) $(SERIES_DEFS) $(PROJ_DEFS)
WARN_STRICT = -Wextra -Werror
CFLAGS = $(CPU_FLAGS) $(DEFS) $(INCLUDES) \
         -nostdinc -ffreestanding -Og -g3 -Wall -std=gnu11 \
         $(WARN_STRICT) \
         -ffunction-sections -fdata-sections \
         $(EXTRA_CFLAGS) -MMD -MP
ASFLAGS = $(CPU_FLAGS) $(DEFS) $(INCLUDES) -nostdinc

LDFLAGS = $(CPU_FLAGS) -nostdlib -Wl,--gc-sections
LDLIBS  = -lgcc

ifeq ($(APP_OFFSET),0)
APP_LENGTH = $(FLASH_SIZE)
else
APP_LENGTH = $(SLOT_SIZE)
endif

# 源 → objs/ 镜像源目录树
C_SOURCES   := $(abspath $(C_SOURCES))
ASM_SOURCES := $(abspath $(ASM_SOURCES))
vpath %.c $(ROOT)
vpath %.s $(ROOT)
OBJS = $(addprefix $(BUILD_DIR)/,$(patsubst $(ROOT)/%,%,$(ASM_SOURCES:.s=.o) $(C_SOURCES:.c=.o)))
OBJS += $(PROJ_OBJS)

ifneq ($(filter /%,$(patsubst $(ROOT)/%,%,$(ASM_SOURCES) $(C_SOURCES))),)
$(error source outside ROOT: $(filter /%,$(patsubst $(ROOT)/%,%,$(ASM_SOURCES) $(C_SOURCES))); PROJ_SRC must be inside ROOT or relative)
endif

$(filter $(BUILD_DIR)/%,$(OBJS)): $(MAKEFILE_LIST)

ifeq ($(V),1)
Q   :=
MSG := @true
else
Q   := @
MSG := @echo
endif

.PHONY: all clean help

.DEFAULT_GOAL := all
all: $(TARGET).elf $(TARGET)_ram.elf

$(TARGET).elf: $(OBJS) $(LINKER) $(MAKEFILE_LIST)
	$(MSG) "  LD      $(TARGET).elf"
	$(Q)$(CC) $(LDFLAGS) -T $(LINKER) $(OBJS) $(LDLIBS) -o $@ \
		-Wl,--defsym=APP_OFFSET=$(APP_OFFSET) \
		-Wl,--defsym=APP_LENGTH=$(APP_LENGTH) \
		-Wl,--defsym=RAM_LEN=$(SRAM_SIZE) \
		-Wl,-Map=$(TARGET).map
	$(call size_report,$@,1)

$(TARGET)_ram.elf: $(OBJS) $(LINKER_RAM) $(MAKEFILE_LIST)
	$(MSG) "  LD      $(TARGET)_ram.elf"
	$(Q)$(CC) $(LDFLAGS) -Wl,--no-warn-rwx-segments \
		-T $(LINKER_RAM) $(OBJS) $(LDLIBS) -o $@ \
		-Wl,--defsym=RAM_LEN=$(SRAM_SIZE) \
		-Wl,-Map=$(TARGET)_ram.map
	$(call size_report,$@,0)

$(BUILD_DIR)/%.o: %.c
	$(Q)mkdir -p $(dir $@)
	$(MSG) "  CC      $*.o"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/third_party/%.o: WARN_STRICT =

$(BUILD_DIR)/%.o: %.s
	$(Q)mkdir -p $(dir $@)
	$(MSG) "  AS      $*.o"
	$(Q)$(CC) $(ASFLAGS) -c $< -o $@

FLASH_LO := $(shell printf '%d' 0x08000000)
FLASH_HI := $(shell printf '%d' $$((0x08000000 + $(FLASH_SIZE))))
FLASH_TOT := $(shell printf '%d' $(APP_LENGTH))
RAM_LO   := $(shell printf '%d' 0x20000000)
RAM_HI   := $(shell printf '%d' $$((0x20000000 + $(SRAM_SIZE))))
RAM_TOT  := $(shell printf '%d' $(SRAM_SIZE))
size_report = $(Q)$(SIZE) -A $(1) | awk -v flo=$(FLASH_LO) -v fhi=$(FLASH_HI) \
	-v rlo=$(RAM_LO) -v rhi=$(RAM_HI) -v ftot=$(FLASH_TOT) -v rtot=$(RAM_TOT) -v cnt=$(2) \
	'NR>2 { if ($$3>=flo && $$3<fhi) f+=$$2; if ($$3>=rlo && $$3<rhi) { r+=$$2; if (cnt && $$1 !~ /\.(bss|noinit)/) f+=$$2 } } \
	END { if (cnt) printf "\tFLASH %8d / %8d B (%5.1f%%)\n", f, ftot, f*100/ftot; \
	printf "\tRAM   %8d / %8d B (%5.1f%%)\n", r, rtot, r*100/rtot }'

clean:
	$(Q)rm -rf $(BUILD_DIR) $(TARGET).elf $(TARGET)_ram.elf $(TARGET).map $(TARGET)_ram.map

help:
	@echo "BOARD = $(BOARD)($(BOARD_NAME))   APP_OFFSET = $(APP_OFFSET)   TARGET = $(TARGET)"
	@echo "make            firmware.elf + firmware_ram.elf + map + size summary"
	@echo "make flash      flash firmware to target"
	@echo "make openocd    start openocd"
	@echo "make gdb        gdb session on flash image"
	@echo "make gdb_ram    gdb session on RAM image"
	@echo "make vimdb      tags + cscope"
	@echo "make cc         compile_commands.json"
	@echo "make V=1        print full command lines"
	@echo "make clean      remove build artifacts"
	@echo "make distclean  clean + tags/cscope/compile_commands"
	@echo "LL  valid: $(LL_VALID)"
	@echo "HAL valid: $(HAL_VALID)"

-include $(OBJS:.o=.d)
