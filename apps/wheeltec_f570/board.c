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
#include "pal/led.h"
#include "pal/uart.h"
#include "subsys/tty.h"
#include "board.h"

#define	BOARD_C
#include "soc.h"

#if defined(CONFIG_LED)
PIN_DESC(led_1, gpiob, 14, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);
PIN_DESC(led_2, gpiob, 15, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);

static const struct led_dev led1 = { .ops = &led_gpio_ops, .ctx = &led_1, .name = "led1" };
static const struct led_dev led2 = { .ops = &led_gpio_ops, .ctx = &led_2, .name = "led2" };
#define	BOARD_LED1	(&led1)
#define	BOARD_LED2	(&led2)
#else
#define	BOARD_LED1	(NULL)
#define	BOARD_LED2	(NULL)
#endif

#if defined(CONFIG_BUZZER)
PIN_DESC(buzzer, gpioa, 4, PIN_OUT | PIN_SPEED_LOW | PIN_INIT_INACTIVE);
#define	BOARD_BUZZER	(&buzzer)
#else
#define	BOARD_BUZZER	(NULL)
#endif

#if defined(CONFIG_USART1)
PIN_DESC(usart1_tx, gpioa, 9,  PIN_ALT | PIN_MUX(7) | PIN_SPEED_VERY_HIGH);
PIN_DESC(usart1_rx, gpioa, 10, PIN_ALT | PIN_MUX(7));
static const struct uart_pins usart1_pins =
{
	.tx	= &usart1_tx,
	.rx	= &usart1_rx,
};
static const struct uart_config con_cfg =
{
	.baud = 1152000,
	.data_bits = 8,
	.parity = 0,
	.stop_bits = 1,
};
static const struct uart_dev con_dev =
{
	.port	= &usart1,
	.cfg	= &con_cfg,
};

#if defined(CONFIG_USART1_IRQ) || defined(CONFIG_USART1_DMA)
static unsigned char con_rx_buf[256];
static unsigned char con_tx_buf[256];
#endif
#ifdef CONFIG_USART1_DMA
static unsigned char con_dma_tx_buf[96];
static unsigned char con_dma_rx_buf[64];
#endif

static struct tty con_tty =
{
	.dev		= &con_dev,
#if defined(CONFIG_USART1_DMA)
	.mode		= TTY_ASYNC,
#elif defined(CONFIG_USART1_IRQ)
	.mode		= TTY_IRQ,
#else
	.mode		= TTY_POLL,
#endif
#if defined(CONFIG_USART1_IRQ) || defined(CONFIG_USART1_DMA)
	.rx_buf		= con_rx_buf,
	.rx_len		= sizeof(con_rx_buf),
	.tx_buf		= con_tx_buf,
	.tx_len		= sizeof(con_tx_buf),
#endif
#ifdef CONFIG_USART1_DMA
	.dma_tx_buf	= con_dma_tx_buf,
	.dma_tx_len	= sizeof(con_dma_tx_buf),
	.dma_rx_buf	= con_dma_rx_buf,
	.dma_rx_len	= sizeof(con_dma_rx_buf),
#endif
};
#define	BOARD_CONSOLE	(&con_tty)
#else
#define	BOARD_CONSOLE	(NULL)
#endif

static void brd_uart_init(void)
{
#if defined(CONFIG_USART1)
	uart_dev_init(&con_dev, &usart1_pins);
#endif
}

static void brd_led_init(void)
{
#if defined(CONFIG_LED)
	led_init(&led1);
	led_init(&led2);
#endif
}

const struct board board =
{
	.led1		= BOARD_LED1,
	.led2		= BOARD_LED2,
	.buzzer		= BOARD_BUZZER,
	.console	= BOARD_CONSOLE,
};

void brd_init(void)
{
	brd_led_init();
	brd_uart_init();
}
