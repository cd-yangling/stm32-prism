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

#if defined(CONFIG_USART1) || defined(CONFIG_USART2) || defined(CONFIG_USART3) \
	|| defined(CONFIG_UART4) || defined(CONFIG_UART5) || defined(CONFIG_USART6) \
	|| defined(CONFIG_UART7) || defined(CONFIG_UART8) || defined(CONFIG_UART9) \
	|| defined(CONFIG_UART10)
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_usart.h"
#include "pal/uart.h"
#include "pal/pin.h"
#include "tick.h"
#include "irq.h"
#ifdef CONFIG_UART_DMA
#include "drv/dma.h"
#endif

#define	UART_TMO_MS	(100u)

#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
struct uart_dat
{
#ifdef CONFIG_UART_IRQ
	uart_irq_cb_t		cb;
	void			*	ud;
#endif
#ifdef CONFIG_UART_DMA
	uart_async_cb_t		acb;
	void			*	aud;
	uint8_t			*	rx_buf;
	uint32_t			rx_len;
	uint32_t			rx_pos;
	const uint8_t	*	tx_buf;
	uint32_t			tx_len;
#endif
};
#endif

struct uart_ctx
{
	USART_TypeDef	*regs;
#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
	struct uart_dat	*dat;
#endif
#ifdef CONFIG_UART_DMA
	const struct dma_chan	*rx_dma;
	const struct dma_chan	*tx_dma;
#endif
};

static int f4_uart_init(const struct uart_port *p, const struct uart_pins *pins,
		const struct uart_config *cfg)
{
	const struct uart_ctx *x = p->ctx;
	LL_RCC_ClocksTypeDef rcc_clock;
	uint32_t pclk;

	LL_RCC_GetSystemClocksFreq(&rcc_clock);

	if (USART1 == x->regs)
	{
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
		pclk = rcc_clock.PCLK2_Frequency;
#if defined(CONFIG_USART1_IRQ) || defined(CONFIG_USART1_DMA)
		NVIC_SetPriority(USART1_IRQn, IRQ_PRIO_USART1);
		NVIC_EnableIRQ(USART1_IRQn);
#endif
	}
	else if (USART2 == x->regs)
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
		pclk = rcc_clock.PCLK1_Frequency;
#if defined(CONFIG_USART2_IRQ) || defined(CONFIG_USART2_DMA)
		NVIC_SetPriority(USART2_IRQn, IRQ_PRIO_USART2);
		NVIC_EnableIRQ(USART2_IRQn);
#endif
	}
	else if (USART3 == x->regs)
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
		pclk = rcc_clock.PCLK1_Frequency;
#if defined(CONFIG_USART3_IRQ) || defined(CONFIG_USART3_DMA)
		NVIC_SetPriority(USART3_IRQn, IRQ_PRIO_USART3);
		NVIC_EnableIRQ(USART3_IRQn);
#endif
	}
#if defined(UART4)
	else if (UART4 == x->regs)
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
		pclk = rcc_clock.PCLK1_Frequency;
#if defined(CONFIG_UART4_IRQ) || defined(CONFIG_UART4_DMA)
		NVIC_SetPriority(UART4_IRQn, IRQ_PRIO_UART4);
		NVIC_EnableIRQ(UART4_IRQn);
#endif
	}
#endif
#if defined(UART5)
	else if (UART5 == x->regs)
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);
		pclk = rcc_clock.PCLK1_Frequency;
#if defined(CONFIG_UART5_IRQ) || defined(CONFIG_UART5_DMA)
		NVIC_SetPriority(UART5_IRQn, IRQ_PRIO_UART5);
		NVIC_EnableIRQ(UART5_IRQn);
#endif
	}
#endif
#if defined(USART6)
	else if (USART6 == x->regs)
	{
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
		pclk = rcc_clock.PCLK2_Frequency;
#if defined(CONFIG_USART6_IRQ) || defined(CONFIG_USART6_DMA)
		NVIC_SetPriority(USART6_IRQn, IRQ_PRIO_USART6);
		NVIC_EnableIRQ(USART6_IRQn);
#endif
	}
#endif
#if defined(UART7)
	else if (UART7 == x->regs)
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART7);
		pclk = rcc_clock.PCLK1_Frequency;
#if defined(CONFIG_UART7_IRQ) || defined(CONFIG_UART7_DMA)
		NVIC_SetPriority(UART7_IRQn, IRQ_PRIO_UART7);
		NVIC_EnableIRQ(UART7_IRQn);
