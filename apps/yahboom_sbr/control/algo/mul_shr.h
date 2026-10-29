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

#ifndef	__APPS_YAHBOOM_SBR_CONTROL_ALGO_MUL_SHR_HEADER__
#define	__APPS_YAHBOOM_SBR_CONTROL_ALGO_MUL_SHR_HEADER__

#include <stddef.h>
#include <stdint.h>

static inline __attribute__((always_inline)) int32_t mul_shr_carry(int32_t *rem,
		int32_t v, int32_t num, int shr)
{
	int32_t	p = v * num;
	int32_t	acc = p + ((NULL != rem) ? *rem : 0);
	int32_t	h = ((int32_t)1) << (shr - 1);
	int32_t	q = (0 <= acc) ? ((acc + h) >> shr) : -((-acc + h) >> shr);

	if (NULL != rem)
	{
		*rem = acc - q * (((int32_t)1) << shr);
	}
	return q;
}

static inline __attribute__((always_inline)) int32_t mul_shr(int32_t v, int32_t num, int shr)
{
	return mul_shr_carry(NULL, v, num, shr);
}

#endif	//	__APPS_YAHBOOM_SBR_CONTROL_ALGO_MUL_SHR_HEADER__
