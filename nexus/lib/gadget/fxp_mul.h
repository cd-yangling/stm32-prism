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

#ifndef	__NEXUS_LIB_GADGET_FXP_MUL_HEADER__
#define	__NEXUS_LIB_GADGET_FXP_MUL_HEADER__

#include <stddef.h>
#include <stdint.h>

//	参考 https://en.wikipedia.org/wiki/IEEE_754
//	IEEE 754 标准建议的 4舍 6入 5取偶 的原则

static inline __attribute__((always_inline)) int32_t fxp_mul32_rem(int32_t *rem,
		int32_t v, int32_t num, int shr)
{
	int32_t	p;
	int32_t	acc;
	int32_t	h;
	int32_t	mod;
	int32_t	q;
	int32_t	r;

	if ((1 > shr) || (30 < shr))
	{
		return 0;
	}

	p = v * num;
	acc = p + ((NULL != rem) ? *rem : 0);
	h = ((int32_t)1) << (shr - 1);
	mod = (((int32_t)1) << shr) - 1;
	q = acc >> shr;
	r = acc & mod;

	if ((h < r) || ((h == r) && (q & 1)))
	{
		q++;
		r -= mod + 1;
	}

	if (NULL != rem)
	{
		*rem = r;
	}
	return q;
}

static inline __attribute__((always_inline)) int32_t fxp_mul32(int32_t v, int32_t num, int shr)
{
	return fxp_mul32_rem(NULL, v, num, shr);
}

static inline __attribute__((always_inline)) int64_t fxp_mul64_rem(int64_t *rem,
		int64_t v, int64_t num, int shr)
{
	int64_t	p;
	int64_t	acc;
	int64_t	h;
	int64_t	mod;
	int64_t	q;
	int64_t	r;

	if ((1 > shr) || (62 < shr))
	{
		return 0;
	}

	p = v * num;
	acc = p + ((NULL != rem) ? *rem : 0);
	h = ((int64_t)1) << (shr - 1);
	mod = (((int64_t)1) << shr) - 1;
	q = acc >> shr;
	r = acc & mod;

	if ((h < r) || ((h == r) && (q & 1)))
	{
		q++;
		r -= mod + 1;
	}

	if (NULL != rem)
	{
		*rem = r;
	}
	return q;
}

static inline __attribute__((always_inline)) int64_t fxp_mul64(int64_t v, int64_t num, int shr)
{
	return fxp_mul64_rem(NULL, v, num, shr);
}

#endif	//	__NEXUS_LIB_GADGET_FXP_MUL_HEADER__
