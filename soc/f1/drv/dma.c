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

#include <stddef.h>
#include <errno.h>
#include "drv/dma.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_bus.h"

struct dma_slot
{
	const struct dma_chan	*	chan;
	void					(*	cb)(const struct dma_chan *c, enum dma_event_type ev, void *ud);
	void					*	ud;
};

static struct dma_slot dma1_slots[7];
#if defined(DMA2)
static struct dma_slot dma2_slots[5];
#endif

static struct dma_slot *slot_by(DMA_TypeDef *ctrl, uint32_t ch)
{
#if defined(DMA2)
	if (DMA2 == ctrl)
	{
		return &dma2_slots[ch - 1u];
	}
#else
	(void)ctrl;
#endif

	return &dma1_slots[ch - 1u];
}

static IRQn_Type chan_irqn(const struct dma_chan *c)
{
#if defined(DMA2)
	if (DMA2 == c->ctrl)
	{
		if (c->ch <= 3u)
		{
			return (IRQn_Type)(DMA2_Channel1_IRQn + (c->ch - 1u));
		}

		return DMA2_Channel4_5_IRQn;
	}
#endif

	return (IRQn_Type)(DMA1_Channel1_IRQn + (c->ch - 1u));
}

static void clk_enable(const struct dma_chan *c)
{
	if (DMA1 == c->ctrl)
	{
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
	}
#if defined(DMA2)
	else
	{
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
	}
#endif
}

static uint32_t ll_dir(enum dma_dir dir)
{
	if (DMA_DIR_PERIPH_TO_MEM == dir)
	{
		return LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
	}

	return LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
}

static uint32_t ll_width(enum dma_width width)
{
	if (DMA_WIDTH_U16 == width)
	{
		return LL_DMA_PDATAALIGN_HALFWORD | LL_DMA_MDATAALIGN_HALFWORD;
	}

	if (DMA_WIDTH_U32 == width)
	{
		return LL_DMA_PDATAALIGN_WORD | LL_DMA_MDATAALIGN_WORD;
	}

	return LL_DMA_PDATAALIGN_BYTE | LL_DMA_MDATAALIGN_BYTE;
}

int dma_start(const struct dma_chan *c, const struct dma_xfer *x)
{
	struct dma_slot *s = slot_by(c->ctrl, c->ch);
	IRQn_Type irq = chan_irqn(c);
	uint32_t mode;

	if (x->len > 0xFFFFu)
	{
		return -EINVAL;
	}

	NVIC_DisableIRQ(irq);
#if defined(DMA2)
	if (DMA2_Channel4_5_IRQn != irq)
#endif
	{
		NVIC_ClearPendingIRQ(irq);
	}

	s->chan = c;
	s->cb = x->cb;
	s->ud = x->ud;

	clk_enable(c);

	if (DMA_MODE_CYCLIC == x->mode)
	{
		mode = LL_DMA_MODE_CIRCULAR;
	}
	else
	{
		mode = LL_DMA_MODE_NORMAL;
	}

	LL_DMA_DisableChannel(c->ctrl, c->ch);
	LL_DMA_ConfigTransfer(c->ctrl, c->ch,
			ll_dir(c->dir) | mode | LL_DMA_PERIPH_NOINCREMENT
			| LL_DMA_MEMORY_INCREMENT | ll_width(c->width)
			| LL_DMA_PRIORITY_HIGH);
	LL_DMA_SetPeriphAddress(c->ctrl, c->ch, (uint32_t)c->par);
	LL_DMA_SetMemoryAddress(c->ctrl, c->ch, (uint32_t)x->buf);
	LL_DMA_SetDataLength(c->ctrl, c->ch, x->len);
	c->ctrl->IFCR = (uint32_t)DMA_IFCR_CGIF1 << ((c->ch - 1u) * 4u);

	if (NULL != x->cb)
	{
		LL_DMA_EnableIT_TC(c->ctrl, c->ch);
		LL_DMA_EnableIT_HT(c->ctrl, c->ch);
		LL_DMA_EnableIT_TE(c->ctrl, c->ch);
		NVIC_SetPriority(irq, c->prio);
		NVIC_EnableIRQ(irq);
	}
	else
	{
		LL_DMA_DisableIT_TC(c->ctrl, c->ch);
		LL_DMA_DisableIT_HT(c->ctrl, c->ch);
		LL_DMA_DisableIT_TE(c->ctrl, c->ch);
	}

	LL_DMA_EnableChannel(c->ctrl, c->ch);

	return 0;
}

int dma_stop(const struct dma_chan *c)
{
	LL_DMA_DisableChannel(c->ctrl, c->ch);

	return 0;
}

uint32_t dma_remain(const struct dma_chan *c)
{
	return LL_DMA_GetDataLength(c->ctrl, c->ch);
}

static void chan_irq(DMA_TypeDef *ctrl, uint32_t ch)
{
	struct dma_slot *s = slot_by(ctrl, ch);
	uint32_t tc = (uint32_t)DMA_ISR_TCIF1 << ((ch - 1u) * 4u);
	uint32_t isr = ctrl->ISR & (tc | (tc << 1u) | (tc << 2u));
	enum dma_event_type ev = DMA_EV_DONE;

	if (!isr)
	{
		return;
	}

	ctrl->IFCR = isr;

	if (NULL == s->cb)
	{
		return;
	}

	if (!(isr & tc))
	{
		if (isr & (tc << 1u))
		{
			ev = DMA_EV_HALF_DONE;
		}
		else
		{
			ev = DMA_EV_ERROR;
		}
	}

	s->cb(s->chan, ev, s->ud);
}

#define	DMA_VEC(_ctrl, _n)						\
	void _ctrl##_Channel##_n##_IRQHandler(void)	\
	{											\
		chan_irq(_ctrl, LL_DMA_CHANNEL_##_n);	\
	}

DMA_VEC(DMA1, 1)
DMA_VEC(DMA1, 2)
DMA_VEC(DMA1, 3)
DMA_VEC(DMA1, 4)
DMA_VEC(DMA1, 5)
DMA_VEC(DMA1, 6)
DMA_VEC(DMA1, 7)

#if defined(DMA2)
DMA_VEC(DMA2, 1)
DMA_VEC(DMA2, 2)
DMA_VEC(DMA2, 3)

void DMA2_Channel4_5_IRQHandler(void)
{
	chan_irq(DMA2, LL_DMA_CHANNEL_4);
	chan_irq(DMA2, LL_DMA_CHANNEL_5);
}
#endif
