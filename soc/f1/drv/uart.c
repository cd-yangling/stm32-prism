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
	|| defined(CONFIG_UART4) || defined(CONFIG_UART5)
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_gpio.h"
#include "pal/uart.h"
#include "pal/pin.h"
#include "tick.h"
#include "irq.h"
#ifdef CONFIG_UART_ASYNC_API
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_dma.h"
#endif

#define	UART_TMO_MS	(100u)

#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
struct uart_dat
{
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	uart_irq_cb_t		cb;
	void			*	ud;
#endif
#ifdef CONFIG_UART_ASYNC_API
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
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	struct uart_dat	*dat;
#endif
#ifdef CONFIG_UART_ASYNC_API
	uint32_t		rx_ch;
	uint32_t		tx_ch;
#endif
};

static int f1_uart_init(const struct uart_port *p, const struct uart_pins *pins,
		const struct uart_config *cfg)
{
	const struct uart_ctx *x = p->ctx;
	uint32_t mux = pins->tx->flags & PIN_MUX_MASK;
	LL_RCC_ClocksTypeDef rcc_clock;
	uint32_t pclk;

	LL_RCC_GetSystemClocksFreq(&rcc_clock);

	if (USART1 == x->regs)
	{
		if (mux)
		{
			LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
			LL_GPIO_AF_EnableRemap_USART1();
		}
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
		pclk = rcc_clock.PCLK2_Frequency;
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
		NVIC_SetPriority(USART1_IRQn, IRQ_PRIO_USART1);
		NVIC_EnableIRQ(USART1_IRQn);
#endif
#ifdef CONFIG_UART_ASYNC_API
		NVIC_SetPriority(DMA1_Channel4_IRQn, IRQ_PRIO_USART1);
		NVIC_EnableIRQ(DMA1_Channel4_IRQn);
		NVIC_SetPriority(DMA1_Channel5_IRQn, IRQ_PRIO_USART1);
		NVIC_EnableIRQ(DMA1_Channel5_IRQn);
#endif
	}
	else if (USART2 == x->regs)
	{
		if (mux)
		{
			LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
			LL_GPIO_AF_EnableRemap_USART2();
		}
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
		pclk = rcc_clock.PCLK1_Frequency;
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
		NVIC_SetPriority(USART2_IRQn, IRQ_PRIO_USART2);
		NVIC_EnableIRQ(USART2_IRQn);
#endif
#ifdef CONFIG_UART_ASYNC_API
		NVIC_SetPriority(DMA1_Channel7_IRQn, IRQ_PRIO_USART2);
		NVIC_EnableIRQ(DMA1_Channel7_IRQn);
		NVIC_SetPriority(DMA1_Channel6_IRQn, IRQ_PRIO_USART2);
		NVIC_EnableIRQ(DMA1_Channel6_IRQn);
#endif
	}
	else if (USART3 == x->regs)
	{
		if (mux)
		{
			LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
			LL_GPIO_AF_EnableRemap_USART3();
		}
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
		pclk = rcc_clock.PCLK1_Frequency;
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
		NVIC_SetPriority(USART3_IRQn, IRQ_PRIO_USART3);
		NVIC_EnableIRQ(USART3_IRQn);
#endif
#ifdef CONFIG_UART_ASYNC_API
		NVIC_SetPriority(DMA1_Channel2_IRQn, IRQ_PRIO_USART3);
		NVIC_EnableIRQ(DMA1_Channel2_IRQn);
		NVIC_SetPriority(DMA1_Channel3_IRQn, IRQ_PRIO_USART3);
		NVIC_EnableIRQ(DMA1_Channel3_IRQn);
#endif
	}
#if defined(UART4)
	else if (UART4 == x->regs)
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
		pclk = rcc_clock.PCLK1_Frequency;
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
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
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
		NVIC_SetPriority(UART5_IRQn, IRQ_PRIO_UART5);
		NVIC_EnableIRQ(UART5_IRQn);
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
#ifdef CONFIG_UART_ASYNC_API
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
#endif
	LL_USART_SetTransferDirection(x->regs, LL_USART_DIRECTION_TX_RX);
	LL_USART_SetHWFlowCtrl(x->regs, LL_USART_HWCONTROL_NONE);
	LL_USART_SetBaudRate(x->regs, pclk, cfg->baud);

	LL_USART_Enable(x->regs);

	return 0;
}

