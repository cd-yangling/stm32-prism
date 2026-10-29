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

static unsigned char con_rx_buf[256];
static unsigned char con_tx_buf[256];
static unsigned char con_dma_tx_buf[96];
static unsigned char con_dma_rx_buf[64];

static struct tty con_tty =
{
	.dev		= &con_dev,
	.mode		= TTY_ASYNC,
	.rx_buf		= con_rx_buf,
	.rx_len		= sizeof(con_rx_buf),
	.tx_buf		= con_tx_buf,
	.tx_len		= sizeof(con_tx_buf),
	.dma_tx_buf	= con_dma_tx_buf,
	.dma_tx_len	= sizeof(con_dma_tx_buf),
	.dma_rx_buf	= con_dma_rx_buf,
	.dma_rx_len	= sizeof(con_dma_rx_buf),
};

static void brd_usart_init(void)
{
	uart_dev_init(&con_dev, &usart1_pins);
}
#define	BOARD_CONSOLE	(&con_tty)
#else
#define	BOARD_CONSOLE	(NULL)
#endif

const struct board board =
{
	.console	= BOARD_CONSOLE,
};

void brd_init(void)
{
#if defined(CONFIG_USART1)
	brd_usart_init();
#endif
}
