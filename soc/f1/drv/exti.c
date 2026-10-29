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

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_gpio.h"
#include <stddef.h>
#include <errno.h>
#include "pal/pin.h"
#include "irq.h"

static struct pin_callback	*line_cb[16];
static const struct pin_port	*line_port[16];

static IRQn_Type line_irqn(uint8_t line)
{
	if (5u > line)
	{
		return (IRQn_Type)(EXTI0_IRQn + line);
	}

	if (10u > line)
	{
		return EXTI9_5_IRQn;
	}

	return EXTI15_10_IRQn;
}

static uint32_t line_prio(uint8_t line)
{
	switch (line)
	{
	case 0u:
	{
		return IRQ_PRIO_EXTI0;
	}
	case 1u:
	{
		return IRQ_PRIO_EXTI1;
	}
	case 2u:
	{
		return IRQ_PRIO_EXTI2;
	}
	case 3u:
	{
		return IRQ_PRIO_EXTI3;
	}
	case 4u:
	{
		return IRQ_PRIO_EXTI4;
	}
	case 5u:
	case 6u:
	case 7u:
	case 8u:
	case 9u:
	{
		return IRQ_PRIO_EXTI9_5;
	}
	default:
	{
		return IRQ_PRIO_EXTI15_10;
	}
	}
}

static uint8_t port_index(const struct pin_port *pt)
{
	const GPIO_TypeDef * const *regs = pt->ctx;

	return (uint8_t)(((uint32_t)(uintptr_t)*regs - GPIOA_BASE) >> 10);
}

static void exticr_set(uint8_t line, uint8_t port_idx)
{
	uint32_t idx = line >> 2;
	uint32_t shift = (line & 3u) << 2;

	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
	AFIO->EXTICR[idx] = (AFIO->EXTICR[idx] & ~(0xFu << shift))
		| ((uint32_t)port_idx << shift);
}

int exti_manage_cb(const struct pin_port *pt, struct pin_callback *cb, int set)
{
	uint8_t line = cb->pin;

	if (set)
	{
		if (NULL != line_cb[line])
		{
			return (cb == line_cb[line]) ? -EINVAL : -EBUSY;
		}

		if ((NULL != line_port[line]) && (pt != line_port[line]))
		{
			return -EBUSY;
		}

		if (NULL == line_port[line])
		{
			exticr_set(line, port_index(pt));
			line_port[line] = pt;
		}

		line_cb[line] = cb;
		return 0;
	}

	if (cb != line_cb[line])
	{
		return -EINVAL;
	}

	line_cb[line] = NULL;
	return 0;
}

int exti_irq_configure(const struct pin_port *pt, uint8_t pin, uint32_t flags)
{
	uint32_t mask = 1u << pin;

	(void)pt;

	if (PIN_INT_DISABLE == flags)
	{
		LL_EXTI_DisableIT_0_31(mask);
		LL_EXTI_ClearFlag_0_31(mask);
		return 0;
	}

	if (!(flags & PIN_INT_BOTH))
	{
		return -EINVAL;
	}

	if (flags & PIN_INT_RISING)
	{
		LL_EXTI_EnableRisingTrig_0_31(mask);
	}
	else
	{
		LL_EXTI_DisableRisingTrig_0_31(mask);
	}

	if (flags & PIN_INT_FALLING)
	{
		LL_EXTI_EnableFallingTrig_0_31(mask);
	}
	else
	{
		LL_EXTI_DisableFallingTrig_0_31(mask);
	}

	NVIC_SetPriority(line_irqn(pin), line_prio(pin));
	NVIC_EnableIRQ(line_irqn(pin));
	LL_EXTI_EnableIT_0_31(mask);

	return 0;
}

static void exti_pend(uint8_t line)
{
	uint32_t bit = 1u << line;

	LL_EXTI_ClearFlag_0_31(bit);

	if (NULL != line_cb[line])
	{
		line_cb[line]->handler(line_port[line], line_cb[line]);
	}
}

static void exti_pend_range(uint8_t start, uint8_t len)
{
	uint32_t pending = EXTI->PR;

	for (uint8_t line = start; line < (uint8_t)(start + len); line++)
	{
		if (pending & (1u << line))
		{
			exti_pend(line);
		}
	}
}

void EXTI0_IRQHandler(void)
{
	exti_pend(0u);
}

void EXTI1_IRQHandler(void)
{
	exti_pend(1u);
}

void EXTI2_IRQHandler(void)
{
	exti_pend(2u);
}

void EXTI3_IRQHandler(void)
{
	exti_pend(3u);
}

void EXTI4_IRQHandler(void)
{
	exti_pend(4u);
}

void EXTI9_5_IRQHandler(void)
{
	exti_pend_range(5u, 5u);
}

void EXTI15_10_IRQHandler(void)
{
	exti_pend_range(10u, 6u);
}