static int f1_uart_poll_out(const struct uart_port *p, uint8_t c)
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

static int f1_uart_poll_in(const struct uart_port *p, uint8_t *c)
{
	const struct uart_ctx *x = p->ctx;

	if (!LL_USART_IsActiveFlag_RXNE(x->regs))
		return -EAGAIN;

	*c = (uint8_t)LL_USART_ReceiveData8(x->regs);

	return 0;
}

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static int f1_uart_fifo_fill(const struct uart_port *p, const uint8_t *tx,
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

static int f1_uart_fifo_read(const struct uart_port *p, uint8_t *rx,
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

static int f1_uart_irq_update(const struct uart_port *p)
{
	(void)p;

	return 1;
}

static int f1_uart_irq_callback_set(const struct uart_port *p, uart_irq_cb_t cb, void *ud)
{
	const struct uart_ctx *x = p->ctx;

	x->dat->cb = cb;
	x->dat->ud = ud;

	return 0;
}

static int f1_uart_irq_is_pending(const struct uart_port *p)
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

static int f1_uart_irq_tx_ready(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	return LL_USART_IsActiveFlag_TXE(x->regs);
}

static int f1_uart_irq_rx_ready(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	return LL_USART_IsActiveFlag_RXNE(x->regs);
}

static void f1_uart_irq_tx_disable(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	LL_USART_DisableIT_TXE(x->regs);
}

static void f1_uart_irq_rx_disable(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	LL_USART_DisableIT_RXNE(x->regs);
}

static void f1_uart_irq_tx_enable(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	LL_USART_EnableIT_TXE(x->regs);
}

static void f1_uart_irq_rx_enable(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	LL_USART_EnableDirectionRx(x->regs);
	LL_USART_EnableIT_RXNE(x->regs);
}
#endif

#ifdef CONFIG_UART_ASYNC_API
static void f1_uart_harvest(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;
	struct uart_event ev;
	uint32_t cur;

	if ((NULL == d->rx_buf) || (NULL == d->acb))
	{
		return;
	}

	cur = d->rx_len - LL_DMA_GetDataLength(DMA1, x->rx_ch);

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

static int f1_uart_callback_set(const struct uart_port *p, uart_async_cb_t cb,
		void *ud)
{
	const struct uart_ctx *x = p->ctx;

	x->dat->acb = cb;
	x->dat->aud = ud;

	return 0;
}

static int f1_uart_rx_enable(const struct uart_port *p, uint8_t *buf, uint32_t len)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;

	if (0 == x->rx_ch)
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

	LL_DMA_DisableChannel(DMA1, x->rx_ch);
	LL_DMA_ConfigTransfer(DMA1, x->rx_ch,
			LL_DMA_DIRECTION_PERIPH_TO_MEMORY | LL_DMA_MODE_CIRCULAR
			| LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT
			| LL_DMA_PDATAALIGN_BYTE | LL_DMA_MDATAALIGN_BYTE
			| LL_DMA_PRIORITY_HIGH);
	LL_DMA_SetPeriphAddress(DMA1, x->rx_ch, LL_USART_DMA_GetRegAddr(x->regs));
	LL_DMA_SetMemoryAddress(DMA1, x->rx_ch, (uint32_t)buf);
	LL_DMA_SetDataLength(DMA1, x->rx_ch, len);
	LL_DMA_EnableIT_TC(DMA1, x->rx_ch);
	LL_DMA_EnableIT_HT(DMA1, x->rx_ch);
	LL_DMA_EnableIT_TE(DMA1, x->rx_ch);
	DMA1->IFCR = (uint32_t)DMA_IFCR_CGIF1 << ((x->rx_ch - 1u) * 4u);
	LL_DMA_EnableChannel(DMA1, x->rx_ch);
	LL_USART_EnableDMAReq_RX(x->regs);
	LL_USART_EnableIT_IDLE(x->regs);
	LL_USART_EnableDirectionRx(x->regs);

	return 0;
}

static int f1_uart_rx_disable(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;
	struct uart_event ev;

	if (NULL == d->rx_buf)
	{
		return -EINVAL;
	}

	LL_USART_DisableDirectionRx(x->regs);
	LL_USART_DisableIT_IDLE(x->regs);
	LL_USART_DisableDMAReq_RX(x->regs);
	LL_DMA_DisableChannel(DMA1, x->rx_ch);
	f1_uart_harvest(p);
	d->rx_buf = NULL;

	if (NULL != d->acb)
	{
		ev.type = UART_RX_DISABLED;
		d->acb(p, &ev, d->aud);
	}

	return 0;
}

static int f1_uart_tx(const struct uart_port *p, const uint8_t *buf, uint32_t len)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;

	if (0 == x->tx_ch)
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
	LL_DMA_DisableChannel(DMA1, x->tx_ch);
	LL_DMA_ConfigTransfer(DMA1, x->tx_ch,
			LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_MODE_NORMAL
			| LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT
			| LL_DMA_PDATAALIGN_BYTE | LL_DMA_MDATAALIGN_BYTE
			| LL_DMA_PRIORITY_HIGH);
	LL_DMA_SetPeriphAddress(DMA1, x->tx_ch, LL_USART_DMA_GetRegAddr(x->regs));
	LL_DMA_SetMemoryAddress(DMA1, x->tx_ch, (uint32_t)buf);
	LL_DMA_SetDataLength(DMA1, x->tx_ch, len);
	DMA1->IFCR = (uint32_t)DMA_IFCR_CGIF1 << ((x->tx_ch - 1u) * 4u);
	LL_DMA_EnableChannel(DMA1, x->tx_ch);
	LL_USART_EnableDMAReq_TX(x->regs);

	return 0;
}

static int f1_uart_tx_abort(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;
	struct uart_event ev;
	const uint8_t *b;
	uint32_t sent;

	if (NULL == d->tx_buf)
	{
		return -ENODEV;
	}

	LL_USART_DisableDMAReq_TX(x->regs);
	LL_USART_DisableIT_TC(x->regs);
	LL_DMA_DisableChannel(DMA1, x->tx_ch);
	b = d->tx_buf;
	sent = d->tx_len - LL_DMA_GetDataLength(DMA1, x->tx_ch);
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

#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
static void f1_uart_isr(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;
	struct uart_dat *d = x->dat;
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	if (NULL != d->cb)
	{
		d->cb(p, d->ud);
	}
#endif
#ifdef CONFIG_UART_ASYNC_API
	if ((LL_USART_IsActiveFlag_IDLE(x->regs))
		&& (LL_USART_IsEnabledIT_IDLE(x->regs)))
	{
		LL_USART_ClearFlag_IDLE(x->regs);
		f1_uart_harvest(p);
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

#ifdef CONFIG_UART_ASYNC_API
static void f1_uart_dma_rx_isr(const struct uart_port *p)
{
	const struct uart_ctx *x = p->ctx;

	DMA1->IFCR = (uint32_t)DMA_IFCR_CGIF1 << ((x->rx_ch - 1u) * 4u);
	f1_uart_harvest(p);
}
#endif

static const struct uart_ops f1_uart_ops =
{
	.init				= f1_uart_init,
	.poll_out			= f1_uart_poll_out,
	.poll_in			= f1_uart_poll_in,
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	.fifo_fill			= f1_uart_fifo_fill,
	.fifo_read			= f1_uart_fifo_read,
	.irq_update			= f1_uart_irq_update,
	.irq_callback_set	= f1_uart_irq_callback_set,
	.irq_is_pending		= f1_uart_irq_is_pending,
	.irq_tx_ready		= f1_uart_irq_tx_ready,
	.irq_rx_ready		= f1_uart_irq_rx_ready,
	.irq_tx_disable		= f1_uart_irq_tx_disable,
	.irq_rx_disable		= f1_uart_irq_rx_disable,
	.irq_tx_enable		= f1_uart_irq_tx_enable,
	.irq_rx_enable		= f1_uart_irq_rx_enable,
#endif
#ifdef CONFIG_UART_ASYNC_API
	.callback_set		= f1_uart_callback_set,
	.tx					= f1_uart_tx,
	.tx_abort			= f1_uart_tx_abort,
	.rx_enable			= f1_uart_rx_enable,
	.rx_disable			= f1_uart_rx_disable,
#endif
};

#if defined(USART1) && defined(CONFIG_USART1)
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
static struct uart_dat usart1_dat;
#endif
static const struct uart_ctx usart1_ctx =
{
	.regs		= USART1,
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	.dat		= &usart1_dat,
#endif
#ifdef CONFIG_UART_ASYNC_API
	.rx_ch		= LL_DMA_CHANNEL_5,
	.tx_ch		= LL_DMA_CHANNEL_4,
#endif
};
const struct uart_port usart1 =
{
	.ops		= &f1_uart_ops,
	.ctx		= &usart1_ctx,
	.name		= "usart1",
};
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
void USART1_IRQHandler(void)
{
	f1_uart_isr(&usart1);
}
#endif
#ifdef CONFIG_UART_ASYNC_API
void DMA1_Channel5_IRQHandler(void)
{
	f1_uart_dma_rx_isr(&usart1);
}
#endif
#endif	//	USART1

#if defined(USART2) && defined(CONFIG_USART2)
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
static struct uart_dat usart2_dat;
#endif
static const struct uart_ctx usart2_ctx =
{
	.regs		= USART2,
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	.dat		= &usart2_dat,
#endif
#ifdef CONFIG_UART_ASYNC_API
	.rx_ch		= LL_DMA_CHANNEL_6,
	.tx_ch		= LL_DMA_CHANNEL_7,
#endif
};
const struct uart_port usart2 =
{
	.ops		= &f1_uart_ops,
	.ctx		= &usart2_ctx,
	.name		= "usart2",
};
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
void USART2_IRQHandler(void)
{
	f1_uart_isr(&usart2);
}
#endif
#ifdef CONFIG_UART_ASYNC_API
void DMA1_Channel6_IRQHandler(void)
{
	f1_uart_dma_rx_isr(&usart2);
}
#endif
#endif	//	USART2

#if defined(USART3) && defined(CONFIG_USART3)
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
static struct uart_dat usart3_dat;
#endif
static const struct uart_ctx usart3_ctx =
{
	.regs		= USART3,
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	.dat		= &usart3_dat,
#endif
#ifdef CONFIG_UART_ASYNC_API
	.rx_ch		= LL_DMA_CHANNEL_3,
	.tx_ch		= LL_DMA_CHANNEL_2,
#endif
};
const struct uart_port usart3 =
{
	.ops		= &f1_uart_ops,
	.ctx		= &usart3_ctx,
	.name		= "usart3",
};
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
void USART3_IRQHandler(void)
{
	f1_uart_isr(&usart3);
}
#endif
#ifdef CONFIG_UART_ASYNC_API
void DMA1_Channel3_IRQHandler(void)
{
	f1_uart_dma_rx_isr(&usart3);
}
#endif
#endif	//	USART3

#if defined(UART4) && defined(CONFIG_UART4)
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
static struct uart_dat uart4_dat;
#endif
static const struct uart_ctx uart4_ctx =
{
	.regs		= UART4,
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	.dat		= &uart4_dat,
#endif
};
const struct uart_port uart4 =
{
	.ops		= &f1_uart_ops,
	.ctx		= &uart4_ctx,
	.name		= "uart4",
};
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
void UART4_IRQHandler(void)
{
	f1_uart_isr(&uart4);
}
#endif
#endif	//	UART4

#if defined(UART5) && defined(CONFIG_UART5)
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
static struct uart_dat uart5_dat;
#endif
static const struct uart_ctx uart5_ctx =
{
	.regs		= UART5,
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	.dat		= &uart5_dat,
#endif
};
const struct uart_port uart5 =
{
	.ops		= &f1_uart_ops,
	.ctx		= &uart5_ctx,
	.name		= "uart5",
};
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
void UART5_IRQHandler(void)
{
	f1_uart_isr(&uart5);
}
#endif
#endif	//	UART5
#endif
