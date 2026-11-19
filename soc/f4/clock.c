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

#include "clock_conf.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_utils.h"

#define	CAT(a,b)	a##b
#define	XCAT(a,b)	CAT(a,b)

_Static_assert(SYSCLK_HZ <= RCC_MAX_FREQUENCY);
_Static_assert((long long)HSE_VALUE * SYSCLK_PLL_N ==
	(long long)SYSCLK_HZ * SYSCLK_PLL_M * SYSCLK_PLL_P);
_Static_assert((long long)HSE_VALUE * SYSCLK_PLL_N ==
	48000000LL * SYSCLK_PLL_M * SYSCLK_PLL_Q);

#if SYSCLK_HZ > 150000000
#define	FLASH_LAT	5
#elif SYSCLK_HZ > 120000000
#define	FLASH_LAT	4
#elif SYSCLK_HZ > 90000000
#define	FLASH_LAT	3
#elif SYSCLK_HZ > 60000000
#define	FLASH_LAT	2
#elif SYSCLK_HZ > 30000000
#define	FLASH_LAT	1
#else
#define	FLASH_LAT	0
#endif

#if SYSCLK_HZ > 84000000
#define	APB1_DIV	LL_RCC_APB1_DIV_4
#elif SYSCLK_HZ > 42000000
#define	APB1_DIV	LL_RCC_APB1_DIV_2
#else
#define	APB1_DIV	LL_RCC_APB1_DIV_1
#endif

#if SYSCLK_HZ > 84000000
#define	APB2_DIV	LL_RCC_APB2_DIV_2
#else
#define	APB2_DIV	LL_RCC_APB2_DIV_1
#endif

void clock_init(void)
{
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

	LL_FLASH_SetLatency(XCAT(LL_FLASH_LATENCY_, FLASH_LAT));
	if (!LL_FLASH_IsPrefetchEnabled())
		LL_FLASH_EnablePrefetch();
	LL_FLASH_EnableInstCache();
	LL_FLASH_EnableDataCache();

	LL_RCC_HSE_Enable();
	while (!LL_RCC_HSE_IsReady()) {}

	LL_RCC_PLL_Disable();
	while (LL_RCC_PLL_IsReady()) {}
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE,
			XCAT(LL_RCC_PLLM_DIV_, SYSCLK_PLL_M),
			SYSCLK_PLL_N,
			XCAT(LL_RCC_PLLP_DIV_, SYSCLK_PLL_P));
	LL_RCC_PLL_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSE,
			XCAT(LL_RCC_PLLM_DIV_, SYSCLK_PLL_M),
			SYSCLK_PLL_N,
			XCAT(LL_RCC_PLLQ_DIV_, SYSCLK_PLL_Q));
	LL_RCC_PLL_Enable();
	while (!LL_RCC_PLL_IsReady()) {}

	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_16);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_16);

	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while (LL_RCC_SYS_CLKSOURCE_STATUS_PLL != LL_RCC_GetSysClkSource()) {}

	LL_RCC_SetAPB1Prescaler(APB1_DIV);
	LL_RCC_SetAPB2Prescaler(APB2_DIV);

	LL_SetSystemCoreClock(SYSCLK_HZ);
}
