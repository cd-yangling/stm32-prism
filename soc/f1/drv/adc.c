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

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_adc.h"

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

static int f1_adc_init(const struct adc_dev *d)
{
	const struct adc_ctx *ctx = d->ctx;
	ADC_TypeDef *regs = ctx->regs;

	LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSRC_PCLK2_DIV_6);
	LL_APB2_GRP1_EnableClock(ctx->rcc_en);
	LL_APB2_GRP1_ForceReset(ctx->rcc_en);
	LL_APB2_GRP1_ReleaseReset(ctx->rcc_en);
	ctx->dat->busy = 0u;

	LL_ADC_Enable(regs);
	//	等待 tSTAB
	for (volatile uint32_t i = 200u; 0u != i; i--)
	{
	}

	SET_BIT(regs->CR2, ADC_CR2_RSTCAL);
	while (READ_BIT(regs->CR2, ADC_CR2_RSTCAL))
	{
	}

	LL_ADC_StartCalibration(regs);
	while (LL_ADC_IsCalibrationOnGoing(regs))
	{
	}

	LL_ADC_REG_SetTriggerSource(regs, LL_ADC_REG_TRIG_SOFTWARE);

	return 0;
}

//	采样档半周期数(x.5 cyc 乘 2 取整),ADCCLK=12MHz;us 换算乘 24
static const uint16_t acq_half[] = {3, 15, 27, 57, 83, 111, 143, 479};
static const uint32_t acq_ll[] =
{
	LL_ADC_SAMPLINGTIME_1CYCLE_5,	LL_ADC_SAMPLINGTIME_7CYCLES_5,
	LL_ADC_SAMPLINGTIME_13CYCLES_5,	LL_ADC_SAMPLINGTIME_28CYCLES_5,
	LL_ADC_SAMPLINGTIME_41CYCLES_5,	LL_ADC_SAMPLINGTIME_55CYCLES_5,
	LL_ADC_SAMPLINGTIME_71CYCLES_5,	LL_ADC_SAMPLINGTIME_239CYCLES_5,
};

_Static_assert(ARRAY_SIZE(acq_half) == ARRAY_SIZE(acq_ll), "acq_half/acq_ll length mismatch");

static int f1_adc_setup_channel(const struct adc_dev *d, uint8_t ch, uint16_t acq_us)
{
	const struct adc_ctx *ctx = d->ctx;
	uint32_t ll_st = LL_ADC_SAMPLINGTIME_239CYCLES_5;

	if (17u < ch)
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
		uint32_t need = (uint32_t)acq_us * 24u;
		uint32_t i;

		for (i = 0u; i < ARRAY_SIZE(acq_half); i++)
		{
			if (acq_half[i] >= need)
			{
				break;
			}
		}

		if (ARRAY_SIZE(acq_half) == i)
		{
			return -EINVAL;
		}
		ll_st = acq_ll[i];
	}

	LL_ADC_SetChannelSamplingTime(ctx->regs,
		__LL_ADC_DECIMAL_NB_TO_CHANNEL(ch), ll_st);

	if (ch >= 16u)
	{
		LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ctx->regs),
			LL_ADC_PATH_INTERNAL_VREFINT);
	}

	return 0;
}

static int f1_adc_submit(const struct adc_dev *d, uint8_t ch)
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

static int f1_adc_poll(const struct adc_dev *d, uint16_t *raw)
{
	const struct adc_ctx *ctx = d->ctx;

	if (!LL_ADC_IsActiveFlag_EOS(ctx->regs))
	{
		return -EAGAIN;
	}

	*raw = LL_ADC_REG_ReadConversionData12(ctx->regs);
	ctx->dat->busy = 0u;

	return 0;
}

static const struct adc_ops adc_ops =
{
	.init			= f1_adc_init,
	.setup_channel	= f1_adc_setup_channel,
	.submit			= f1_adc_submit,
	.poll			= f1_adc_poll,
};

#if defined(ADC1) && defined(CONFIG_ADC1)
static struct adc_dat adc1_dat;
static const struct adc_ctx adc1_ctx =
{
	.regs	= ADC1,
	.dat	= &adc1_dat,
	.rcc_en	= LL_APB2_GRP1_PERIPH_ADC1,
	.ch_mask	= 0x3FFFFu,
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
