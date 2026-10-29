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

#if defined(CONFIG_SPI1) || defined(CONFIG_SPI2) || defined(CONFIG_SPI3)
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_gpio.h"
#include "pal/spi.h"
#include "pal/pin.h"
#include "tick.h"

#define	SPI_TMO_MS	(100u)

struct spi_dat
{
	uint32_t		cur_hz;
	uint8_t			cur_mode;
	uint8_t			cur_lsb;
};

struct spi_ctx
{
	SPI_TypeDef		*regs;
	struct spi_dat	*data;
};

static int f1_SPIe_detect(const struct spi_ctx *c, uint32_t t0)
{
	if (LL_SPI_IsActiveFlag_OVR(c->regs))
	{
		LL_SPI_ClearFlag_OVR(c->regs);
		return -EIO;
	}

	if (LL_SPI_IsActiveFlag_MODF(c->regs))
	{
		LL_SPI_ClearFlag_MODF(c->regs);
		LL_SPI_Enable(c->regs);
		return -EIO;
	}

	if ((tick_get() - t0) >= SPI_TMO_MS)
		return -ETIMEDOUT;

	return 0;
}

static int f1_wait_txe(const struct spi_ctx *c, uint32_t t0)
{
	int ret = 0;

	while ((!LL_SPI_IsActiveFlag_TXE(c->regs)) && (0 == (ret = f1_SPIe_detect(c, t0)))) { }

	return ret;
}

static int f1_wait_rxne(const struct spi_ctx *c, uint32_t t0)
{
	int ret = 0;

	while ((!LL_SPI_IsActiveFlag_RXNE(c->regs)) && (0 == (ret = f1_SPIe_detect(c, t0)))) { }

	return ret;
}

static int f1_wait_bsy(const struct spi_ctx *c, uint32_t t0)
{
	int ret = 0;

	while ((LL_SPI_IsActiveFlag_BSY(c->regs)) && (0 == (ret = f1_SPIe_detect(c, t0)))) { }

	return ret;
}

static int f1_spi_xfer(const struct spi_bus *b, const uint8_t *tx, uint8_t *rx, uint16_t len)
{
	const struct spi_ctx *c = b->ctx;
	uint32_t t0 = tick_get();
	int ret = 0;

	if (LL_SPI_IsActiveFlag_RXNE(c->regs))
	{
		(void)LL_SPI_ReceiveData8(c->regs);
	}

	for (int i = 0; i < len; i++)
	{
		ret = f1_wait_txe(c, t0);
		if (ret)
			return ret;

		LL_SPI_TransmitData8(c->regs, (NULL == tx) ? 0xFF : tx[i]);

		ret = f1_wait_rxne(c, t0);
		if (ret)
			return ret;

		uint8_t v = (uint8_t)LL_SPI_ReceiveData8(c->regs);

		if (NULL != rx)
			rx[i] = v;
	}

	return f1_wait_bsy(c, t0);
}

static uint32_t f1_spi_pclk(const struct spi_ctx *c)
{
	LL_RCC_ClocksTypeDef rcc_clock;

	LL_RCC_GetSystemClocksFreq(&rcc_clock);

	if (SPI1 == c->regs)
		return rcc_clock.PCLK2_Frequency;

	return rcc_clock.PCLK1_Frequency;
}

static void f1_spi_config(const struct spi_bus *b, const struct spi_dev *d)
{
	const struct spi_ctx *c = b->ctx;
	static const uint32_t div_tab[8] = { 2, 4, 8, 16, 32, 64, 128, 256 };
	static const uint32_t presc_tab[8] =
	{
		LL_SPI_BAUDRATEPRESCALER_DIV2, LL_SPI_BAUDRATEPRESCALER_DIV4,
		LL_SPI_BAUDRATEPRESCALER_DIV8, LL_SPI_BAUDRATEPRESCALER_DIV16,
		LL_SPI_BAUDRATEPRESCALER_DIV32, LL_SPI_BAUDRATEPRESCALER_DIV64,
		LL_SPI_BAUDRATEPRESCALER_DIV128, LL_SPI_BAUDRATEPRESCALER_DIV256,
	};
	uint32_t pclk = f1_spi_pclk(c);
	int i = 0;

	if ((d->max_hz == c->data->cur_hz) && (d->mode == c->data->cur_mode)
			&& (d->lsb_first == c->data->cur_lsb))
		return;

	while ((i < 7) && (pclk / div_tab[i] > d->max_hz))
		i++;

	LL_SPI_Disable(c->regs);

	LL_SPI_SetBaudRatePrescaler(c->regs, presc_tab[i]);
	LL_SPI_SetClockPolarity(c->regs, (d->mode & 0x2) ? LL_SPI_POLARITY_HIGH : LL_SPI_POLARITY_LOW);
	LL_SPI_SetClockPhase(c->regs, (d->mode & 0x1) ? LL_SPI_PHASE_2EDGE : LL_SPI_PHASE_1EDGE);
	LL_SPI_SetTransferBitOrder(c->regs, d->lsb_first ? LL_SPI_LSB_FIRST : LL_SPI_MSB_FIRST);

	LL_SPI_Enable(c->regs);

	c->data->cur_hz = pclk / div_tab[i];
	c->data->cur_mode = d->mode;
	c->data->cur_lsb = d->lsb_first;
}

