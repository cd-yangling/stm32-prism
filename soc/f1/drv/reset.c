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

#include "stm32f1xx_ll_rcc.h"
#include "pal/reset.h"

void reset_cause_consume(uint32_t *cause)
{
	uint32_t f = 0u;

	if (LL_RCC_IsActiveFlag_PINRST())
	{
		f |= RESET_CAUSE_PIN;
	}
	if (LL_RCC_IsActiveFlag_PORRST())
	{
		f |= RESET_CAUSE_POR;
	}
	if (LL_RCC_IsActiveFlag_SFTRST())
	{
		f |= RESET_CAUSE_SOFTWARE;
	}
	if (LL_RCC_IsActiveFlag_IWDGRST())
	{
		f |= RESET_CAUSE_IWDG;
	}
	if (LL_RCC_IsActiveFlag_WWDGRST())
	{
		f |= RESET_CAUSE_WWDG;
	}
	if (LL_RCC_IsActiveFlag_LPWRRST())
	{
		f |= RESET_CAUSE_LPWR;
	}
	LL_RCC_ClearResetFlags();
	*cause = f;
}
