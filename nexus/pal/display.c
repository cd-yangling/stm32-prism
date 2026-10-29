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

#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include "pal/display.h"

int display_init(const struct display *d)
{
	return d->ops->init(d);
}

int display_write(const struct display *d, const struct display_area *a, const uint8_t *buf)
{
	const struct display_caps *c = d->caps;
	uint32_t x_end;
	uint32_t y_end;

	if (NULL == a || NULL == buf)
	{
		return -EINVAL;
	}
	x_end = (uint32_t)a->x + a->w;
	y_end = (uint32_t)a->y + a->h;
	if ((0 == a->w) || (0 == a->h) || (a->pitch < a->w))
	{
		return -EINVAL;
	}
	if ((x_end > c->x_res) || (y_end > c->y_res))
	{
		return -EINVAL;
	}
	if ((0 != (a->y & 7u)) || (0 != (a->h & 7u)))
	{
		return -EINVAL;
	}
	return d->ops->write(d, a, buf);
}

int display_blanking_on(const struct display *d)
{
	return d->ops->blanking_on(d);
}

int display_blanking_off(const struct display *d)
{
	return d->ops->blanking_off(d);
}

uint32_t display_frame_bytes(const struct display_caps *caps)
{
	switch (caps->fmt)
	{
		case DISPLAY_FMT_MONO_VTILED:
			return (uint32_t)caps->x_res * caps->y_res / 8u;
		default:
			return 0u;
	}
}
