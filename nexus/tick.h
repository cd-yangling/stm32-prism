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

#ifndef	__NEXUS_TICK_HEADER__
#define	__NEXUS_TICK_HEADER__

#include <stdint.h>

void tick_init(void);

uint32_t tick_get(void);

uint32_t tick_get_cyc(void);

void delay_ms(uint32_t ms);

void delay_us(uint32_t us);

void delay_cyc(uint32_t cyc);

#ifdef SYSCLK_HZ
static inline uint32_t tick_cyc_per_us(void)
{
	return (uint32_t)(SYSCLK_HZ / 1000000U);
}
#else
uint32_t tick_cyc_per_us(void);
#endif

static inline int tick_expired(uint32_t deadline)
{
	return (int32_t)(tick_get() - deadline) >= 0;
}

static inline int tick_expired_cyc(uint32_t deadline)
{
	return (int32_t)(tick_get_cyc() - deadline) >= 0;
}

#endif	//	__NEXUS_TICK_HEADER__
