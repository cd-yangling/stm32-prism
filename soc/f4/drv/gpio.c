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

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "pal/pin.h"

int exti_irq_configure(const struct pin_port *pt, uint8_t pin, uint32_t flags);
int exti_manage_cb(const struct pin_port *pt, struct pin_callback *cb, int set);

struct gpio_ctx
{
	GPIO_TypeDef	*regs;
	uint32_t	 clk_en;
};

static const struct gpio_ctx *ctx_of(const struct pin_port *pt)
{
	return pt->ctx;
}

static const uint32_t llpin[16] =
{
	LL_GPIO_PIN_0,  LL_GPIO_PIN_1,  LL_GPIO_PIN_2,  LL_GPIO_PIN_3,
	LL_GPIO_PIN_4,  LL_GPIO_PIN_5,  LL_GPIO_PIN_6,  LL_GPIO_PIN_7,
	LL_GPIO_PIN_8,  LL_GPIO_PIN_9,  LL_GPIO_PIN_10, LL_GPIO_PIN_11,
	LL_GPIO_PIN_12, LL_GPIO_PIN_13, LL_GPIO_PIN_14, LL_GPIO_PIN_15,
};

static void gpio_configure(const struct pin_port *pt, uint8_t pin, uint32_t flags)
{
	const struct gpio_ctx *x = ctx_of(pt);
	uint32_t dir	= flags & PIN_DIR_MASK;
	uint32_t mode	= LL_GPIO_MODE_INPUT;
	uint32_t speed	= LL_GPIO_SPEED_FREQ_LOW;
	uint32_t otype	= LL_GPIO_OUTPUT_PUSHPULL;
	uint32_t pull	= LL_GPIO_PULL_NO;

	switch (dir)
	{
	case PIN_OUT:
	{
		mode = LL_GPIO_MODE_OUTPUT;
		break;
	}
	case PIN_ALT:
	{
		mode = LL_GPIO_MODE_ALTERNATE;
		break;
	}
	case PIN_ANALOG:
	{
		mode = LL_GPIO_MODE_ANALOG;
		break;
	}
	default:
	{
		break;
	}
	}

	if (flags & PIN_PULL_UP)
	{
		pull = LL_GPIO_PULL_UP;
	}
	else if (flags & PIN_PULL_DOWN)
	{
		pull = LL_GPIO_PULL_DOWN;
	}

	switch (flags & PIN_SPEED_MASK)
	{
	case PIN_SPEED_MID:
	{
		speed = LL_GPIO_SPEED_FREQ_MEDIUM;
		break;
	}
	case PIN_SPEED_HIGH:
	{
		speed = LL_GPIO_SPEED_FREQ_HIGH;
		break;
	}
	case PIN_SPEED_VERY_HIGH:
	{
		speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		break;
	}
	default:
	{
		break;
	}
	}

	if (flags & PIN_OD)
	{
		otype = LL_GPIO_OUTPUT_OPENDRAIN;
	}

	LL_AHB1_GRP1_EnableClock(x->clk_en);

	//	INIT 电平在模式切换
	if (flags & (PIN_INIT_ACTIVE | PIN_INIT_INACTIVE))
	{
		int on = (0 != (flags & PIN_INIT_ACTIVE));

		if (0 != (flags & PIN_ACTIVE_LOW))
		{
			on = !on;
		}

		if (on)
		{
			LL_GPIO_SetOutputPin(x->regs, llpin[pin]);
		}
		else
		{
			LL_GPIO_ResetOutputPin(x->regs, llpin[pin]);
		}
	}

	if (PIN_IN == dir)
	{
		LL_GPIO_SetPinPull(x->regs, llpin[pin], pull);
	}
	else
	{
		LL_GPIO_SetPinPull(x->regs, llpin[pin], LL_GPIO_PULL_NO);
	}

	if ((PIN_OUT == dir) || (PIN_ALT == dir))
	{
		uint32_t af = (flags & PIN_MUX_MASK) >> 10;

		LL_GPIO_SetPinSpeed(x->regs, llpin[pin], speed);
		LL_GPIO_SetPinOutputType(x->regs, llpin[pin], otype);

		if (PIN_ALT == dir)
		{
			if (8 > pin)
			{
				LL_GPIO_SetAFPin_0_7(x->regs, llpin[pin], af);
			}
			else
			{
				LL_GPIO_SetAFPin_8_15(x->regs, llpin[pin], af);
			}
		}
	}

	LL_GPIO_SetPinMode(x->regs, llpin[pin], mode);
}

static void gpio_toggle(const struct pin_port *pt, uint8_t pin)
{
	LL_GPIO_TogglePin(ctx_of(pt)->regs, llpin[pin]);
}

static void gpio_set(const struct pin_port *pt, uint8_t pin, int on)
{
	GPIO_TypeDef *regs = ctx_of(pt)->regs;

	if (on)
	{
		LL_GPIO_SetOutputPin(regs, llpin[pin]);
	}
	else
	{
		LL_GPIO_ResetOutputPin(regs, llpin[pin]);
	}
}

static int gpio_get(const struct pin_port *pt, uint8_t pin)
{
	if (LL_GPIO_IsInputPinSet(ctx_of(pt)->regs, llpin[pin]))
	{
		return 1;
	}

	return 0;
}

static const struct pin_ops gpio_ops =
{
	.configure		= gpio_configure,
	.toggle			= gpio_toggle,
	.set			= gpio_set,
	.get			= gpio_get,
	.irq_configure	= exti_irq_configure,
	.manage_cb		= exti_manage_cb,
};

#if defined(GPIOA_PINS) && (0 != GPIOA_PINS)
static const struct gpio_ctx gpioa_ctx = { GPIOA, LL_AHB1_GRP1_PERIPH_GPIOA };
const struct pin_port gpioa_port = { &gpio_ops, &gpioa_ctx, "gpioa" };
#endif
#if defined(GPIOB_PINS) && (0 != GPIOB_PINS)
static const struct gpio_ctx gpiob_ctx = { GPIOB, LL_AHB1_GRP1_PERIPH_GPIOB };
const struct pin_port gpiob_port = { &gpio_ops, &gpiob_ctx, "gpiob" };
#endif
#if defined(GPIOC_PINS) && (0 != GPIOC_PINS)
static const struct gpio_ctx gpioc_ctx = { GPIOC, LL_AHB1_GRP1_PERIPH_GPIOC };
const struct pin_port gpioc_port = { &gpio_ops, &gpioc_ctx, "gpioc" };
#endif
#if defined(GPIOD_PINS) && (0 != GPIOD_PINS)
static const struct gpio_ctx gpiod_ctx = { GPIOD, LL_AHB1_GRP1_PERIPH_GPIOD };
const struct pin_port gpiod_port = { &gpio_ops, &gpiod_ctx, "gpiod" };
#endif
#if defined(GPIOH_PINS) && (0 != GPIOH_PINS)
static const struct gpio_ctx gpioh_ctx = { GPIOH, LL_AHB1_GRP1_PERIPH_GPIOH };
const struct pin_port gpioh_port = { &gpio_ops, &gpioh_ctx, "gpioh" };
#endif
