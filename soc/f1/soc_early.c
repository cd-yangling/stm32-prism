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

#include "stm32f1xx.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"

//	BOARD_SWJ_NO_NJTRST:	SWJ_CFG=001,释放 PB4
//	BOARD_SWJ_NO_JTAG:		SWJ_CFG=010,释放 PA15/PB3/PB4
//	BOARD_SWJ_DISABLE:		SWJ_CFG=100,释放 PA15/PB3/PB4/PA13/PA14(SWD 断)
void soc_early_init(void)
{
#if defined(CONFIG_STM32_ENABLE_DEBUG_SLEEP_STOP)
	DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;
#endif

#if defined(BOARD_SWJ_NO_NJTRST)
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
	LL_GPIO_AF_Remap_SWJ_NONJTRST();
#elif defined(BOARD_SWJ_NO_JTAG)
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
	LL_GPIO_AF_Remap_SWJ_NOJTAG();
#elif defined(BOARD_SWJ_DISABLE)
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
	LL_GPIO_AF_DisableRemap_SWJ();
#endif
}
