USE_LIBC   ?= attolibc
LIBC_VALID = attolibc
BAD_LIBC = $(filter-out $(LIBC_VALID),$(USE_LIBC))
ifneq ($(strip $(BAD_LIBC)),)
$(error invalid USE_LIBC [$(strip $(BAD_LIBC))]; valid: $(LIBC_VALID))
endif
LIBC_INC = -I$(NEXUS)/lib/$(USE_LIBC)
LIBC_C   = $(wildcard $(NEXUS)/lib/$(USE_LIBC)/*.c)
