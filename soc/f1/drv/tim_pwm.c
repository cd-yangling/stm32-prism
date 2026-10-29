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

#if defined(CONFIG_PWM_TIM1) || defined(CONFIG_PWM_TIM2) || defined(CONFIG_PWM_TIM3) || \
	defined(CONFIG_PWM_TIM4) || defined(CONFIG_PWM_TIM5) || defined(CONFIG_PWM_TIM8)

#include <stdint.h>
#include <errno.h>

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_tim.h"

#include "pal/pin.h"
#include "pal/pwm.h"

#define	TIM_BUS_APB1	((uint8_t)0)
#define	TIM_BUS_APB2	((uint8_t)1)

#define	TIM_ADV_NO		((uint8_t)0)
#define	TIM_ADV_YES		((uint8_t)1)

struct tim_dat
{
	uint8_t			ch_en;
};

struct tim_ctx
{
	TIM_TypeDef		*regs;
	struct tim_dat	*dat;
	uint32_t		rcc_en;
	uint8_t			bus;
	uint8_t			adv;
};

static const uint32_t tim_ch_map[] =
{
	LL_TIM_CHANNEL_CH1,
	LL_TIM_CHANNEL_CH2,
	LL_TIM_CHANNEL_CH3,
	LL_TIM_CHANNEL_CH4,
};

static uint32_t f1_tim_cycles_per_sec(const struct pwm_dev *d)
{
	const struct tim_ctx *c = d->ctx;
	LL_RCC_ClocksTypeDef rcc_clock;
	uint32_t pclk;

	LL_RCC_GetSystemClocksFreq(&rcc_clock);

	pclk = rcc_clock.PCLK1_Frequency;
	if (TIM_BUS_APB2 == c->bus)
	{
		pclk = rcc_clock.PCLK2_Frequency;
	}

	if (pclk != rcc_clock.HCLK_Frequency)
	{
		pclk *= 2u;
	}

	return pclk;
}

static void f1_tim_set_ccr(TIM_TypeDef *t, uint8_t ch, uint32_t val)
{
	switch (ch)
	{
	case 1u:
		LL_TIM_OC_SetCompareCH1(t, val);
		break;
	case 2u:
		LL_TIM_OC_SetCompareCH2(t, val);
		break;
	case 3u:
		LL_TIM_OC_SetCompareCH3(t, val);
		break;
	case 4u:
		LL_TIM_OC_SetCompareCH4(t, val);
		break;
	default:
		break;
	}
}

static int f1_tim_init(const struct pwm_dev *d, const struct pwm_ch *c)
{
	const struct tim_ctx *ctx = d->ctx;

	pin_configure(c->pin);

	if (TIM_BUS_APB2 == ctx->bus)
	{
		LL_APB2_GRP1_EnableClock(ctx->rcc_en);
		if (0u == ctx->dat->ch_en)
		{
			LL_APB2_GRP1_ForceReset(ctx->rcc_en);
			LL_APB2_GRP1_ReleaseReset(ctx->rcc_en);
		}
	}
	else
	{
		LL_APB1_GRP1_EnableClock(ctx->rcc_en);
		if (0u == ctx->dat->ch_en)
		{
			LL_APB1_GRP1_ForceReset(ctx->rcc_en);
			LL_APB1_GRP1_ReleaseReset(ctx->rcc_en);
		}
	}

	if (TIM_ADV_YES == ctx->adv)
	{
		LL_TIM_EnableAllOutputs(ctx->regs);
	}

	LL_TIM_EnableCounter(ctx->regs);

	return 0;
}

static void f1_tim_ch_init(TIM_TypeDef *t, uint8_t ch, uint32_t flags)
{
	uint32_t polarity;

	polarity = LL_TIM_OCPOLARITY_HIGH;
	if (0u != (flags & PWM_POLARITY_INVERTED))
	{
		polarity = LL_TIM_OCPOLARITY_LOW;
	}

	LL_TIM_OC_SetMode(t, tim_ch_map[ch - 1u], LL_TIM_OCMODE_PWM1);
	LL_TIM_OC_SetPolarity(t, tim_ch_map[ch - 1u], polarity);
	LL_TIM_CC_EnableChannel(t, tim_ch_map[ch - 1u]);
	LL_TIM_OC_EnablePreload(t, tim_ch_map[ch - 1u]);
	LL_TIM_EnableARRPreload(t);
}

