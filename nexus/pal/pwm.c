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

int pwm_init(const struct pwm_ch *c)
{
	if ((NULL == c) || (NULL == c->dev) || (NULL == c->pin))
	{
		return -EINVAL;
	}

	return c->dev->ops->init(c->dev, c);
}

int pwm_set(const struct pwm_ch *c, uint32_t period_ns, uint32_t pulse_ns)
{
	uint64_t rate;
	uint64_t period;
	uint64_t pulse;

	if ((NULL == c) || (NULL == c->dev) || (0u == period_ns) || (pulse_ns > period_ns))
	{
		return -EINVAL;
	}

	rate = (uint64_t)pwm_get_cycles_per_sec(c->dev);
	period = rate * (uint64_t)period_ns / 1000000000u;
	pulse = rate * (uint64_t)pulse_ns / 1000000000u;

	if (0u == period)
	{
		return -EINVAL;
	}

	return c->dev->ops->set_cycles(c->dev, c->ch,
		(uint32_t)period, (uint32_t)pulse, c->flags);
}

uint32_t pwm_get_cycles_per_sec(const struct pwm_dev *d)
{
	return d->ops->get_cycles_per_sec(d);
}
