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

#include "tick.h"
#include "irq.h"

#ifdef USE_HAL_DRIVER

#if defined(STM32F1)
#include "stm32f1xx_hal.h"
#elif defined(STM32H7)
#include "stm32h7xx_hal.h"
#endif

#else

#if defined(STM32F1)
#include "stm32f1xx.h"
#elif defined(STM32H7)
#include "stm32h7xx.h"
#endif

#endif

#ifdef SYSCLK_HZ
_Static_assert(0 == SYSCLK_HZ % 1000000);
#define	cyc_per_us	(SYSCLK_HZ / 1000000U)
#else
static uint32_t cyc_per_us;
#endif

static void dwt_calibrate(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

#ifndef SYSCLK_HZ
	cyc_per_us = SystemCoreClock / 1000000U;
#endif
}

#ifdef USE_HAL_DRIVER

uint32_t tick_get(void)
{
	return HAL_GetTick();
}

void SysTick_Handler(void)
{
	HAL_IncTick();
}

void tick_init(void)
{
	dwt_calibrate();
}

#else

static volatile uint32_t tick_count;

uint32_t tick_get(void)
{
	return tick_count;
}

void SysTick_Handler(void)
{
	tick_count++;
}

void tick_init(void)
{
#ifdef SYSCLK_HZ
	SysTick_Config(SYSCLK_HZ / 1000U);
#else
	SysTick_Config(SystemCoreClock / 1000U);
#endif
	NVIC_SetPriority(SysTick_IRQn, IRQ_PRIO_SYSTICK);
	dwt_calibrate();
}

#endif

uint32_t tick_get_cyc(void)
{
	return DWT->CYCCNT;
}

void delay_ms(uint32_t ms)
{
	uint32_t until = tick_get() + ms;

	while (!tick_expired(until))
	{
	}
}

void delay_us(uint32_t us)
{
	uint32_t start = DWT->CYCCNT;
	uint32_t cycles = us * cyc_per_us;

	while ((DWT->CYCCNT - start) < cycles)
	{
	}
}

void delay_cyc(uint32_t cyc)
{
	uint32_t start = DWT->CYCCNT;

	while ((DWT->CYCCNT - start) < cyc)
	{
	}
}

#ifndef SYSCLK_HZ
uint32_t tick_cyc_per_us(void)
{
	return cyc_per_us;
}
#endif