#endif
	}
#endif
#if defined(UART8)
	else if (UART8 == x->regs)
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART8);
		pclk = rcc_clock.PCLK1_Frequency;
#if defined(CONFIG_UART8_IRQ) || defined(CONFIG_UART8_DMA)
		NVIC_SetPriority(UART8_IRQn, IRQ_PRIO_UART8);
		NVIC_EnableIRQ(UART8_IRQn);
#endif
	}
#endif
#if defined(UART9)
	else if (UART9 == x->regs)
	{
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_UART9);
		pclk = rcc_clock.PCLK2_Frequency;
#if defined(CONFIG_UART9_IRQ) || defined(CONFIG_UART9_DMA)
		NVIC_SetPriority(UART9_IRQn, IRQ_PRIO_UART9);
		NVIC_EnableIRQ(UART9_IRQn);
#endif
	}
#endif
#if defined(UART10)
	else if (UART10 == x->regs)
	{
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_UART10);
		pclk = rcc_clock.PCLK2_Frequency;
#if defined(CONFIG_UART10_IRQ) || defined(CONFIG_UART10_DMA)
		NVIC_SetPriority(UART10_IRQn, IRQ_PRIO_UART10);
		NVIC_EnableIRQ(UART10_IRQn);
#endif
	}
#endif
	else
	{
		return -ENODEV;
	}

	pin_configure(pins->tx);
	pin_configure(pins->rx);

	LL_USART_Disable(x->regs);

	LL_USART_SetDataWidth(x->regs, (9 == cfg->data_bits) ? LL_USART_DATAWIDTH_9B : LL_USART_DATAWIDTH_8B);
	LL_USART_SetParity(x->regs, (1 == cfg->parity) ? LL_USART_PARITY_EVEN
			: (2 == cfg->parity) ? LL_USART_PARITY_ODD : LL_USART_PARITY_NONE);
	LL_USART_SetStopBitsLength(x->regs, (2 == cfg->stop_bits) ? LL_USART_STOPBITS_2 : LL_USART_STOPBITS_1);
	LL_USART_SetTransferDirection(x->regs, LL_USART_DIRECTION_TX_RX);
	LL_USART_SetHWFlowCtrl(x->regs, LL_USART_HWCONTROL_NONE);
	LL_USART_SetBaudRate(x->regs, pclk, LL_USART_OVERSAMPLING_16, cfg->baud);

	LL_USART_Enable(x->regs);

	return 0;
}

static int f4_uart_poll_out(const struct uart_port *p, uint8_t c)
{
	const struct uart_ctx *x = p->ctx;
	uint32_t t0 = tick_get();

	while (!LL_USART_IsActiveFlag_TXE(x->regs))
	{
		if ((tick_get() - t0) >= UART_TMO_MS)
		{
			return -ETIMEDOUT;
		}
	}

	LL_USART_TransmitData8(x->regs, c);

	return 0;
}

static int f4_uart_poll_in(const struct uart_port *p, uint8_t *c)
{
	const struct uart_ctx *x = p->ctx;

	if (!LL_USART_IsActiveFlag_RXNE(x->regs))
		return -EAGAIN;

	*c = (uint8_t)LL_USART_ReceiveData8(x->regs);

	return 0;
}

#ifdef CONFIG_UART_IRQ
static int f4_uart_fifo_fill(const struct uart_port *p, const uint8_t *tx,
		uint32_t len)
{
	const struct uart_ctx *x = p->ctx;
	uint32_t n = 0;

	while ((n < len) && (LL_USART_IsActiveFlag_TXE(x->regs)))
	{
		LL_USART_TransmitData8(x->regs, tx[n]);
		n++;
	}

	return n;
}

static int f4_uart_fifo_read(const struct uart_port *p, uint8_t *rx,
		uint32_t len)
{
	const struct uart_ctx *x = p->ctx;
	uint32_t n = 0;

	while ((n < len) && LL_USART_IsActiveFlag_RXNE(x->regs))
	{
		rx[n] = (uint8_t)LL_USART_ReceiveData8(x->regs);
		n++;

		if (LL_USART_IsActiveFlag_ORE(x->regs))
		{
			LL_USART_ClearFlag_ORE(x->regs);
		}
	}

	return n;
}

static int f4_uart_irq_update(const struct uart_port *p)
{
	(void)p;

	return 1;
}

