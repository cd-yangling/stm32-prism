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

#if defined(CONFIG_ENC_TIM1) || defined(CONFIG_ENC_TIM2) || defined(CONFIG_ENC_TIM3) || \
	defined(CONFIG_ENC_TIM4) || defined(CONFIG_ENC_TIM5) || defined(CONFIG_ENC_TIM8)

#include <stdint.h>
#include <errno.h>

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"

#include "pal/pin.h"
#include "pal/enc.h"

#define	TIM_BUS_APB1	((uint8_t)0)
#define	TIM_BUS_APB2	((uint8_t)1)

struct tim_enc_dat
{
	int32_t		acc;
	uint16_t	last;
};

struct tim_enc_ctx
{
	TIM_TypeDef			*regs;
	struct tim_enc_dat	*dat;
	uint32_t			rcc_en;
	uint8_t				bus;
};

static int f1_tim_enc_remap(TIM_TypeDef *t, const struct enc_ch *c)
{
	uint32_t mux = (c->pin_a->flags & PIN_MUX_MASK) >> 10;

	if (TIM3 == t)
	{
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
		if (2u == mux)
		{
			LL_GPIO_AF_EnableRemap_TIM3();
		}
		else if (1u == mux)
		{
			LL_GPIO_AF_RemapPartial_TIM3();
		}
		else
		{
			LL_GPIO_AF_DisableRemap_TIM3();
		}
		return 0;
	}
	if (0u != mux)
	{
		return -EINVAL;
	}
	return 0;
}

static int f1_tim_enc_init(const struct enc_dev *e, const struct enc_ch *c)
{
	const struct tim_enc_ctx *ctx = e->ctx;
	TIM_TypeDef *t = ctx->regs;
	int r;

	if ((c->mode < ENC_MODE_X2_CH2) || (c->mode > ENC_MODE_X4))
	{
		return -EINVAL;
	}

	if ((c->filter > 15u) || (c->ckd > ENC_CKD_DIV4))
	{
		return -EINVAL;
	}

	r = f1_tim_enc_remap(t, c);
	if (0 != r)
	{
		return r;
	}

	pin_configure(c->pin_a);
	pin_configure(c->pin_b);

	if (TIM_BUS_APB2 == ctx->bus)
	{
		LL_APB2_GRP1_EnableClock(ctx->rcc_en);
		LL_APB2_GRP1_ForceReset(ctx->rcc_en);
		LL_APB2_GRP1_ReleaseReset(ctx->rcc_en);
	}
	else
	{
		LL_APB1_GRP1_EnableClock(ctx->rcc_en);
		LL_APB1_GRP1_ForceReset(ctx->rcc_en);
		LL_APB1_GRP1_ReleaseReset(ctx->rcc_en);
	}

	LL_TIM_SetPrescaler(t, 0u);
	LL_TIM_SetAutoReload(t, 0xFFFFu);

	//	配置CH1输入源为直接映射 (IC1 -> TI1)
	LL_TIM_IC_SetActiveInput(t, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);

	//	配置CH1输入源为直接映射 (IC2 -> TI2)
	LL_TIM_IC_SetActiveInput(t, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);

	//	LL 的 CKD/滤波宏 = 寄存器原值预移位,乘基数还原
	LL_TIM_SetClockDivision(t, (uint32_t)c->ckd * TIM_CR1_CKD_0);
	LL_TIM_IC_SetFilter(t, LL_TIM_CHANNEL_CH1,
		(uint32_t)c->filter * LL_TIM_IC_FILTER_FDIV1_N2);
	LL_TIM_IC_SetFilter(t, LL_TIM_CHANNEL_CH2,
		(uint32_t)c->filter * LL_TIM_IC_FILTER_FDIV1_N2);

	LL_TIM_SetEncoderMode(t, (uint32_t)c->mode);

	LL_TIM_CC_EnableChannel(t, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2);

	LL_TIM_EnableCounter(t);

	ctx->dat->acc = 0;
	ctx->dat->last = (uint16_t)LL_TIM_GetCounter(t);

	return 0;
}

