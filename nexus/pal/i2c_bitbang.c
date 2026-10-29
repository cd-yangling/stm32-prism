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

#if defined(CONFIG_I2C_BITBANG)
#include <stdint.h>
#include <errno.h>
#include "pal/i2c.h"
#include "pal/pin.h"
#include "pal/i2c_bitbang.h"
#include "tick.h"

//	CONFIG_I2C_BITBANG_FAST 定义这个宏绕开 pal 层的抽象 直接读写寄存器(MPU6050实测读取14字节提升200us)
#ifdef CONFIG_I2C_BITBANG_FAST
#if defined(STM32F1)
#include "stm32f1xx.h"
#elif defined(STM32H7)
#include "stm32h7xx.h"
#endif
#endif

#define	BITBANG_FAST_TH_HZ		(200000u)
#define	BITBANG_STD_LOW_NS		(4700u)
#define	BITBANG_STD_HIGH_NS		(4000u)
#define	BITBANG_FAST_LOW_NS		(1500u)
#define	BITBANG_FAST_HIGH_NS	(800u)

struct i2c_bitbang_dat
{
	uint32_t			low;	//	半周期 拍
	uint32_t			high;	//	半周期 拍
#ifdef CONFIG_I2C_BITBANG_FAST
	GPIO_TypeDef		*scl_regs;
	GPIO_TypeDef		*sda_regs;
	uint32_t			scl_mask;
	uint32_t			sda_mask;
#endif
};

_Static_assert(sizeof(struct i2c_bitbang_dat) == BITBANG_DAT_SIZE, "BITBANG_DAT_SIZE out of sync with struct i2c_bitbang_dat");

static inline __attribute__((always_inline)) void bb_scl(const struct i2c_bitbang_ctx *c, int on)
{
#ifdef CONFIG_I2C_BITBANG_FAST
	uint32_t m = c->dat->scl_mask;

	if (0 != on)
	{
		c->dat->scl_regs->BSRR = m;
	}
	else
	{
		c->dat->scl_regs->BSRR = m << 16;
	}
#else
	pin_set(c->pins->scl, on);
#endif
}

static inline __attribute__((always_inline)) void bb_sda(const struct i2c_bitbang_ctx *c, int on)
{
#ifdef CONFIG_I2C_BITBANG_FAST
	uint32_t m = c->dat->sda_mask;

	if (0 != on)
	{
		c->dat->sda_regs->BSRR = m;
	}
	else
	{
		c->dat->sda_regs->BSRR = m << 16;
	}
#else
	pin_set(c->pins->sda, on);
#endif
}

static inline __attribute__((always_inline)) int bb_sda_read(const struct i2c_bitbang_ctx *c)
{
#ifdef CONFIG_I2C_BITBANG_FAST
	return (0 != (c->dat->sda_regs->IDR & c->dat->sda_mask));
#else
	return pin_get(c->pins->sda);
#endif
}

static void bb_write_bit(const struct i2c_bitbang_ctx *c, int bit)
{
	struct i2c_bitbang_dat *d = c->dat;

	bb_sda(c, bit);
	bb_scl(c, 1);
	delay_cyc(d->high);
	bb_scl(c, 0);
	delay_cyc(d->low);
}

static int bb_read_bit(const struct i2c_bitbang_ctx *c)
{
	struct i2c_bitbang_dat *d = c->dat;
	int bit;

	bb_sda(c, 1);
	bb_scl(c, 1);
	delay_cyc(d->high);

	bit = bb_sda_read(c);

	bb_scl(c, 0);
	delay_cyc(d->low);

	return bit;
}

static int bb_write_byte(const struct i2c_bitbang_ctx *c, uint8_t v)
{
	uint8_t mask = 1u << 7;

	do
	{
		bb_write_bit(c, 0 != (v & mask));
	}
	while (0 != (mask >>= 1));

	return !bb_read_bit(c);
}

static uint8_t bb_read_byte(const struct i2c_bitbang_ctx *c)
{
	uint8_t v = 0;

	for (int i = 0; i < 8; i++)
	{
		v = (uint8_t)((v << 1) | (uint8_t)bb_read_bit(c));
	}

	return v;
}