static int f1_pwm_set_cycles(const struct pwm_dev *d, uint8_t ch,
		uint32_t period, uint32_t pulse, uint32_t flags)
{
	const struct tim_ctx *c = d->ctx;
	uint32_t psc;
	uint32_t arr;
	uint32_t ccr;

	if ((ch < 1u) || (ch > 4u) || (0u == period) || (pulse > period))
	{
		return -EINVAL;
	}

	psc = period / 65536u;
	arr = (period / (psc + 1u)) - 1u;
	ccr = pulse / (psc + 1u);

	LL_TIM_SetPrescaler(c->regs, psc);
	LL_TIM_SetAutoReload(c->regs, arr);
	f1_tim_set_ccr(c->regs, ch, ccr);

	if (0u == (c->dat->ch_en & (uint8_t)(1u << ch)))
	{
		f1_tim_ch_init(c->regs, ch, flags);

		if (0u == c->dat->ch_en)
		{
			LL_TIM_GenerateEvent_UPDATE(c->regs);
		}

		c->dat->ch_en |= (uint8_t)(1u << ch);
	}

	return 0;
}

static const struct pwm_ops tim_ops =
{
	.init				= f1_tim_init,
	.set_cycles			= f1_pwm_set_cycles,
	.get_cycles_per_sec	= f1_tim_cycles_per_sec,
};

#if defined(TIM1) && defined(CONFIG_PWM_TIM1)
static struct tim_dat tim1_dat =
{
	.ch_en	= 0u,
};
static const struct tim_ctx tim1_ctx =
{
	.regs	= TIM1,
	.dat	= &tim1_dat,
	.rcc_en	= LL_APB2_GRP1_PERIPH_TIM1,
	.bus	= TIM_BUS_APB2,
	.adv	= TIM_ADV_YES,
};
const struct pwm_dev tim1 =
{
	.ops	= &tim_ops,
	.ctx	= &tim1_ctx,
	.name	= "tim1",
};
#endif	//	TIM1

#if defined(TIM2) && defined(CONFIG_PWM_TIM2)
static struct tim_dat tim2_dat =
{
	.ch_en	= 0u,
};
static const struct tim_ctx tim2_ctx =
{
	.regs	= TIM2,
	.dat	= &tim2_dat,
	.rcc_en	= LL_APB1_GRP1_PERIPH_TIM2,
	.bus	= TIM_BUS_APB1,
	.adv	= TIM_ADV_NO,
};
const struct pwm_dev tim2 =
{
	.ops	= &tim_ops,
	.ctx	= &tim2_ctx,
	.name	= "tim2",
};
#endif	//	TIM2

#if defined(TIM3) && defined(CONFIG_PWM_TIM3)
static struct tim_dat tim3_dat =
{
	.ch_en	= 0u,
};
static const struct tim_ctx tim3_ctx =
{
	.regs	= TIM3,
	.dat	= &tim3_dat,
	.rcc_en	= LL_APB1_GRP1_PERIPH_TIM3,
	.bus	= TIM_BUS_APB1,
	.adv	= TIM_ADV_NO,
};
const struct pwm_dev tim3 =
{
	.ops	= &tim_ops,
	.ctx	= &tim3_ctx,
	.name	= "tim3",
};
#endif	//	TIM3

#if defined(TIM4) && defined(CONFIG_PWM_TIM4)
static struct tim_dat tim4_dat =
{
	.ch_en	= 0u,
};
static const struct tim_ctx tim4_ctx =
{
	.regs	= TIM4,
	.dat	= &tim4_dat,
	.rcc_en	= LL_APB1_GRP1_PERIPH_TIM4,
	.bus	= TIM_BUS_APB1,
	.adv	= TIM_ADV_NO,
};
const struct pwm_dev tim4 =
{
	.ops	= &tim_ops,
	.ctx	= &tim4_ctx,
	.name	= "tim4",
};
#endif	//	TIM4

#if defined(TIM5) && defined(CONFIG_PWM_TIM5)
static struct tim_dat tim5_dat =
{
	.ch_en	= 0u,
};
static const struct tim_ctx tim5_ctx =
{
	.regs	= TIM5,
	.dat	= &tim5_dat,
	.rcc_en	= LL_APB1_GRP1_PERIPH_TIM5,
	.bus	= TIM_BUS_APB1,
	.adv	= TIM_ADV_NO,
};
const struct pwm_dev tim5 =
{
	.ops	= &tim_ops,
	.ctx	= &tim5_ctx,
	.name	= "tim5",
};
#endif	//	TIM5

#if defined(TIM8) && defined(CONFIG_PWM_TIM8)
static struct tim_dat tim8_dat =
{
	.ch_en	= 0u,
};
static const struct tim_ctx tim8_ctx =
{
	.regs	= TIM8,
	.dat	= &tim8_dat,
	.rcc_en	= LL_APB2_GRP1_PERIPH_TIM8,
	.bus	= TIM_BUS_APB2,
	.adv	= TIM_ADV_YES,
};
const struct pwm_dev tim8 =
{
	.ops	= &tim_ops,
	.ctx	= &tim8_ctx,
	.name	= "tim8",
};
#endif	//	TIM8

#endif
