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
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_bus.h"

struct dma_slot
{
	const struct dma_chan	*	chan;
	void					(*	cb)(const struct dma_chan *c, enum dma_event_type ev, void *ud);
	void					*	ud;
};

static struct dma_slot dma1_slots[8];
static struct dma_slot dma2_slots[8];

static struct dma_slot *slot_by(DMA_TypeDef *ctrl, uint32_t stream)
{
	if (DMA2 == ctrl)
	{
		return &dma2_slots[stream];
	}

	return &dma1_slots[stream];
}

static IRQn_Type chan_irqn(const struct dma_chan *c)
{
	if (DMA1 == c->ctrl)
	{
		if (7u > c->stream)
		{
			return (IRQn_Type)(DMA1_Stream0_IRQn + c->stream);
		}

		return DMA1_Stream7_IRQn;
	}

	if (5u > c->stream)
	{
		return (IRQn_Type)(DMA2_Stream0_IRQn + c->stream);
	}

	return (IRQn_Type)(DMA2_Stream5_IRQn + (c->stream - 5u));
}

static void clk_enable(const struct dma_chan *c)
{
	if (DMA1 == c->ctrl)
	{
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
	}
	else
	{
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
	}
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

static uint32_t stream_slot(uint32_t stream)
{
	return (((stream & 3u) >> 1) * 16u) + ((stream & 1u) * 6u);
}

static void flags_clear(DMA_TypeDef *ctrl, uint32_t stream)
{
	uint32_t mask = ((uint32_t)DMA_LIFCR_CFEIF0 | DMA_LIFCR_CDMEIF0
		| DMA_LIFCR_CTEIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTCIF0)
		<< stream_slot(stream);

	if (4u > stream)
	{
		ctrl->LIFCR = mask;
	}
	else
	{
		ctrl->HIFCR = mask;
	}
}

int dma_start(const struct dma_chan *c, const struct dma_xfer *x)
{
	struct dma_slot *s = slot_by(c->ctrl, c->stream);
	IRQn_Type irq = chan_irqn(c);
	uint32_t mode;

	if (x->len > 0xFFFFu)
	{
		return -EINVAL;
	}

	NVIC_DisableIRQ(irq);
	NVIC_ClearPendingIRQ(irq);

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

	LL_DMA_DisableStream(c->ctrl, c->stream);
	while (LL_DMA_IsEnabledStream(c->ctrl, c->stream))
	{
	}

	LL_DMA_ConfigTransfer(c->ctrl, c->stream,
			ll_dir(c->dir) | mode | LL_DMA_PERIPH_NOINCREMENT
			| LL_DMA_MEMORY_INCREMENT | ll_width(c->width)
			| LL_DMA_PRIORITY_HIGH);
	LL_DMA_SetChannelSelection(c->ctrl, c->stream, c->chsel << DMA_SxCR_CHSEL_Pos);
	LL_DMA_SetPeriphAddress(c->ctrl, c->stream, (uint32_t)c->par);
	LL_DMA_SetMemoryAddress(c->ctrl, c->stream, (uint32_t)x->buf);
	LL_DMA_SetDataLength(c->ctrl, c->stream, x->len);
	flags_clear(c->ctrl, c->stream);

	if (NULL != x->cb)
	{
		LL_DMA_EnableIT_TC(c->ctrl, c->stream);
		LL_DMA_EnableIT_HT(c->ctrl, c->stream);
		LL_DMA_EnableIT_TE(c->ctrl, c->stream);
		NVIC_SetPriority(irq, c->prio);
		NVIC_EnableIRQ(irq);
	}
	else
	{
		LL_DMA_DisableIT_TC(c->ctrl, c->stream);
		LL_DMA_DisableIT_HT(c->ctrl, c->stream);
		LL_DMA_DisableIT_TE(c->ctrl, c->stream);
	}

	LL_DMA_EnableStream(c->ctrl, c->stream);

	return 0;
}

int dma_stop(const struct dma_chan *c)
{
	LL_DMA_DisableStream(c->ctrl, c->stream);
	while (LL_DMA_IsEnabledStream(c->ctrl, c->stream))
	{
	}

	return 0;
}

uint32_t dma_remain(const struct dma_chan *c)
{
	return LL_DMA_GetDataLength(c->ctrl, c->stream);
}

static void chan_irq(DMA_TypeDef *ctrl, uint32_t stream)
{
	struct dma_slot *s = slot_by(ctrl, stream);
	uint32_t tc = (uint32_t)DMA_LISR_TCIF0 << stream_slot(stream);
	uint32_t sr = (4u > stream) ? ctrl->LISR : ctrl->HISR;
	uint32_t isr = sr & (tc | (tc >> 1u) | (tc >> 2u));
	enum dma_event_type ev = DMA_EV_DONE;

	if (!isr)
	{
		return;
	}

	if (4u > stream)
	{
		ctrl->LIFCR = isr;
	}
	else
	{
		ctrl->HIFCR = isr;
	}

	if (NULL == s->cb)
	{
		return;
	}

	if (!(isr & tc))
	{
		if (isr & (tc >> 1u))
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

#define	DMA_VEC(_ctrl, _n)							\
	void _ctrl##_Stream##_n##_IRQHandler(void)		\
	{												\
		chan_irq(_ctrl, LL_DMA_STREAM_##_n);			\
	}

DMA_VEC(DMA1, 0)
DMA_VEC(DMA1, 1)
DMA_VEC(DMA1, 2)
DMA_VEC(DMA1, 3)
DMA_VEC(DMA1, 4)
DMA_VEC(DMA1, 5)
DMA_VEC(DMA1, 6)
DMA_VEC(DMA1, 7)

DMA_VEC(DMA2, 0)
DMA_VEC(DMA2, 1)
DMA_VEC(DMA2, 2)
DMA_VEC(DMA2, 3)
DMA_VEC(DMA2, 4)
DMA_VEC(DMA2, 5)
DMA_VEC(DMA2, 6)
DMA_VEC(DMA2, 7)