static int f4_uart_irq_callback_set(const struct uart_port *p, uart_irq_cb_t cb, void *ud)
{
	const struct uart_ctx *x = p->ctx;

	if (NULL == x->dat)
	{
		return -ENODEV;
	}

	x->dat->cb = cb;
	x->dat->ud = ud;

	return 0;
}

static int f4_uart_irq_is_pending(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	if (((LL_USART_IsActiveFlag_RXNE(x->regs))
		&& (LL_USART_IsEnabledIT_RXNE(x->regs)))
		|| ((LL_USART_IsActiveFlag_TXE(x->regs))
		&& (LL_USART_IsEnabledIT_TXE(x->regs))))
	{
		return 1;
	}

	return 0;
}

static int f4_uart_irq_tx_ready(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	return LL_USART_IsActiveFlag_TXE(x->regs);
}

static int f4_uart_irq_rx_ready(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	return LL_USART_IsActiveFlag_RXNE(x->regs);
}

static void f4_uart_irq_tx_disable(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	LL_USART_DisableIT_TXE(x->regs);
}

static void f4_uart_irq_rx_disable(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	LL_USART_DisableIT_RXNE(x->regs);
}

static void f4_uart_irq_tx_enable(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	LL_USART_EnableIT_TXE(x->regs);
}

static void f4_uart_irq_rx_enable(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	LL_USART_EnableDirectionRx(x->regs);
	LL_USART_EnableIT_RXNE(x->regs);
}
#endif

#ifdef CONFIG_UART_DMA
static void f4_uart_harvest(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;
	struct uart_event ev;
	uint32_t cur;

	if ((NULL == d->rx_buf) || (NULL == d->acb))
	{
		return;
	}

	cur = d->rx_len - dma_remain(x->rx_dma);

	if (cur == d->rx_pos)
	{
		return;
	}

	ev.type = UART_RX_RDY;

	if (cur > d->rx_pos)
	{
		ev.data.rx.buf = d->rx_buf;
		ev.data.rx.offset = d->rx_pos;
		ev.data.rx.len = cur - d->rx_pos;
		d->acb(p, &ev, d->aud);
	}
	else
	{
		ev.data.rx.buf = d->rx_buf;
		ev.data.rx.offset = d->rx_pos;
		ev.data.rx.len = d->rx_len - d->rx_pos;
		d->acb(p, &ev, d->aud);

		if (cur > 0)
		{
			ev.data.rx.offset = 0;
			ev.data.rx.len = cur;
			d->acb(p, &ev, d->aud);
		}
	}

	d->rx_pos = cur;
}

static void f4_uart_dma_ev(const struct dma_chan *c, enum dma_event_type ev, void *ud)
{
	(void)c;
	(void)ev;

	f4_uart_harvest((const struct uart_port *)ud);
}

static int f4_uart_callback_set(const struct uart_port *p, uart_async_cb_t cb,
		void *ud)
{
	const struct uart_ctx *x = p->ctx;

	if (NULL == x->dat)
	{
		return -ENODEV;
	}

	x->dat->acb = cb;
	x->dat->aud = ud;

	return 0;
}

static int f4_uart_rx_enable(const struct uart_port *p, uint8_t *buf, uint32_t len)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;

	if (NULL == x->rx_dma)
	{
		return -ENODEV;
	}

	if ((NULL != d->rx_buf) || (0 == len))
	{
		return -EINVAL;
	}

	d->rx_buf = buf;
	d->rx_len = len;
	d->rx_pos = 0;

	struct dma_xfer xf =
	{
		.buf	= buf,
		.len	= len,
		.mode	= DMA_MODE_CYCLIC,
		.cb		= f4_uart_dma_ev,
		.ud		= (void *)p,
	};

	if (dma_start(x->rx_dma, &xf))
	{
		d->rx_buf = NULL;
		return -EINVAL;
	}

	LL_USART_EnableDMAReq_RX(x->regs);
	LL_USART_EnableIT_IDLE(x->regs);
	LL_USART_EnableDirectionRx(x->regs);

	return 0;
}

