# entry.mk —— stub 唯一入口,系列由 BOARD 推导:
#   BOARD      = embedfire_zhinanzhe
#   BOARD_CONF = -DCONFIG_USART1 -DCONFIG_LED   (可选)
#   USE_LL     = rcc bus        (可选)
#   USE_HAL    = rcc bus        (可选)
#   include ../../build/entry.mk

MK   := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT := $(abspath $(dir $(MK))/..)

ifeq ($(BOARD),)
$(error BOARD empty; set BOARD = <name> in stub (see build/boards/) or pass make BOARD=<name>)
endif

# BOARD → CHIP → CHIP_SERIES
include $(ROOT)/build/boards/$(BOARD).mk
include $(ROOT)/build/chips/$(CHIP).mk
include $(ROOT)/build/series/$(CHIP_SERIES).mk
