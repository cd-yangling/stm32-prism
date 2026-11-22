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

#if defined(CONFIG_ADC1) || defined(CONFIG_ADC2) || defined(CONFIG_ADC3)

#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_adc.h"

#include "pal/adc.h"
#include "pal/pin.h"
#include "lib/gadget/util.h"

extern const struct pin_port gpioa_port;
extern const struct pin_port gpiob_port;
extern const struct pin_port gpioc_port;

struct adc_dat
{
	uint8_t		busy;
};

struct adc_ctx
{
	ADC_TypeDef		*regs;
	struct adc_dat	*dat;
	uint32_t		rcc_en;
	uint32_t		ch_mask;
};

static uint8_t adc_rst_done;

static int f4_adc_init(const struct adc_dev *d)
{
	const struct adc_ctx *ctx = d->ctx;
	ADC_TypeDef *regs = ctx->regs;

	LL_APB2_GRP1_EnableClock(ctx->rcc_en);

	if (!adc_rst_done)
	{
		LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_ADC);
		LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_ADC);
		adc_rst_done = 1u;
	}

	LL_ADC_SetCommonClock(__LL_ADC_COMMON_INSTANCE(regs),
		LL_ADC_CLOCK_SYNC_PCLK_DIV6);
	ctx->dat->busy = 0u;

	LL_ADC_Enable(regs);
	for (volatile uint32_t i = 200u; 0u != i; i--)
	{
	}

	LL_ADC_REG_SetTriggerSource(regs, LL_ADC_REG_TRIG_SOFTWARE);

	return 0;
}

static const uint16_t acq_cyc[] = {3, 15, 28, 56, 84, 112, 144, 480};
static const uint32_t acq_ll[] =
{
	LL_ADC_SAMPLINGTIME_3CYCLES,	LL_ADC_SAMPLINGTIME_15CYCLES,
	LL_ADC_SAMPLINGTIME_28CYCLES,	LL_ADC_SAMPLINGTIME_56CYCLES,
	LL_ADC_SAMPLINGTIME_84CYCLES,	LL_ADC_SAMPLINGTIME_112CYCLES,
	LL_ADC_SAMPLINGTIME_144CYCLES,	LL_ADC_SAMPLINGTIME_480CYCLES,
};

_Static_assert(ARRAY_SIZE(acq_cyc) == ARRAY_SIZE(acq_ll), "acq_cyc/acq_ll length mismatch");

static int f4_adc_setup_channel(const struct adc_dev *d, uint8_t ch, uint16_t acq_us)
{
	const struct adc_ctx *ctx = d->ctx;
	uint32_t ll_st = LL_ADC_SAMPLINGTIME_480CYCLES;

	if (18u < ch)
	{
		return -EINVAL;
	}

	if (0u == (ctx->ch_mask & (1u << ch)))
	{
		return -ENODEV;
	}

	if (16u > ch)
	{
		struct pin_desc pd;
		const struct pin_port *pt;
		uint32_t mask;

		if (8u > ch)
		{
			pt = &gpioa_port;
			pd.pin = ch;
			mask = GPIOA_PINS;
		}
		else if (10u > ch)
		{
			pt = &gpiob_port;
			pd.pin = (uint8_t)(ch - 8u);
			mask = GPIOB_PINS;
		}
		else
		{
			pt = &gpioc_port;
			pd.pin = (uint8_t)(ch - 10u);
			mask = GPIOC_PINS;
		}

		if (0u == (mask & (1u << pd.pin)))
		{
			return -ENODEV;
		}

		pd.port = pt;
		pd.flags = PIN_ANALOG;
		pin_configure(&pd);
	}

	if (0u != acq_us)
	{
		uint32_t need = (uint32_t)acq_us * 14u;
		uint32_t i;

		for (i = 0u; i < ARRAY_SIZE(acq_cyc); i++)
		{
			if (acq_cyc[i] >= need)
			{
				break;
			}
		}

		if (ARRAY_SIZE(acq_cyc) == i)
		{
			return -EINVAL;
		}
		ll_st = acq_ll[i];
	}

	LL_ADC_SetChannelSamplingTime(ctx->regs,
		__LL_ADC_DECIMAL_NB_TO_CHANNEL(ch), ll_st);

	if (16u == ch)
	{
		LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ctx->regs),
			LL_ADC_PATH_INTERNAL_TEMPSENSOR);
	}
	else if (17u == ch)
	{
		LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ctx->regs),
			LL_ADC_PATH_INTERNAL_VREFINT);
	}
	else if (18u == ch)
	{
		LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ctx->regs),
			LL_ADC_PATH_INTERNAL_VBAT);
	}

	return 0;
}

static int f4_adc_submit(const struct adc_dev *d, uint8_t ch)
{
	const struct adc_ctx *ctx = d->ctx;

	if (ctx->dat->busy)
	{
		return -EBUSY;
	}

	LL_ADC_REG_SetSequencerRanks(ctx->regs, LL_ADC_REG_RANK_1,
		__LL_ADC_DECIMAL_NB_TO_CHANNEL(ch));
	LL_ADC_REG_StartConversionSWStart(ctx->regs);
	ctx->dat->busy = 1u;

	return 0;
}

static int f4_adc_poll(const struct adc_dev *d, uint16_t *raw)
{
	const struct adc_ctx *ctx = d->ctx;

	if (!LL_ADC_IsActiveFlag_EOCS(ctx->regs))
	{
		return -EAGAIN;
	}

	*raw = LL_ADC_REG_ReadConversionData12(ctx->regs);
	ctx->dat->busy = 0u;

	return 0;
}

static const struct adc_ops adc_ops =
{
	.init			= f4_adc_init,
	.setup_channel	= f4_adc_setup_channel,
	.submit			= f4_adc_submit,
	.poll			= f4_adc_poll,
};

#if defined(ADC1) && defined(CONFIG_ADC1)
static struct adc_dat adc1_dat;
static const struct adc_ctx adc1_ctx =
{
	.regs	= ADC1,
	.dat	= &adc1_dat,
	.rcc_en	= LL_APB2_GRP1_PERIPH_ADC1,
	.ch_mask	= 0x7FFFFu,
};
const struct adc_dev adc1 =
{
	.ops	= &adc_ops,
	.ctx	= &adc1_ctx,
	.name	= "adc1",
	.ref_mv	= 3300u,
};
#endif

#if defined(ADC2) && defined(CONFIG_ADC2)
static struct adc_dat adc2_dat;
static const struct adc_ctx adc2_ctx =
{
	.regs	= ADC2,
	.dat	= &adc2_dat,
	.rcc_en	= LL_APB2_GRP1_PERIPH_ADC2,
	.ch_mask	= 0xFFFFu,
};
const struct adc_dev adc2 =
{
	.ops	= &adc_ops,
	.ctx	= &adc2_ctx,
	.name	= "adc2",
	.ref_mv	= 3300u,
};
#endif

#if defined(ADC3) && defined(CONFIG_ADC3)
static struct adc_dat adc3_dat;
static const struct adc_ctx adc3_ctx =
{
	.regs	= ADC3,
	.dat	= &adc3_dat,
	.rcc_en	= LL_APB2_GRP1_PERIPH_ADC3,
	.ch_mask	= 0u,
};
const struct adc_dev adc3 =
{
	.ops	= &adc_ops,
	.ctx	= &adc3_ctx,
	.name	= "adc3",
	.ref_mv	= 3300u,
};
#endif

#endif
