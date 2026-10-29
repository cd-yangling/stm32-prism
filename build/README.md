# build/ —— 构建系统

工程采用基于 GNU Make 的多级模块化架构。每个示例工程（example）的 Makefile 只需声明目标开发板 `BOARD` 并包含 `build/entry.mk`，构建系统会自动按 **开发板（Board） -> 芯片（Chip） -> 架构系列（Series）** 的顺序层叠加载配置。

## 配置文件加载顺序

```
example/Makefile (指定 BOARD)
└─ entry.mk
   ├─ boards/$(BOARD).mk         (板级配置：芯片型号、外部晶振 HSE、板级定义)
   ├─ chips/$(CHIP).mk           (芯片配置：所属系列、内存/Flash容量、OpenOCD Target)
   └─ series/$(CHIP_SERIES).mk   (系列配置：内核指令集、CPU_FLAGS)
      │
      └─ series/common.mk        (通用配置汇聚)
         │
         ├─ offset.mk            (App 偏移与分区逻辑)
         ├─ libc.mk              (C 标准库源码选择)
         ├─ toolchain.mk         (交叉编译工具链、编译/链接选项及 Target 规则)
         ├─ flash.mk             (OpenOCD Flash 烧录与 GDB 调试目标)
         └─ vimdb.mk             (代码索引 tag 与 compile_commands.json 生成)
```

## 其他模块说明

- `boot/Makefile`：独立 Bootloader 构建逻辑。跳过 `series/common.mk`，仅包含 `chips/$(CHIP).mk` 与 `offset.mk`，由 `CHIP_SERIES` 直接推导 `CPU_FLAGS`。
- `smoke/`：构建系统本身的冒烟测试用例，用于验证多芯片/板级组合配置的正确性。
- `ram_reset.gdb`：用于 GDB 调试 RAM 版本固件的初始化脚本（重置 PC 与 SP 指针）。
- `mount-ro.sh`：将第三方依赖库 (`third_party`) 以 bind-mount 方式挂载为只读，避免对依赖代码造成非预期修改，替代传统的 `git submodule` 管理方式。

## 示例 Makefile 接口与变量表

### 用户工程可配置变量（Example 接口）

| 类别 | 变量 | 属性 | 默认值 | 说明 |
|---|---|---|---|---|
| **核心配置** | `BOARD` | **必填** | - | 目标开发板标识（如 `bluepill_f103c8t6`），触发链式加载 |
| | `BOARD_CONF` | 可选 | 空 | 工程启用的 `CONFIG_*` 模块功能开关（追加至板级 `BOARD_DEFS`） |
| **驱动与库** | `USE_LL` | 可选 | 空 | 按需引入 STM32 Cube LL 库模块（按文件名后缀选通，如 `rcc dma`） |
| | `USE_HAL` | 可选 | 空 | 按需引入 STM32 Cube HAL 库模块 |
| | `USE_LIBC` | 可选 | `attolibc` | C 标准库选源（目前仅支持 `attolibc`） |
| **源码与选项** | `PROJ_SRC` | 可选 | 空 | 追加的 `.c` 源码路径（相对于工程目录，如 `board.c main.c`） |
| | `PROJ_OBJS` | 可选 | 空 | 引入自备 `.o` 预编译文件（不进依赖追踪） |
| | `PROJ_DEFS` | 可选 | 空 | 追加的 `-D` 宏（C/汇编同生效） |
| | `EXTRA_CFLAGS` | 可选 | 空 | 追加的特定 C 编译器标志 |


### 系统内部与命令行控制变量

> **注意**：标有 **[内部只读]** 的变量属于底层根据 `BOARD` 自动推导的系统变量，**严禁在工程 Makefile 中直接修改或覆盖**，否则会破坏 `开发板 -> 芯片 -> 系列` 的自动链式加载机制。

| 类别 | 变量 | 属性 | 来源 / 覆盖方式 | 默认值 | 说明 |
|---|---|---|---|---|---|
| **板级配置** | `CHIP` | **[内部只读]** | `boards/*.mk` | - | 开发板对应芯片，严禁工程直接指定 |
| | `HSE_FREQ` | **[内部只读]** | `boards/*.mk` | - | 板载晶振频率，自动展开为 `-DHSE_VALUE` |
| | `BOARD_DEFS` | **[内部只读]** | `boards/*.mk` | - | 板级编译定义（自动合并 `BOARD_CONF`） |
| **芯片配置** | `CHIP_SERIES` | **[内部只读]** | `chips/*.mk` | - | 芯片架构系列（如 `f1`/`h7`），**严禁直接传递** |
| | `CHIP_DEFS` | **[内部只读]** | `chips/*.mk` | - | 芯片级宏定义（芯片型号、外设 Pin 掩码） |
| **内存与分区** | `FLASH_SIZE` / `SRAM_SIZE` | **[内部只读]** | `chips/*.mk` | - | Flash/SRAM 物理容量，供链接脚本计算 |
| | `SLOT_SIZE` | **[内部只读]** | `chips/*.mk` | - | 磨损平衡槽位尺寸上限 |
| | `APP_OFFSET` | 可覆盖 | 命令行 / Makefile | `0` | 固件 Flash 槽位偏移（命令行 `make APP_OFFSET=...` 指定） |
| **调试与构建** | `OCD_TARGET` | **[内部只读]** | `chips/*.mk` | - | OpenOCD target 配置文件名 |
| | `OCD_RESET` | 可覆盖 | `boards/*.mk` / Makefile | `硬件复位 (srst)` | OpenOCD 复位策略 |
| | `SPEED` | 可覆盖 | 命令行 / Makefile | `1000` | OpenOCD 调试速率 (kHz) |
| | `V` | 可覆盖 | 命令行 | `0` | 设置 `V=1` 显示完整构建命令行 |


## 工程 Makefile 快速示例

创建新示例工程时，只需在工程目录下添加极简 `Makefile`：

```make
BOARD      = bluepill_f103c8t6
BOARD_CONF = -DCONFIG_LED -DCONFIG_I2C1 -DCONFIG_SSD1306
USE_LL     = rcc dma
PROJ_SRC   = board.c main.c
include ../../build/entry.mk
```

## 常用构建与调试命令

在示例工程目录下执行以下常用 `make` 命令：

```bash
# 基本构建与部署
make                            # 默认编译产生 firmware.elf、firmware_ram.elf 及内存使用报告
make flash                      # 使用 OpenOCD 将固件烧录至 Flash
make clean                      # 清理所有构建产物 (objs/ 及 *.elf/*.map)

# Flash 磨损平衡轮换烧录 (如偏移 32KB 槽位)
make APP_OFFSET=0x8000 flash

# 调试与工具
make gdb                        # 启动 GDB 调试 Flash 版本固件
make gdb_ram                    # 启动 GDB 调试 RAM 版本固件 (利用 ram_reset.gdb)
make openocd                    # 单独启动 OpenOCD 服务
make cc                         # 生成 compile_commands.json (供 Clangd / VS Code 使用)
make vimdb                      # 生成 tags / cscope 代码索引
make V=1                        # 显示完整的编译与链接命令行
```