static int f4_uart_rx_disable(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;
	struct uart_event ev;

	if (NULL == x->dat)
	{
		return -ENODEV;
	}

	if (NULL == d->rx_buf)
	{
		return -EINVAL;
	}

	LL_USART_DisableDirectionRx(x->regs);
	LL_USART_DisableIT_IDLE(x->regs);
	LL_USART_DisableDMAReq_RX(x->regs);
	dma_stop(x->rx_dma);
	f4_uart_harvest(p);
	d->rx_buf = NULL;

	if (NULL != d->acb)
	{
		ev.type = UART_RX_DISABLED;
		d->acb(p, &ev, d->aud);
	}

	return 0;
}

static int f4_uart_tx(const struct uart_port *p, const uint8_t *buf, uint32_t len)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;

	if (NULL == x->tx_dma)
	{
		return -ENODEV;
	}

	if (0 == len)
	{
		return -EINVAL;
	}

	d->tx_buf = buf;
	d->tx_len = len;

	LL_USART_ClearFlag_TC(x->regs);
	LL_USART_EnableIT_TC(x->regs);

	struct dma_xfer xf =
	{
		.buf	= (uint8_t *)buf,
		.len	= len,
		.mode	= DMA_MODE_SINGLE,
		.cb		= NULL,
		.ud		= NULL,
	};

	if (dma_start(x->tx_dma, &xf))
	{
		LL_USART_DisableIT_TC(x->regs);
		d->tx_buf = NULL;
		return -EINVAL;
	}

	LL_USART_EnableDMAReq_TX(x->regs);

	return 0;
}

static int f4_uart_tx_abort(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;
	struct uart_event ev;
	const uint8_t *b;
	uint32_t sent;

	if (NULL == x->dat)
	{
		return -ENODEV;
	}

	if (NULL == d->tx_buf)
	{
		return -ENODEV;
	}

	LL_USART_DisableDMAReq_TX(x->regs);
	LL_USART_DisableIT_TC(x->regs);
	dma_stop(x->tx_dma);
	b = d->tx_buf;
	sent = d->tx_len - dma_remain(x->tx_dma);
	d->tx_buf = NULL;

	if (NULL != d->acb)
	{
		ev.type = UART_TX_ABORTED;
		ev.data.tx.buf = b;
		ev.data.tx.len = sent;
		d->acb(p, &ev, d->aud);
	}

	return 0;
}
#endif

#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
static void f4_uart_isr(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;
#ifdef CONFIG_UART_IRQ
	if (NULL != d->cb)
	{
		d->cb(p, d->ud);
	}
#endif
#ifdef CONFIG_UART_DMA
	if ((LL_USART_IsActiveFlag_IDLE(x->regs))
		&& (LL_USART_IsEnabledIT_IDLE(x->regs)))
	{
		LL_USART_ClearFlag_IDLE(x->regs);
		f4_uart_harvest(p);
	}
	else if ((LL_USART_IsActiveFlag_TC(x->regs))
		&& (LL_USART_IsEnabledIT_TC(x->regs)))
	{
		const uint8_t *b = d->tx_buf;
		uint32_t l = d->tx_len;

		LL_USART_ClearFlag_TC(x->regs);
		LL_USART_DisableIT_TC(x->regs);
		LL_USART_DisableDMAReq_TX(x->regs);
		d->tx_buf = NULL;

		if ((NULL != b) && (NULL != d->acb))
		{
			struct uart_event ev;

			ev.type = UART_TX_DONE;
			ev.data.tx.buf = b;
			ev.data.tx.len = l;
			d->acb(p, &ev, d->aud);
		}
	}
#endif
}
#endif

static const struct uart_ops f4_uart_ops =
{
	.init				= f4_uart_init,
	.poll_out			= f4_uart_poll_out,
	.poll_in			= f4_uart_poll_in,
#ifdef CONFIG_UART_IRQ
	.fifo_fill			= f4_uart_fifo_fill,
	.fifo_read			= f4_uart_fifo_read,
	.irq_update			= f4_uart_irq_update,
	.irq_callback_set	= f4_uart_irq_callback_set,
	.irq_is_pending		= f4_uart_irq_is_pending,
	.irq_tx_ready		= f4_uart_irq_tx_ready,
	.irq_rx_ready		= f4_uart_irq_rx_ready,
	.irq_tx_disable		= f4_uart_irq_tx_disable,
	.irq_rx_disable		= f4_uart_irq_rx_disable,
	.irq_tx_enable		= f4_uart_irq_tx_enable,
	.irq_rx_enable		= f4_uart_irq_rx_enable,
#endif
#ifdef CONFIG_UART_DMA
	.callback_set		= f4_uart_callback_set,
	.tx					= f4_uart_tx,
	.tx_abort			= f4_uart_tx_abort,
	.rx_enable			= f4_uart_rx_enable,
	.rx_disable			= f4_uart_rx_disable,
#endif
};

