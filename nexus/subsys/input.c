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
#include "subsys/input.h"

static struct input_callback *cbs;

void input_callback_init(struct input_callback *cb, input_cb_t handler,
	const void *dev, void *user)
{
	cb->handler	= handler;
	cb->dev		= dev;
	cb->user	= user;
	cb->next	= NULL;
}

int input_register_callback(struct input_callback *cb)
{
	struct input_callback **p;

	for (p = &cbs; *p; p = &(*p)->next)
	{
		if (*p == cb)
		{
			return -EINVAL;
		}
	}

	*p = cb;
	cb->next = NULL;

	return 0;
}

int input_remove_callback(struct input_callback *cb)
{
	struct input_callback **p;

	for (p = &cbs; *p; p = &(*p)->next)
	{
		if (*p == cb)
		{
			*p = cb->next;
			cb->next = NULL;
			return 0;
		}
	}

	return -ENOENT;
}

void input_report(const void *dev, uint8_t type, uint16_t code,
	int32_t value, uint8_t sync)
{
	struct input_event ev =
	{
		.dev	= dev,
		.type	= type,
		.code	= code,
		.value	= value,
		.sync	= sync,
	};
	struct input_callback *p;

	for (p = cbs; p; p = p->next)
	{
		if (!p->dev || p->dev == dev)
		{
			p->handler(&ev, p->user);
		}
	}
}
