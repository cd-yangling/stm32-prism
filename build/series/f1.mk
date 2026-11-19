# f1 系列

MK   := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT := $(abspath $(dir $(MK))/../..)

SERIES     = f1
SERIES_UC  = F1
CPU_FLAGS  = -mcpu=cortex-m3 -mthumb
SELF_LL    = utils rcc

include $(ROOT)/build/series/common.mk