#if defined(USART1) && defined(CONFIG_USART1)
#if defined(CONFIG_USART1_IRQ) || defined(CONFIG_USART1_DMA)
static struct uart_dat usart1_dat;
#endif
#ifdef CONFIG_USART1_DMA
DMA2_STREAM_OWN(5, 4, &USART1->DR, DMA_DIR_PERIPH_TO_MEM, DMA_WIDTH_U8, IRQ_PRIO_USART1);
DMA2_STREAM_OWN(7, 4, &USART1->DR, DMA_DIR_MEM_TO_PERIPH, DMA_WIDTH_U8, IRQ_PRIO_USART1);
#endif
static const struct uart_ctx usart1_ctx =
{
	.regs		= USART1,
#if defined(CONFIG_USART1_IRQ) || defined(CONFIG_USART1_DMA)
	.dat		= &usart1_dat,
#endif
#ifdef CONFIG_USART1_DMA
	.rx_dma		= &dma2_s5,
	.tx_dma		= &dma2_s7,
#endif
};
const struct uart_port usart1 =
{
	.ops		= &f4_uart_ops,
	.ctx		= &usart1_ctx,
	.name		= "usart1",
};
#if defined(CONFIG_USART1_IRQ) || defined(CONFIG_USART1_DMA)
void USART1_IRQHandler(void)
{
	f4_uart_isr(&usart1);
}
#endif
#endif	//	USART1

#if defined(USART2) && defined(CONFIG_USART2)
#if defined(CONFIG_USART2_IRQ) || defined(CONFIG_USART2_DMA)
static struct uart_dat usart2_dat;
#endif
#ifdef CONFIG_USART2_DMA
DMA1_STREAM_OWN(5, 4, &USART2->DR, DMA_DIR_PERIPH_TO_MEM, DMA_WIDTH_U8, IRQ_PRIO_USART2);
DMA1_STREAM_OWN(6, 4, &USART2->DR, DMA_DIR_MEM_TO_PERIPH, DMA_WIDTH_U8, IRQ_PRIO_USART2);
#endif
static const struct uart_ctx usart2_ctx =
{
	.regs		= USART2,
#if defined(CONFIG_USART2_IRQ) || defined(CONFIG_USART2_DMA)
	.dat		= &usart2_dat,
#endif
#ifdef CONFIG_USART2_DMA
	.rx_dma		= &dma1_s5,
	.tx_dma		= &dma1_s6,
#endif
};
const struct uart_port usart2 =
{
	.ops		= &f4_uart_ops,
	.ctx		= &usart2_ctx,
	.name		= "usart2",
};
#if defined(CONFIG_USART2_IRQ) || defined(CONFIG_USART2_DMA)
void USART2_IRQHandler(void)
{
	f4_uart_isr(&usart2);
}
#endif
#endif	//	USART2

#if defined(USART3) && defined(CONFIG_USART3)
#if defined(CONFIG_USART3_IRQ) || defined(CONFIG_USART3_DMA)
static struct uart_dat usart3_dat;
#endif
#ifdef CONFIG_USART3_DMA
DMA1_STREAM_OWN(1, 4, &USART3->DR, DMA_DIR_PERIPH_TO_MEM, DMA_WIDTH_U8, IRQ_PRIO_USART3);
DMA1_STREAM_OWN(3, 4, &USART3->DR, DMA_DIR_MEM_TO_PERIPH, DMA_WIDTH_U8, IRQ_PRIO_USART3);
#endif
static const struct uart_ctx usart3_ctx =
{
	.regs		= USART3,
#if defined(CONFIG_USART3_IRQ) || defined(CONFIG_USART3_DMA)
	.dat		= &usart3_dat,
#endif
#ifdef CONFIG_USART3_DMA
	.rx_dma		= &dma1_s1,
	.tx_dma		= &dma1_s3,
#endif
};
const struct uart_port usart3 =
{
	.ops		= &f4_uart_ops,
	.ctx		= &usart3_ctx,
	.name		= "usart3",
};
#if defined(CONFIG_USART3_IRQ) || defined(CONFIG_USART3_DMA)
void USART3_IRQHandler(void)
{
	f4_uart_isr(&usart3);
}
#endif
#endif	//	USART3

