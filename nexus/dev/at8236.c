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

#include <errno.h>
#include <stddef.h>

#include "pal/pwm.h"
#include "dev/at8236.h"

int at8236_init(const struct at8236_dev *m)
{
	int ret;

	if ((NULL == m) || (NULL == m->in1) || (NULL == m->in2) ||
		(0u == m->period_ns) || (AT8236_PERIOD_NS_MAX < m->period_ns))
	{
		return -EINVAL;
	}

	ret = pwm_init(m->in1);
	if (0 != ret)
	{
		return ret;
	}

	ret = pwm_init(m->in2);
	if (0 != ret)
	{
		return ret;
	}

	return at8236_coast(m);
}

int at8236_set(const struct at8236_dev *m, int16_t duty)
{
	const struct pwm_ch	*drv;
	const struct pwm_ch	*oth;
	uint32_t			pulse;
	int					ret;

	if ((NULL == m) || (1000 < duty) || (-1000 > duty))
	{
		return -EINVAL;
	}

	if (0 <= duty)
	{
		drv = m->in1;
		oth = m->in2;
	}
	else
	{
		drv = m->in2;
		oth = m->in1;
		duty = (int16_t)-duty;
	}

	pulse = m->period_ns * (uint32_t)duty / 1000u;

	if (0u != (m->flags & AT8236_DECAY_SLOW))
	{
		ret = pwm_set(drv, m->period_ns, m->period_ns);
		if (0 != ret)
		{
			return ret;
		}

		return pwm_set(oth, m->period_ns, m->period_ns - pulse);
	}

	ret = pwm_set(oth, m->period_ns, 0u);
	if (0 != ret)
	{
		return ret;
	}

	return pwm_set(drv, m->period_ns, pulse);
}

int at8236_brake(const struct at8236_dev *m)
{
	int ret;

	if (NULL == m)
	{
		return -EINVAL;
	}

	ret = pwm_set(m->in1, m->period_ns, m->period_ns);
	if (0 != ret)
	{
		return ret;
	}

	return pwm_set(m->in2, m->period_ns, m->period_ns);
}

int at8236_coast(const struct at8236_dev *m)
{
	int ret;

	if (NULL == m)
	{
		return -EINVAL;
	}

	ret = pwm_set(m->in1, m->period_ns, 0u);
	if (0 != ret)
	{
		return ret;
	}

	return pwm_set(m->in2, m->period_ns, 0u);
}