static int f1_tim_enc_get(const struct enc_dev *e, int32_t *cnt)
{
	const struct tim_enc_ctx *ctx = e->ctx;
	uint16_t now = (uint16_t)LL_TIM_GetCounter(ctx->regs);

	ctx->dat->acc += (int16_t)(now - ctx->dat->last);
	ctx->dat->last = now;
	*cnt = ctx->dat->acc;

	return 0;
}

static uint16_t f1_tim_enc_get_raw(const struct enc_dev *e)
{
	const struct tim_enc_ctx *ctx = e->ctx;

	return (uint16_t)LL_TIM_GetCounter(ctx->regs);
}

static const struct enc_ops tim_enc_ops =
{
	.init		= f1_tim_enc_init,
	.get_count	= f1_tim_enc_get,
	.get_raw	= f1_tim_enc_get_raw,
};

#if defined(TIM1) && defined(CONFIG_ENC_TIM1)
static struct tim_enc_dat tim1_dat;
static const struct tim_enc_ctx tim1_ctx =
{
	.regs	= TIM1,
	.dat	= &tim1_dat,
	.rcc_en	= LL_APB2_GRP1_PERIPH_TIM1,
	.bus	= TIM_BUS_APB2,
};
const struct enc_dev tim1 =
{
	.ops	= &tim_enc_ops,
	.ctx	= &tim1_ctx,
	.name	= "tim1",
};
#endif	//	TIM1

#if defined(TIM2) && defined(CONFIG_ENC_TIM2)
static struct tim_enc_dat tim2_dat;
static const struct tim_enc_ctx tim2_ctx =
{
	.regs	= TIM2,
	.dat	= &tim2_dat,
	.rcc_en	= LL_APB1_GRP1_PERIPH_TIM2,
	.bus	= TIM_BUS_APB1,
};
const struct enc_dev tim2 =
{
	.ops	= &tim_enc_ops,
	.ctx	= &tim2_ctx,
	.name	= "tim2",
};
#endif	//	TIM2

#if defined(TIM3) && defined(CONFIG_ENC_TIM3)
static struct tim_enc_dat tim3_dat;
static const struct tim_enc_ctx tim3_ctx =
{
	.regs	= TIM3,
	.dat	= &tim3_dat,
	.rcc_en	= LL_APB1_GRP1_PERIPH_TIM3,
	.bus	= TIM_BUS_APB1,
};
const struct enc_dev tim3 =
{
	.ops	= &tim_enc_ops,
	.ctx	= &tim3_ctx,
	.name	= "tim3",
};
#endif	//	TIM3

#if defined(TIM4) && defined(CONFIG_ENC_TIM4)
static struct tim_enc_dat tim4_dat;
static const struct tim_enc_ctx tim4_ctx =
{
	.regs	= TIM4,
	.dat	= &tim4_dat,
	.rcc_en	= LL_APB1_GRP1_PERIPH_TIM4,
	.bus	= TIM_BUS_APB1,
};
const struct enc_dev tim4 =
{
	.ops	= &tim_enc_ops,
	.ctx	= &tim4_ctx,
	.name	= "tim4",
};
#endif	//	TIM4

#if defined(TIM5) && defined(CONFIG_ENC_TIM5)
static struct tim_enc_dat tim5_dat;
static const struct tim_enc_ctx tim5_ctx =
{
	.regs	= TIM5,
	.dat	= &tim5_dat,
	.rcc_en	= LL_APB1_GRP1_PERIPH_TIM5,
	.bus	= TIM_BUS_APB1,
};
const struct enc_dev tim5 =
{
	.ops	= &tim_enc_ops,
	.ctx	= &tim5_ctx,
	.name	= "tim5",
};
#endif	//	TIM5

#if defined(TIM8) && defined(CONFIG_ENC_TIM8)
static struct tim_enc_dat tim8_dat;
static const struct tim_enc_ctx tim8_ctx =
{
	.regs	= TIM8,
	.dat	= &tim8_dat,
	.rcc_en	= LL_APB2_GRP1_PERIPH_TIM8,
	.bus	= TIM_BUS_APB2,
};
const struct enc_dev tim8 =
{
	.ops	= &tim_enc_ops,
	.ctx	= &tim8_ctx,
	.name	= "tim8",
};
#endif	//	TIM8

#endif
