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
#include "pal/led.h"
#include "pal/button.h"
#include "subsys/input.h"
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

#if defined(CONFIG_LED)
PIN_DESC(led_green, gpiob, 0, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);
PIN_DESC(led_blue,  gpiob, 1, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);
PIN_DESC(led_red,   gpiob, 5, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);

static const struct led_dev led_r = { .ops = &led_gpio_ops, .ctx = &led_red,   .name = "red" };
static const struct led_dev led_g = { .ops = &led_gpio_ops, .ctx = &led_green, .name = "green" };
static const struct led_dev led_b = { .ops = &led_gpio_ops, .ctx = &led_blue,  .name = "blue" };

static void brd_leds_init(void)
{
	led_init(&led_g);
	led_init(&led_b);
	led_init(&led_r);
}
#define	BOARD_LED_R	(&led_r)
#define	BOARD_LED_G	(&led_g)
#define	BOARD_LED_B	(&led_b)
#else
#define	BOARD_LED_R	(NULL)
#define	BOARD_LED_G	(NULL)
#define	BOARD_LED_B	(NULL)
#endif

#if defined(CONFIG_BUTTON)
PIN_DESC(key1_pin, gpioa, 0, PIN_IN);
PIN_DESC(key2_pin, gpioc, 13, PIN_IN);
BUTTON_DAT(key1_dat);
BUTTON_DAT(key2_dat);

static const struct button_dev key1_dev = { &key1_pin, 20, INPUT_KEY_1, key1_dat, "key1" };
static const struct button_dev key2_dev = { &key2_pin, 20, INPUT_KEY_2, key2_dat, "key2" };

static void brd_buttons_init(void)
{
	button_init(&key1_dev);
	button_init(&key2_dev);
}
#define	BOARD_KEY1	(&key1_dev)
#define	BOARD_KEY2	(&key2_dev)
#else
#define	BOARD_KEY1	(NULL)
#define	BOARD_KEY2	(NULL)
#endif

const struct board board =
{
	.console	= BOARD_CONSOLE,
	.led_r		= BOARD_LED_R,
	.led_g		= BOARD_LED_G,
	.led_b		= BOARD_LED_B,
	.key1		= BOARD_KEY1,
	.key2		= BOARD_KEY2,
};

void brd_init(void)
{
#if defined(CONFIG_LED)
	brd_leds_init();
#endif
#if defined(CONFIG_BUTTON)
	brd_buttons_init();
#endif
#if defined(CONFIG_USART1)
	brd_usart_init();
#endif
}
