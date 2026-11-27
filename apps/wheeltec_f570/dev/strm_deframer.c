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
#include <errno.h>
#include <stddef.h>
#include "tick.h"
#include "pal/uart.h"
#include "dev/strm_deframer.h"

#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
static void strm_deframer_push(struct strm_deframer *s, const unsigned char *data,
		unsigned int len)
{
	unsigned int n = fifo_enqueue(&s->rxq, data, len);

	s->rx_drop += len - n;
}
#endif

#ifdef CONFIG_UART_IRQ
static void strm_deframer_irq_isr(const struct uart_port *p, void *ud)
{
	struct strm_deframer *s = ud;
	unsigned char buf[16];
	int r;

	uart_irq_update(p);

	if (uart_irq_rx_ready(p))
	{
		r = uart_fifo_read(p, buf, sizeof(buf));

		if (0 < r)
		{
			strm_deframer_push(s, buf, (unsigned int)r);
		}
	}
}
#endif

#ifdef CONFIG_UART_DMA
static void strm_deframer_dma_isr(const struct uart_port *p,
		const struct uart_event *evt, void *ud)
{
	struct strm_deframer *s = ud;

	(void)p;

	if (UART_RX_RDY == evt->type)
	{
		strm_deframer_push(s, &evt->data.rx.buf[evt->data.rx.offset],
				evt->data.rx.len);
	}
}
#endif

int strm_deframer_init(struct strm_deframer *s)
{
	int r;

	r = deframer_init(&s->dfr, s->dfr_cfg);

	if (0 != r)
	{
		return r;
	}

#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
	s->rx_drop = 0u;
#endif

	if (STRM_DEFRAMER_POLL == s->mode)
	{
		return 0;
	}

#ifdef CONFIG_UART_IRQ
	if (STRM_DEFRAMER_IRQ == s->mode)
	{
		r = fifo_init(&s->rxq, s->rxq_buf, STRM_DEFRAMER_QUE_RX_LEN);

		if (0 != r)
		{
			return r;
		}

		r = uart_irq_callback_set(s->dev->port, strm_deframer_irq_isr, s);

		if (0 != r)
		{
			return r;
		}

		uart_irq_rx_enable(s->dev->port);

		return 0;
	}
#endif
#ifdef CONFIG_UART_DMA
	if (STRM_DEFRAMER_ASYNC == s->mode)
	{
		r = fifo_init(&s->rxq, s->rxq_buf, STRM_DEFRAMER_QUE_RX_LEN);

		if (0 != r)
		{
			return r;
		}

		r = uart_callback_set(s->dev->port, strm_deframer_dma_isr, s);

		if (0 != r)
		{
			return r;
		}

		return uart_rx_enable(s->dev->port, s->dma_buf, STRM_DEFRAMER_DMA_RX_LEN);
	}
#endif

	return -ENOSYS;
}

void strm_deframer_feed(struct strm_deframer *s)
{
	if (STRM_DEFRAMER_POLL == s->mode)
	{
		unsigned char c;

		while (0 == uart_poll_getc(s->dev->port, &c))
		{
			(void)deframer_feed(&s->dfr, &c, 1u, tick_get_cyc());
		}

		return;
	}

#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
	while (!fifo_is_empty(&s->rxq))
	{
		unsigned char buf[32];
		unsigned int n = fifo_dequeue(&s->rxq, buf, sizeof(buf));

		(void)deframer_feed(&s->dfr, buf, (unsigned short)n, tick_get_cyc());
	}
#endif
}

const struct deframer_rec *strm_deframer_peek(const struct strm_deframer *s)
{
	return deframer_peek(&s->dfr);
}

void strm_deframer_advance(struct strm_deframer *s)
{
	deframer_advance(&s->dfr);
}

#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
unsigned int strm_deframer_rx_drop_get(const struct strm_deframer *s)
{
	return s->rx_drop;
}
#endif

unsigned int strm_deframer_frame_drop_get(const struct strm_deframer *s)
{
	return deframer_fail_get(&s->dfr);
}
