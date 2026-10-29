# boot —— 跳板小程序
MK    := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT  := $(abspath $(dir $(MK))/..)

CHIP ?= f103vet6
include $(ROOT)/build/chips/$(CHIP).mk
include $(ROOT)/build/offset.mk

ifneq (,$(filter-out clean,$(MAKECMDGOALS)))
ifeq ($(APP_OFFSET),0)
$(error APP_OFFSET=0: trampoline jumps back to itself (e.g. APP_OFFSET=0x20000))
endif
endif

ifeq ($(CHIP_SERIES),h7)
CPU_FLAGS = -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb
else
CPU_FLAGS = -mcpu=cortex-m3 -mthumb
endif

CROSS = arm-none-eabi-
CFLAGS = $(CPU_FLAGS) -ffreestanding -Os -Wall -std=gnu11 \
	-DAPP_OFFSET=$(APP_OFFSET) -DFLASH_SIZE=$(FLASH_SIZE) -DSRAM_SIZE=$(SRAM_SIZE) \
	-DCHIP_SERIES_$(shell echo $(CHIP_SERIES) | tr a-z A-Z)

all: boot.elf

boot.elf: startup.c boot.ld
	$(CROSS)gcc $(CFLAGS) -nostdlib -T boot.ld startup.c -o boot.elf \
		-Wl,--defsym=BOOT_LEN=$(SLOT_SIZE) \
		-Wl,--defsym=RAM_LEN=$(SRAM_SIZE) \
		-Wl,-Map=boot.map
	$(CROSS)size boot.elf

flash: boot.elf
	openocd -f interface/cmsis-dap.cfg -f $(OCD_TARGET) \
		-c "adapter speed $(SPEED)" -c init -c "reset halt" \
		-c "flash write_image erase boot.elf" -c "reset run" -c shutdown

SPEED ?= 1000
clean:
	rm -f boot.elf boot.map

.PHONY: all flash clean