#if defined(UART4) && defined(CONFIG_UART4)
#if defined(CONFIG_UART4_IRQ) || defined(CONFIG_UART4_DMA)
static struct uart_dat uart4_dat;
#endif
#ifdef CONFIG_UART4_DMA
DMA1_STREAM_OWN(2, 4, &UART4->DR, DMA_DIR_PERIPH_TO_MEM, DMA_WIDTH_U8, IRQ_PRIO_UART4);
DMA1_STREAM_OWN(4, 4, &UART4->DR, DMA_DIR_MEM_TO_PERIPH, DMA_WIDTH_U8, IRQ_PRIO_UART4);
#endif
static const struct uart_ctx uart4_ctx =
{
	.regs		= UART4,
#if defined(CONFIG_UART4_IRQ) || defined(CONFIG_UART4_DMA)
	.dat		= &uart4_dat,
#endif
#ifdef CONFIG_UART4_DMA
	.rx_dma		= &dma1_s2,
	.tx_dma		= &dma1_s4,
#endif
};
const struct uart_port uart4 =
{
	.ops		= &f4_uart_ops,
	.ctx		= &uart4_ctx,
	.name		= "uart4",
};
#if defined(CONFIG_UART4_IRQ) || defined(CONFIG_UART4_DMA)
void UART4_IRQHandler(void)
{
	f4_uart_isr(&uart4);
}
#endif
#endif	//	UART4

#if defined(UART5) && defined(CONFIG_UART5)
#if defined(CONFIG_UART5_IRQ) || defined(CONFIG_UART5_DMA)
static struct uart_dat uart5_dat;
#endif
#ifdef CONFIG_UART5_DMA
DMA1_STREAM_OWN(0, 4, &UART5->DR, DMA_DIR_PERIPH_TO_MEM, DMA_WIDTH_U8, IRQ_PRIO_UART5);
DMA1_STREAM_OWN(7, 4, &UART5->DR, DMA_DIR_MEM_TO_PERIPH, DMA_WIDTH_U8, IRQ_PRIO_UART5);
#endif
static const struct uart_ctx uart5_ctx =
{
	.regs		= UART5,
#if defined(CONFIG_UART5_IRQ) || defined(CONFIG_UART5_DMA)
	.dat		= &uart5_dat,
#endif
#ifdef CONFIG_UART5_DMA
	.rx_dma		= &dma1_s0,
	.tx_dma		= &dma1_s7,
#endif
};
const struct uart_port uart5 =
{
	.ops		= &f4_uart_ops,
	.ctx		= &uart5_ctx,
	.name		= "uart5",
};
#if defined(CONFIG_UART5_IRQ) || defined(CONFIG_UART5_DMA)
void UART5_IRQHandler(void)
{
	f4_uart_isr(&uart5);
}
#endif
#endif	//	UART5

#if defined(USART6) && defined(CONFIG_USART6)
#if defined(CONFIG_USART6_IRQ) || defined(CONFIG_USART6_DMA)
static struct uart_dat usart6_dat;
#endif
#ifdef CONFIG_USART6_DMA
DMA2_STREAM_OWN(1, 5, &USART6->DR, DMA_DIR_PERIPH_TO_MEM, DMA_WIDTH_U8, IRQ_PRIO_USART6);
DMA2_STREAM_OWN(6, 5, &USART6->DR, DMA_DIR_MEM_TO_PERIPH, DMA_WIDTH_U8, IRQ_PRIO_USART6);
#endif
static const struct uart_ctx usart6_ctx =
{
	.regs		= USART6,
#if defined(CONFIG_USART6_IRQ) || defined(CONFIG_USART6_DMA)
	.dat		= &usart6_dat,
#endif
#ifdef CONFIG_USART6_DMA
	.rx_dma		= &dma2_s1,
	.tx_dma		= &dma2_s6,
#endif
};
const struct uart_port usart6 =
{
	.ops		= &f4_uart_ops,
	.ctx		= &usart6_ctx,
	.name		= "usart6",
};
#if defined(CONFIG_USART6_IRQ) || defined(CONFIG_USART6_DMA)
void USART6_IRQHandler(void)
{
	f4_uart_isr(&usart6);
}
#endif
#endif	//	USART6