static void i2c_bitbang_start(const struct i2c_bus *b)
{
	const struct i2c_bitbang_ctx *c = b->ctx;
	struct i2c_bitbang_dat *d = c->dat;

	bb_sda(c, 1);
	bb_scl(c, 1);
	delay_cyc(d->high);
	delay_cyc(d->low);

	if (0 == bb_sda_read(c))
	{
		bb_scl(c, 0);
		delay_cyc(d->low);
		bb_scl(c, 1);
		delay_cyc(d->low);
	}

	bb_sda(c, 0);
	delay_cyc(d->high);

	bb_scl(c, 0);
	delay_cyc(d->low);
}

static void i2c_bitbang_stop(const struct i2c_bus *b)
{
	const struct i2c_bitbang_ctx *c = b->ctx;
	struct i2c_bitbang_dat *d = c->dat;

	bb_sda(c, 0);
	delay_cyc(d->low);

	bb_scl(c, 1);
	delay_cyc(d->high);

	delay_cyc(d->high);
	bb_sda(c, 1);
	delay_cyc(d->low);
}

static int i2c_bitbang_xfer(const struct i2c_bus *b, uint8_t addr7, int rd,
	uint8_t *buf, uint16_t len)
{
	const struct i2c_bitbang_ctx *c = b->ctx;
	uint8_t byte0 = (uint8_t)((addr7 << 1) | (uint8_t)(0 != rd));

	if (0 == bb_write_byte(c, byte0))
	{
		return -ENXIO;
	}

	if (0 == rd)
	{
		for (uint16_t i = 0; i < len; i++)
		{
			if (0 == bb_write_byte(c, buf[i]))
			{
				return -ENXIO;
			}
		}
	}
	else
	{
		for (uint16_t i = 0; i < len; i++)
		{
			buf[i] = bb_read_byte(c);
			bb_write_bit(c, (i + 1u < len) ? 0 : 1);
		}
	}

	return 0;
}

static void i2c_bitbang_config(const struct i2c_bus *b, uint32_t spd_hz)
{
	const struct i2c_bitbang_ctx *c = b->ctx;
	struct i2c_bitbang_dat *d = c->dat;
	uint32_t low_ns;
	uint32_t high_ns;

	if (spd_hz > BITBANG_FAST_TH_HZ)
	{
		low_ns = BITBANG_FAST_LOW_NS;
		high_ns = BITBANG_FAST_HIGH_NS;
	}
	else
	{
		low_ns = BITBANG_STD_LOW_NS;
		high_ns = BITBANG_STD_HIGH_NS;
	}

	d->low = (low_ns * tick_cyc_per_us() + 999u) / 1000u;
	d->high = (high_ns * tick_cyc_per_us() + 999u) / 1000u;
}

static int i2c_bitbang_init(const struct i2c_bus *b, const struct i2c_pins *pins)
{
	const struct i2c_bitbang_ctx *c = b->ctx;
#ifdef CONFIG_I2C_BITBANG_FAST
	struct i2c_bitbang_dat *d = c->dat;
#endif

	(void)pins;

	pin_configure(c->pins->scl);
	pin_configure(c->pins->sda);

#ifdef CONFIG_I2C_BITBANG_FAST
	d->scl_regs = *(GPIO_TypeDef *const *)c->pins->scl->port->ctx;
	d->sda_regs = *(GPIO_TypeDef *const *)c->pins->sda->port->ctx;
	d->scl_mask = 1u << c->pins->scl->pin;
	d->sda_mask = 1u << c->pins->sda->pin;
#endif

	bb_scl(c, 1);
	bb_sda(c, 1);

	i2c_bitbang_config(b, 0);

	return 0;
}

const struct i2c_ops i2c_bitbang_ops =
{
	.init	= i2c_bitbang_init,
	.start	= i2c_bitbang_start,
	.stop	= i2c_bitbang_stop,
	.xfer	= i2c_bitbang_xfer,
	.config	= i2c_bitbang_config,
};
#endif	//	CONFIG_I2C_BITBANG
