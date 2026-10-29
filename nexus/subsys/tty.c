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
#include <errno.h>
#include "irq.h"
#include "pal/uart.h"
#include "subsys/tty.h"

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void tty_isr(const struct uart_port *p, void *ud)
{
	struct tty *t = ud;
	uint8_t buf[16];
	uint8_t ch;
	int r;
	int c;

	uart_irq_update(p);

	if (uart_irq_rx_ready(p))
	{
		r = uart_fifo_read(p, buf, sizeof(buf));

		if (0 < r)
		{
			t->rx_drop += r
					- fifo_enqueue(&t->rx_que, buf, r);
		}
	}

	if (uart_irq_tx_ready(p))
	{
		c = fifo_getc(&t->tx_que);

		if (0 <= c)
		{
			ch = (uint8_t)c;
			(void)uart_fifo_fill(p, &ch, 1);
		}
		else
		{
			uart_irq_tx_disable(p);
		}
	}
}

static unsigned int tty_irq_write(struct tty *t, const char *buf,
		unsigned int len)
{
	unsigned int n;

	n = fifo_enqueue(&t->tx_que, buf, len);
	t->tx_drop += len - n;

	if (!fifo_is_empty(&t->tx_que))
	{
		uart_irq_tx_enable(t->dev->port);
	}

	return n;
}
#endif

#ifdef CONFIG_UART_ASYNC_API
static void tty_async_submit(struct tty *t)
{
	unsigned int n;

	n = fifo_dequeue(&t->tx_que, t->dma_tx_buf, t->dma_tx_len);

	if (0u == n)
	{
		t->tx_idle = 1;
		return;
	}

	(void)uart_tx(t->dev->port, t->dma_tx_buf, n);
}

static void tty_async_event(const struct uart_port *p,
		const struct uart_event *evt, void *ud)
{
	struct tty *t = ud;
	unsigned int n;

	(void)p;

	if (UART_RX_RDY == evt->type)
	{
		n = fifo_enqueue(&t->rx_que, &evt->data.rx.buf[evt->data.rx.offset],
				evt->data.rx.len);
		t->rx_drop += evt->data.rx.len - n;
	}
	else if ((UART_TX_DONE == evt->type) || (UART_TX_ABORTED == evt->type))
	{
		if (fifo_is_empty(&t->tx_que))
		{
			t->tx_idle = 1;
		}
		else
		{
			tty_async_submit(t);
		}
	}
}

static unsigned int tty_async_write(struct tty *t, const char *buf,
		unsigned int len)
{
	unsigned int key;
	unsigned int n;

	n = fifo_enqueue(&t->tx_que, buf, len);
	t->tx_drop += len - n;

	if (!t->tx_idle)
	{
		return n;
	}

	key = irq_lock();

	if (!t->tx_idle)
	{
		irq_unlock(key);
		return n;
	}

	t->tx_idle = 0;
	irq_unlock(key);

	tty_async_submit(t);

	return n;
}
#endif

int tty_write(struct tty *t, const char *buf, unsigned int len)
{
#ifdef CONFIG_UART_ASYNC_API
	if (TTY_ASYNC == t->mode)
	{
		return (int)tty_async_write(t, buf, len);
	}
#endif
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	if (TTY_IRQ == t->mode)
	{
		return (int)tty_irq_write(t, buf, len);
	}
#endif

	return uart_poll_write(t->dev->port, (const uint8_t *)buf, len);
}

void tty_lflag_set(struct tty *t, uint8_t bits)
{
	t->lflag |= bits;
}

void tty_lflag_clr(struct tty *t, uint8_t bits)
{
	t->lflag &= (uint8_t)~bits;
}

int tty_init(struct tty *t)
{
	t->lflag = 0u;

	if (TTY_POLL == t->mode)
	{
		return 0;
	}

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	if (TTY_IRQ == t->mode)
	{
		int r;

		r = fifo_init(&t->rx_que, t->rx_buf, t->rx_len);
		if (0 != r)
		{
			return r;
		}

		r = fifo_init(&t->tx_que, t->tx_buf, t->tx_len);
		if (0 != r)
		{
			return r;
		}

		r = uart_irq_callback_set(t->dev->port, tty_isr, t);
		if (0 != r)
		{
			return r;
		}

		uart_irq_rx_enable(t->dev->port);

		return 0;
	}
#endif
#ifdef CONFIG_UART_ASYNC_API
	if (TTY_ASYNC == t->mode)
	{
		int r;

		r = fifo_init(&t->rx_que, t->rx_buf, t->rx_len);
		if (0 != r)
		{
			return r;
		}

		r = fifo_init(&t->tx_que, t->tx_buf, t->tx_len);
		if (0 != r)
		{
			return r;
		}

		t->tx_idle = 1;
		r = uart_callback_set(t->dev->port, tty_async_event, t);
		if (0 != r)
		{
			return r;
		}

		return uart_rx_enable(t->dev->port, t->dma_rx_buf, t->dma_rx_len);
	}
#endif

	return -ENOSYS;
}

int tty_getc(struct tty *t)
{
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	if (TTY_POLL != t->mode)
	{
		return fifo_getc(&t->rx_que);
	}
#endif
	{
		uint8_t c;

		if (0 != uart_poll_getc(t->dev->port, &c))
		{
			return -EAGAIN;
		}

		return c;
	}
}

int tty_getline(struct tty *t, char *buf, unsigned int len)
{
	static const char err_line[] = "ERR line too long\r\n";
	int c;

	if (len < 2u)
	{
		return -EINVAL;
	}

	while (-EAGAIN != (c = tty_getc(t)))
	{
		char ch = (char)c;

		if (('\r' == ch) || ('\n' == ch))
		{
			if (t->lflag & TTY_LFLAG_ECHO)
			{
				tty_write(t, "\r\n", 2u);
			}

			if (t->line_ovf)
			{
				t->line_ovf = 0u;
				t->line_len = 0u;
				tty_write(t, err_line, sizeof(err_line) - 1u);
				return -EAGAIN;
			}

			buf[t->line_len] = '\0';
			t->line_len = 0u;
			return 0;
		}

		if (t->line_len < (len - 1u))
		{
			buf[t->line_len++] = ch;
		}
		else
		{
			t->line_ovf = 1u;
		}

		if (t->lflag & TTY_LFLAG_ECHO)
		{
			tty_write(t, &ch, 1u);
		}
	}

	return -EAGAIN;
}

#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
int tty_idle(struct tty *t)
{
#ifdef CONFIG_UART_ASYNC_API
	if (!t->tx_idle)
	{
		return 0;
	}
#endif

	if ((!fifo_is_empty(&t->tx_que)) || (!fifo_is_empty(&t->rx_que)))
	{
		return 0;
	}

	return 1;
}

uint32_t tty_rx_drop(struct tty *t)
{
	return t->rx_drop;
}

uint32_t tty_tx_drop(struct tty *t)
{
	return t->tx_drop;
}
#endif
