# smoke —— 构建链自检,参数全走命令行:
#   make BOARD=embedfire_h7pro                # h7 裸链(空 main,零 Cube 驱动)
#   make BOARD=embedfire_zhinanzhe            # f1 裸链(缺省)
#   make BOARD=embedfire_zhinanzhe USE_LL=rcc # f1 LL 链
#   make BOARD=embedfire_h7pro USE_HAL=gpio   # h7 HAL 链(base 集全带)
BOARD ?= embedfire_zhinanzhe
PROJ_SRC = main.c board.c
include ../entry.mk
