# stm32-prism

参照 Zephyr 的分层抽象，实现 STM32 抽象架构设计，目标覆盖 STM32 全系列。

STM32Cube LL / HAL 固件包通过只读 bind-mount 引入，不含在仓库内；`startup.s`
与 linker script 全部手写，不调用 `SystemInit`。

可以同时使用 LL / HAL 编程,根据需要使用 USE_HAL_<XXXX> 或者 USE_LL_<XXXX> 详细
参考build目录的文档说明

---

## 结构

```
stm32-prism/
├── nexus/
│   ├── pal/       # 外设抽象契约（只有接口，无芯片细节）
│   ├── dev/       # 器件驱动，只依赖 pal 契约
│   ├── subsys/    # 消费子系统：console / gfx / input
│   └── lib/
│       ├── attolibc/   # 自写 libc （-nostdinc 洁净编译用）
│       └── gadget/     # fifo / imu_math / fixtoa 等
├── soc/
│   ├── f1/        # startup.s、linker.ld、clock.c、soc_init.c、LL drv 实现
│   └── h7/        # 尚未开发
├── boot/          # 磨损跳板，独立 Makefile
├── examples/
│   ├── 00-blink/
│   ├── 01-uart/
│   ├── 02-i2c/
│   ├── 03-spi/
│   ├── 04-input/
│   ├── 05-pwm/
│   ├── 06-enc/
│   ├── 07-adc/
│   └── bluepill_f103c8t6/
├── apps/
│   └── yahboom_sbr/          # Yahboom 平衡小车
├── build/         # GNU Make 构建系统
└── third_party/ST/STM32Cube/ # 只读 mount，永不写入
```

分层方向：`apps / examples` → `subsys` → `dev` → `pal` ← `soc/drv`（LL 实现）。
芯片细节全部收敛在 `soc/` 里，`nexus/` 其余件只认 pal 接口。

---

## 构建

依赖：`arm-none-eabi-gcc`、`openocd`、`gdb-multiarch`。

先挂载固件包：

```bash
sudo build/mount-ro.sh <CubeF1 源目录> third_party/ST/STM32Cube/CubeF1
```

进任意 example 目录 `make BOARD=<板名>` 即可。新 example 的 Makefile 只需三行：

```make
BOARD    = bluepill_f103c8t6
PROJ_SRC = board.c main.c
include ../../build/entry.mk
```

常用变量：

| 变量 | 说明 |
|---|---|
| `BOARD` | 必填，见下表 |
| `BOARD_CONF` | 追加 `CONFIG_*` 开关 |
| `USE_LL` / `USE_HAL` | 按需选通 Cube 模块（写文件名尾巴，如 `rcc dma`） |
| `PROJ_DEFS` | 追加 `-D` 宏 |
| `APP_OFFSET` | Flash 槽位偏移（磨损轮换） |
| `V=1` | 显示完整编译命令 |

常用目标：

```bash
make              # firmware.elf + firmware_ram.elf + 内存报告
make gdb_ram      # RAM 调试，不碰 Flash
make openocd      # 启动 OpenOCD
make cc           # 生成 compile_commands.json
make vimdb        # 生成 tags + cscope
```

已支持开发板：

| BOARD | 芯片 |
|---|---|
| `bluepill_f103c8t6` | STM32F103C8T6 |
| `embedfire_zhinanzhe` | STM32F103 |
| `embedfire_h7pro` | STM32H7xx |
| `yahboom_sbr` | STM32F103 |

---

## 许可

MIT。STM32Cube 固件包不含在本仓库中，版权归 STMicroelectronics 所有（BSD-3-Clause）。
