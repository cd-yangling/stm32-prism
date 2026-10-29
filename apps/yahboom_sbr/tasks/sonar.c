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

#if defined(CONFIG_SONAR)

#include <stdint.h>

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_tim.h"

#include "tick.h"
#include "board.h"
#include "pal/pin.h"
#include "tasks/sonar.h"

#define	SONAR_TRIG_US		15u
#define	SONAR_TIMEOUT_MS	50u
#define	SONAR_PERIOD_MS		100u
#define	SONAR_MIN_MM		20u
#define	SONAR_MAX_MM		5600u

enum sonar_task_st
{
	SONAR_TASK_IDLE,
	SONAR_TASK_WAIT,
};

static enum sonar_task_st task_st;
static uint32_t deadline;
static uint16_t dist_mm;
static enum sonar_st status;

static void tim2_cap_init(void)
{
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

	LL_TIM_SetPrescaler(TIM2, 71u);
	LL_TIM_SetAutoReload(TIM2, 0xFFFFu);

	LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH2,
			LL_TIM_ACTIVEINPUT_DIRECTTI);
	LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH2,
			LL_TIM_IC_POLARITY_RISING);
	LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);

	LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH1,
			LL_TIM_ACTIVEINPUT_INDIRECTTI);
	LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1,
			LL_TIM_IC_POLARITY_FALLING);
	LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);

	LL_TIM_SetTriggerInput(TIM2, LL_TIM_TS_TI2FP2);
	LL_TIM_SetSlaveMode(TIM2, LL_TIM_SLAVEMODE_RESET);

	LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);

	LL_TIM_GenerateEvent_UPDATE(TIM2);
	LL_TIM_EnableCounter(TIM2);
}

static void measure_start(void)
{
	LL_TIM_ClearFlag_CC1(TIM2);

	pin_set(board.sonar_trig, 1u);
	delay_us(SONAR_TRIG_US);
	pin_set(board.sonar_trig, 0u);
}

void sonar_init(void)
{
	tim2_cap_init();

	task_st = SONAR_TASK_IDLE;
	deadline = tick_get();
	dist_mm = 0u;
	status = SONAR_NOHW;
}

void sonar_poll(void)
{
	uint32_t us;
	uint32_t mm;

	if (!tick_expired(deadline))
	{
		return;
	}

	switch (task_st)
	{
	case SONAR_TASK_IDLE:
		measure_start();
		task_st = SONAR_TASK_WAIT;
		deadline = tick_get() + SONAR_TIMEOUT_MS;
		break;

	case SONAR_TASK_WAIT:
		if (LL_TIM_IsActiveFlag_CC1(TIM2))
		{
			us = LL_TIM_IC_GetCaptureCH1(TIM2);
			mm = us * 17u / 100u;

			if (SONAR_MIN_MM > mm || SONAR_MAX_MM < mm)
			{
				dist_mm = 0u;
				status = SONAR_NOECHO;
			}
			else
			{
				dist_mm = (uint16_t)mm;
				status = SONAR_OK;
			}
		}
		else
		{
			dist_mm = 0u;
			status = SONAR_NOHW;
		}

		task_st = SONAR_TASK_IDLE;
		deadline = tick_get() + SONAR_PERIOD_MS;
		break;
	}
}

void sonar_get(uint16_t *mm, enum sonar_st *st)
{
	*mm = dist_mm;
	*st = status;
}

#endif
