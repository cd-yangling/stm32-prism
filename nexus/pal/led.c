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

#include "pal/led.h"

int led_init(const struct led_dev *l)
{
	if ((NULL == l) || (NULL == l->ops) || (NULL == l->ops->init))
	{
		return -EINVAL;
	}

	return l->ops->init(l);
}

int led_on(const struct led_dev *l)
{
	if ((NULL == l) || (NULL == l->ops) || (NULL == l->ops->on))
	{
		return -EINVAL;
	}

	return l->ops->on(l);
}

int led_off(const struct led_dev *l)
{
	if ((NULL == l) || (NULL == l->ops) || (NULL == l->ops->off))
	{
		return -EINVAL;
	}

	return l->ops->off(l);
}

int led_toggle(const struct led_dev *l)
{
	if ((NULL == l) || (NULL == l->ops))
	{
		return -EINVAL;
	}

	if (NULL == l->ops->toggle)
	{
		return -ENOSYS;
	}

	return l->ops->toggle(l);
}

int led_set_brightness(const struct led_dev *l, uint8_t v)
{
	if ((NULL == l) || (NULL == l->ops))
	{
		return -EINVAL;
	}

	if (NULL == l->ops->set_brightness)
	{
		return -ENOSYS;
	}

	return l->ops->set_brightness(l, v);
}
