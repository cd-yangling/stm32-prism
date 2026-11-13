# f4 系列

MK   := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT := $(abspath $(dir $(MK))/../..)

SERIES     = f4
SERIES_UC  = F4
CPU_FLAGS  = -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
SELF_LL    =

include $(ROOT)/build/series/common.mk
