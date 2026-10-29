/*                            _ooOoo_
                            o8888888o
                             88" . "88
                             (| -_- |)
                              O\ = /O
                          ____/`---'\____
                        .   ' \\| |// `.
                         / \\||| : |||// \
                       / _||||| -:- |||||- \
                         | | \\\ - /// | |
                       | \_| ''\---/'' | |
                        \ .-\__ `-` ___/-. /
                     ___`. .' /--.--\ `. . __
                  ."" '< `.___\_<|>_/___.' >'"".
                 | | : `- \`.;`\ _ /`;.`/ - ` : | |
                   \ \ `-. \_ __\ /__ _/ .-` / /
           ======`-.____`-.___\_____/___.-`____.-'======
                              `=---='

           .............................................
                    佛祖保佑             永无BUG
            佛曰:
                    写字楼里写字间,写字间里程序员;
                    程序人员写程序,又拿程序换酒钱.
                    酒醒只在网上坐,酒醉还来网下眠;
                    酒醉酒醒日复日,网上网下年复年.
                    但愿老死电脑间,不愿鞠躬老板前;
                    奔驰宝马贵者趣,公交自行程序员.
                    别人笑我忒疯癫,我笑自己命太贱;
                    不见满街漂亮妹,哪个归得程序员?
*/

/* Copyright (C) 2026 YangLing */

#include <stdint.h>
#include <stddef.h>
#include "pal/pin.h"
#include "pal/uart.h"
#include "pal/spi.h"
#include "pal/flash.h"
#include "dev/w25q64.h"
#include "board.h"

#define	BOARD_C
#include "soc.h"

#if defined(CONFIG_USART1)
PIN_DESC(usart1_tx, gpioa, 9,  PIN_ALT | PIN_SPEED_HIGH);
PIN_DESC(usart1_rx, gpioa, 10, PIN_IN);
static const struct uart_pins usart1_pins =
{
	.tx	= &usart1_tx,
	.rx	= &usart1_rx,
};
static const struct uart_config con_cfg =
{
	.baud = 115200,
	.data_bits = 8,
	.parity = 0,
	.stop_bits = 1,
};
static const struct uart_dev con_dev =
{
	.port	= &usart1,
	.cfg	= &con_cfg,
};

static void brd_usart_init(void)
{
	uart_dev_init(&con_dev, &usart1_pins);
}
#define	BOARD_CONSOLE	(&con_dev)
#else
#define	BOARD_CONSOLE	(NULL)
#endif

#if defined(CONFIG_SPI1)
PIN_DESC(spi1_sck, gpioa, 5, PIN_ALT | PIN_SPEED_HIGH);
PIN_DESC(spi1_miso, gpioa, 6, PIN_IN);
PIN_DESC(spi1_mosi, gpioa, 7, PIN_ALT | PIN_SPEED_HIGH);
PIN_DESC(spi1_flash_cs, gpioc, 0, PIN_OUT | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE | PIN_SPEED_HIGH);

static const struct spi_pins spi1_pins =
{
	.sck	= &spi1_sck,
	.miso	= &spi1_miso,
	.mosi	= &spi1_mosi,
};

static void brd_spi_init(void)
{
	pin_configure(&spi1_flash_cs);
	spi_bus_init(&spi1, &spi1_pins);
}
#endif

#if defined(CONFIG_W25Q64)
static const struct spi_dev flash1 =
{
	.bus	= &spi1,
	.cs	= &spi1_flash_cs,
	.max_hz	= 18 * 1000 * 1000,
	.mode	= 3,
	.lsb_first	= 0,
	.name	= "w25q64",
};

static const struct flash fl_dev =
{
	.ops	= &w25q64_ops,
	.ctx	= &flash1,
	.caps	= &w25q64_caps,
	.name	= "w25q64",
};
#define	BOARD_FLASH1	(&fl_dev)
#else
#define	BOARD_FLASH1	(NULL)
#endif

const struct board board =
{
	.console	= BOARD_CONSOLE,
	.flash1		= BOARD_FLASH1,
};

void brd_init(void)
{
#if defined(CONFIG_SPI1)
	brd_spi_init();
#endif
#if defined(CONFIG_USART1)
	brd_usart_init();
#endif
}