static int f1_spi_init(const struct spi_bus *b, const struct spi_pins *pins)
{
	const struct spi_ctx *c = b->ctx;
	uint32_t mux = pins->sck->flags & PIN_MUX_MASK;

	if (SPI1 == c->regs)
	{
		if (mux)
		{
			LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
			LL_GPIO_AF_EnableRemap_SPI1();
		}
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
	}
	else if (SPI3 == c->regs)
	{
#if defined(AFIO_MAPR_SPI3_REMAP)
		if (mux)
		{
			LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
			LL_GPIO_AF_EnableRemap_SPI3();
		}
#endif
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
	}
	else
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
	}

	pin_configure(pins->sck);
	pin_configure(pins->miso);
	pin_configure(pins->mosi);

	LL_SPI_Disable(c->regs);

	LL_SPI_SetMode(c->regs, LL_SPI_MODE_MASTER);
	LL_SPI_SetTransferDirection(c->regs, LL_SPI_FULL_DUPLEX);
	LL_SPI_SetDataWidth(c->regs, LL_SPI_DATAWIDTH_8BIT);

	LL_SPI_SetNSSMode(c->regs, LL_SPI_NSS_SOFT);
	LL_SPI_SetBaudRatePrescaler(c->regs, LL_SPI_BAUDRATEPRESCALER_DIV64);
	LL_SPI_SetClockPolarity(c->regs, LL_SPI_POLARITY_LOW);
	LL_SPI_SetClockPhase(c->regs, LL_SPI_PHASE_1EDGE);
	LL_SPI_SetTransferBitOrder(c->regs, LL_SPI_MSB_FIRST);

	LL_SPI_Enable(c->regs);

	c->data->cur_hz = f1_spi_pclk(c) / 64;
	c->data->cur_mode = 0;
	c->data->cur_lsb = 0;

	return 0;
}

static const struct spi_ops spid_ops =
{
	.init		= f1_spi_init,
	.config		= f1_spi_config,
	.xfer		= f1_spi_xfer,
};

#if defined(SPI1) && defined(CONFIG_SPI1)
static struct spi_dat spi1_dat =
{
	.cur_hz		= 0,
	.cur_mode	= 0,
	.cur_lsb	= 0,
};
static const struct spi_ctx spi1_ctx =
{
	.regs		= SPI1,
	.data		= &spi1_dat
};
const struct spi_bus spi1 =
{
	.ops		= &spid_ops,
	.ctx		= &spi1_ctx,
	.name		= "spi1",
};
#endif	//	SPI1

#if defined(SPI2) && defined(CONFIG_SPI2)
static struct spi_dat spi2_dat =
{
	.cur_hz		= 0,
	.cur_mode	= 0,
	.cur_lsb	= 0,
};
static const struct spi_ctx spi2_ctx =
{
	.regs		= SPI2,
	.data		= &spi2_dat,
};
const struct spi_bus spi2 =
{
	.ops		= &spid_ops,
	.ctx		= &spi2_ctx,
	.name		= "spi2",
};
#endif	//	SPI2

#if defined(SPI3) && defined(CONFIG_SPI3)
static struct spi_dat spi3_dat =
{
	.cur_hz		= 0,
	.cur_mode	= 0,
	.cur_lsb	= 0,
};
static const struct spi_ctx spi3_ctx =
{
	.regs		= SPI3,
	.data		= &spi3_dat,
};
const struct spi_bus spi3 =
{
	.ops		= &spid_ops,
	.ctx		= &spi3_ctx,
	.name		= "spi3",
};
#endif	//	SPI3
#endif
