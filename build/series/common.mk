# series/common.mk —— 系列层

include $(ROOT)/build/offset.mk
include $(ROOT)/build/libc.mk

DEV_UC  = $(SERIES_UC)xx
DEV_TAG = $(SERIES)xx
NEXUS  = $(ROOT)/nexus
CSER    = $(ROOT)/soc/$(SERIES)

CUBE        = $(ROOT)/third_party/ST/STM32Cube/STM32Cube$(SERIES_UC)
CMSIS_INC   = $(CUBE)/Drivers/CMSIS/Include
DEV_INC     = $(CUBE)/Drivers/CMSIS/Device/ST/STM32$(DEV_UC)/Include
DRV_INC     = $(CUBE)/Drivers/STM32$(DEV_UC)_HAL_Driver/Inc
DRV_SRC     = $(CUBE)/Drivers/STM32$(DEV_UC)_HAL_Driver/Src
SYSTEM_SRC  = $(CUBE)/Drivers/CMSIS/Device/ST/STM32$(DEV_UC)/Source/Templates/system_stm32$(DEV_TAG).c

STARTUP_SRC = $(CSER)/startup.s
LINKER      = $(CSER)/linker.ld
LINKER_RAM  = $(CSER)/linker_ram.ld

# 可用模块从 Cube 目录派生;LL_HDR_ONLY = 纯头内联(无 .c)
LL_ALL      = $(patsubst $(DRV_SRC)/stm32$(DEV_TAG)_ll_%.c,%,$(wildcard $(DRV_SRC)/stm32$(DEV_TAG)_ll_*.c))
HAL_ALL     = $(patsubst $(DRV_SRC)/stm32$(DEV_TAG)_hal_%.c,%,$(wildcard $(DRV_SRC)/stm32$(DEV_TAG)_hal_*.c))
LL_HDR_ONLY = bus
LL_VALID    = $(LL_ALL) $(LL_HDR_ONLY)
HAL_VALID   = $(HAL_ALL)

ifeq ($(wildcard $(DRV_SRC)/stm32$(DEV_TAG)_hal.c),)
$(error Cube$(SERIES_UC) not mounted? run: sudo build/mount-ro.sh <src> third_party/ST/STM32Cube/STM32Cube$(SERIES_UC))
endif

BAD = $(filter-out $(LL_VALID),$(USE_LL)) $(filter-out $(HAL_VALID),$(USE_HAL))
ifneq ($(strip $(BAD)),)
$(error invalid module [$(strip $(BAD))]; LL: $(LL_VALID); HAL: $(HAL_VALID))
endif

HAL_BASE    = cortex rcc rcc_ex gpio dma
HAL_DEP_sd  = dma
HAL_MAIN    = $(if $(USE_HAL),$(DRV_SRC)/stm32$(DEV_TAG)_hal.c)
HAL_LIST    = $(if $(USE_HAL),$(sort $(HAL_BASE) $(USE_HAL) $(foreach m,$(USE_HAL),$(HAL_DEP_$(m)))))
LL_SRC_LIST = $(sort $(filter-out $(LL_HDR_ONLY),$(USE_LL) $(SELF_LL)))

DRV_C = $(HAL_MAIN) $(LL_SRC_LIST:%=$(DRV_SRC)/stm32$(DEV_TAG)_ll_%.c) \
        $(HAL_LIST:%=$(DRV_SRC)/stm32$(DEV_TAG)_hal_%.c)

# STM32<系列宏> 供 tick.c 跨系列判头(为 HAL_IncTick / HAL_GetTick 服务)
SERIES_DEFS = -DUSE_FULL_LL_DRIVER -DSTM32$(SERIES_UC)
ifneq ($(USE_HAL),)
SERIES_DEFS += -DUSE_HAL_DRIVER
endif
ifneq ($(SYSCLK_HZ),)
SERIES_DEFS += -DSYSCLK_PLL_MUL=$(PLL_MUL) -DSYSCLK_HZ=$(SYSCLK_HZ)
endif

SELF_C = $(wildcard $(NEXUS)/*.c $(NEXUS)/pal/*.c \
                   $(NEXUS)/dev/*.c $(NEXUS)/subsys/*.c $(NEXUS)/lib/gadget/*.c \
                   $(CSER)/*.c $(CSER)/drv/*.c) \
         $(LIBC_C)
C_SOURCES   = $(SYSTEM_SRC) $(DRV_C) $(PROJ_SRC) $(SELF_C)
ASM_SOURCES = $(STARTUP_SRC)

INCLUDES    = -I$(CURDIR) -I$(NEXUS) -I$(CSER) $(LIBC_INC) \
	-I$(DRV_INC) -I$(DEV_INC) -I$(CMSIS_INC)
VIMDB_DIRS  = $(NEXUS) $(CSER) $(CMSIS_INC) $(DEV_INC) $(DRV_INC) $(DRV_SRC)

include $(ROOT)/build/toolchain.mk
include $(ROOT)/build/flash.mk
include $(ROOT)/build/vimdb.mk
