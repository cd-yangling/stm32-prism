HOSTCC     ?= cc
CLKGEN_BIN  = $(BUILD_DIR)/clkgen
CLKGEN_SRC  = $(ROOT)/build/clkgen/clkgen.c
CLOCK_HDR   = $(BUILD_DIR)/clock_conf.h

$(CLKGEN_BIN): $(CLKGEN_SRC)
	$(Q)mkdir -p $(BUILD_DIR)
	$(Q)$(HOSTCC) -O2 -Wall -Wextra -Werror -o $@ $(CLKGEN_SRC)

$(CLOCK_HDR): $(CLKGEN_BIN) $(ROOT)/build/boards/$(BOARD).mk \
		$(ROOT)/build/chips/$(CHIP).mk \
		$(ROOT)/build/series/$(CHIP_SERIES).mk FORCE
	$(MSG) "  CLKGEN  clock_conf.h"
	$(Q)$(CLKGEN_BIN) $(SERIES) $(DEV_INC) '$(CHIP_DEFS)' '$(HSE_FREQ)' '$(SYSCLK_HZ)' $@.tmp
	$(Q)if cmp -s $@.tmp $@; then rm -f $@.tmp; else mv -f $@.tmp $@; fi

.PHONY: FORCE
FORCE:

$(filter $(BUILD_DIR)/%,$(OBJS)): $(CLOCK_HDR)