#if defined(UART7) && defined(CONFIG_UART7)
#if defined(CONFIG_UART7_IRQ) || defined(CONFIG_UART7_DMA)
static struct uart_dat uart7_dat;
#endif
#ifdef CONFIG_UART7_DMA
DMA1_STREAM_OWN(3, 5, &UART7->DR, DMA_DIR_PERIPH_TO_MEM, DMA_WIDTH_U8, IRQ_PRIO_UART7);
DMA1_STREAM_OWN(1, 5, &UART7->DR, DMA_DIR_MEM_TO_PERIPH, DMA_WIDTH_U8, IRQ_PRIO_UART7);
#endif
static const struct uart_ctx uart7_ctx =
{
	.regs		= UART7,
#if defined(CONFIG_UART7_IRQ) || defined(CONFIG_UART7_DMA)
	.dat		= &uart7_dat,
#endif
#ifdef CONFIG_UART7_DMA
	.rx_dma		= &dma1_s3,
	.tx_dma		= &dma1_s1,
#endif
};
const struct uart_port uart7 =
{
	.ops		= &f4_uart_ops,
	.ctx		= &uart7_ctx,
	.name		= "uart7",
};
#if defined(CONFIG_UART7_IRQ) || defined(CONFIG_UART7_DMA)
void UART7_IRQHandler(void)
{
	f4_uart_isr(&uart7);
}
#endif
#endif	//	UART7

#if defined(UART8) && defined(CONFIG_UART8)
#if defined(CONFIG_UART8_IRQ) || defined(CONFIG_UART8_DMA)
static struct uart_dat uart8_dat;
#endif
#ifdef CONFIG_UART8_DMA
DMA1_STREAM_OWN(6, 5, &UART8->DR, DMA_DIR_PERIPH_TO_MEM, DMA_WIDTH_U8, IRQ_PRIO_UART8);
DMA1_STREAM_OWN(0, 5, &UART8->DR, DMA_DIR_MEM_TO_PERIPH, DMA_WIDTH_U8, IRQ_PRIO_UART8);
#endif
static const struct uart_ctx uart8_ctx =
{
	.regs		= UART8,
#if defined(CONFIG_UART8_IRQ) || defined(CONFIG_UART8_DMA)
	.dat		= &uart8_dat,
#endif
#ifdef CONFIG_UART8_DMA
	.rx_dma		= &dma1_s6,
	.tx_dma		= &dma1_s0,
#endif
};
const struct uart_port uart8 =
{
	.ops		= &f4_uart_ops,
	.ctx		= &uart8_ctx,
	.name		= "uart8",
};
#if defined(CONFIG_UART8_IRQ) || defined(CONFIG_UART8_DMA)
void UART8_IRQHandler(void)
{
	f4_uart_isr(&uart8);
}
#endif
#endif	//	UART8

#if defined(UART9) && defined(CONFIG_UART9)
#ifdef CONFIG_UART9_DMA
#error "F4 UART9 has no DMA request mapping; CONFIG_UART9_DMA is invalid on F4"
#endif
#if defined(CONFIG_UART9_IRQ)
static struct uart_dat uart9_dat;
#endif
static const struct uart_ctx uart9_ctx =
{
	.regs		= UART9,
#if defined(CONFIG_UART9_IRQ)
	.dat		= &uart9_dat,
#endif
};
const struct uart_port uart9 =
{
	.ops		= &f4_uart_ops,
	.ctx		= &uart9_ctx,
	.name		= "uart9",
};
#if defined(CONFIG_UART9_IRQ)
void UART9_IRQHandler(void)
{
	f4_uart_isr(&uart9);
}
#endif
#endif	//	UART9

#if defined(UART10) && defined(CONFIG_UART10)
#ifdef CONFIG_UART10_DMA
#error "F4 UART10 has no DMA request mapping; CONFIG_UART10_DMA is invalid on F4"
#endif
#if defined(CONFIG_UART10_IRQ)
static struct uart_dat uart10_dat;
#endif
static const struct uart_ctx uart10_ctx =
{
	.regs		= UART10,
#if defined(CONFIG_UART10_IRQ)
	.dat		= &uart10_dat,
#endif
};
const struct uart_port uart10 =
{
	.ops		= &f4_uart_ops,
	.ctx		= &uart10_ctx,
	.name		= "uart10",
};
#if defined(CONFIG_UART10_IRQ)
void UART10_IRQHandler(void)
{
	f4_uart_isr(&uart10);
}
#endif
#endif	//	UART10
#endif
