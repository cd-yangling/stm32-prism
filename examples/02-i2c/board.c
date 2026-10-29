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
#include "pal/i2c.h"
#include "pal/eeprom.h"
#include "dev/at24c02.h"
#include "subsys/tty.h"
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

static struct tty con_tty =
{
	.dev	= &con_dev,
	.mode	= TTY_POLL,
};
#define	BOARD_CONSOLE	(&con_tty)
#else
#define	BOARD_CONSOLE	(NULL)
#endif

#if defined(CONFIG_I2C1)
PIN_DESC(i2c1_scl, gpiob, 6, PIN_ALT | PIN_OD | PIN_SPEED_HIGH);
PIN_DESC(i2c1_sda, gpiob, 7, PIN_ALT | PIN_OD | PIN_SPEED_HIGH);
static const struct i2c_pins i2c1_pins =
{
	.scl	= &i2c1_scl,
	.sda	= &i2c1_sda,
};

static void brd_i2c_init(void)
{
	i2c_bus_init(&i2c1, &i2c1_pins);
}
#endif

#if defined(CONFIG_AT24C02)
static const struct i2c_dev ee1 =
{
	.bus	= &i2c1,
	.addr7	= 0x50,
	.spd_hz	= 100 * 1000,
	.name	= "at24c02",
};

static const struct eeprom ee_dev =
{
	.ops	= &at24c02_ops,
	.ctx	= &ee1,
	.caps	= &at24c02_caps,
	.name	= "at24c02",
};
#define	BOARD_EE1	(&ee_dev)
#else
#define	BOARD_EE1	(NULL)
#endif

const struct board board =
{
	.console	= BOARD_CONSOLE,
	.ee1		= BOARD_EE1,
};

void brd_init(void)
{
#if defined(CONFIG_I2C1)
	brd_i2c_init();
#endif
#if defined(CONFIG_USART1)
	brd_usart_init();
#endif
}
