# h7 系列

MK   := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT := $(abspath $(dir $(MK))/../..)

SERIES     = h7
SERIES_UC  = H7
CPU_FLAGS  = -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb
SELF_LL    =

include $(ROOT)/build/series/common.mk
