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

#include <stddef.h>
#include <errno.h>
#include "pal/sensor.h"

int sensor_init(const struct sensor *s)
{
	return s->ops->init(s);
}

int sensor_sample_fetch(const struct sensor *s)
{
	return s->ops->sample_fetch(s);
}

int sensor_channel_get(const struct sensor *s, enum sensor_chan sc, struct sensor_value *sv)
{
	return s->ops->channel_get(s, sc, sv);
}

int sensor_channel_get_raw(const struct sensor *s, enum sensor_chan sc, struct sensor_raw_value *srv)
{
	return s->ops->channel_get_raw(s, sc, srv);
}

int sensor_trigger_set(const struct sensor *s, const struct sensor_trigger *trig,
	sensor_trigger_handler_t handler)
{
	if (NULL == s->ops->trigger_set)
	{
		return -ENOSYS;
	}

	return s->ops->trigger_set(s, trig, handler);
}

int sensor_attr_set_raw(const struct sensor *s, enum sensor_chan sc,
	enum sensor_attr attr, const struct sensor_raw_value *srv)
{
	if (NULL == s->ops->attr_set_raw)
	{
		return -ENOSYS;
	}

	return s->ops->attr_set_raw(s, sc, attr, srv);
}

int sensor_submit(const struct sensor *s)
{
	if (NULL == s->ops->submit)
	{
		return -ENOSYS;
	}

	return s->ops->submit(s);
}

int sensor_poll(const struct sensor *s)
{
	if (NULL == s->ops->poll)
	{
		return -ENOSYS;
	}

	return s->ops->poll(s);
}

