# flash.mk —— 烧录与调试
SPEED ?= 1000
OCD_FLAGS = -f interface/cmsis-dap.cfg -f $(OCD_TARGET) \
	-c "adapter speed $(SPEED)"

# 板级定义覆盖;无 NRST 板给 reset_config none 
OCD_RESET ?= -c "reset_config srst_only srst_nogate connect_assert_srst"
RAM_RESET = $(ROOT)/build/ram_reset.gdb

.PHONY: flash openocd gdb gdb_ram
flash: $(TARGET).elf
	$(OPENOCD) $(OCD_FLAGS) $(OCD_RESET) \
		-c init -c "reset halt" \
		-c "flash write_image erase $(TARGET).elf" \
		-c "reset run" -c shutdown

openocd:
	$(OPENOCD) $(OCD_FLAGS) $(OCD_RESET)

gdb: $(TARGET).elf
	$(GDB) $(TARGET).elf -ex 'target remote :3333'

gdb_ram: $(TARGET)_ram.elf
	$(GDB) $(TARGET)_ram.elf -ex 'target remote :3333' \
		-ex 'monitor reset halt' \
		-ex load -x $(RAM_RESET) -ex ram_reset \
		-ex 'break main' -ex continue
