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

#ifndef	__NEXUS_SUBSYS_TTY_HEADER__
#define	__NEXUS_SUBSYS_TTY_HEADER__

#include <stdint.h>

#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
#include "lib/gadget/fifo.h"
#endif

struct uart_dev;

#define	TTY_LFLAG_ECHO	0x01u

enum tty_mode
{
	TTY_POLL,
	TTY_IRQ,
	TTY_ASYNC,
};

struct tty
{
	const struct uart_dev	*dev;
	enum tty_mode			 mode;
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	struct fifo				 rx_que;
	unsigned char			*rx_buf;
	unsigned int			 rx_len;
	struct fifo				 tx_que;
	unsigned char			*tx_buf;
	unsigned int			 tx_len;
	uint32_t				 rx_drop;
	uint32_t				 tx_drop;
#endif
#ifdef CONFIG_UART_ASYNC_API
	volatile uint32_t		 tx_idle;
	unsigned char			*dma_tx_buf;
	unsigned int			 dma_tx_len;
	unsigned char			*dma_rx_buf;
	unsigned int			 dma_rx_len;
#endif
	unsigned int			 line_len;
	uint8_t					 line_ovf;
	uint8_t					 lflag;
};

int	tty_init(struct tty *t);
int	tty_write(struct tty *t, const char *buf, unsigned int len);
void		tty_lflag_set(struct tty *t, uint8_t bits);
void		tty_lflag_clr(struct tty *t, uint8_t bits);
int	tty_getc(struct tty *t);
int	tty_getline(struct tty *t, char *buf, unsigned int len);
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
int			tty_idle(struct tty *t);
uint32_t	tty_rx_drop(struct tty *t);
uint32_t	tty_tx_drop(struct tty *t);
#endif

#endif	//	__NEXUS_SUBSYS_TTY_HEADER__
