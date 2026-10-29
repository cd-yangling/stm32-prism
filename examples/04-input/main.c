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
#include "board.h"
#include "pal/led.h"
#include "pal/button.h"
#include "subsys/input.h"
#include "die.h"

static const struct led_dev *cycle[3];
static unsigned int idx;
static unsigned int lit = 1;

static void apply(void)
{
	unsigned int i;

	for (i = 0; i < 3; i++)
	{
		if (lit && i == idx)
		{
			led_on(cycle[i]);
		}
		else
		{
			led_off(cycle[i]);
		}
	}
}

static void on_key(const struct input_event *ev, void *user)
{
	(void)user;

	if (INPUT_KEY_1 == ev->code && 0 != ev->value)
	{
		idx = (idx + 1u) % 3u;
		apply();
	}
	else if (INPUT_KEY_2 == ev->code && 0 == ev->value)
	{
		lit = !lit;
		apply();
	}
}

int main(void)
{
	const struct board *bd = &board;
	static struct input_callback cb;

	if (!bd->led_r || !bd->led_g || !bd->led_b || !bd->key1 || !bd->key2)
		die();

	cycle[0] = bd->led_r;
	cycle[1] = bd->led_g;
	cycle[2] = bd->led_b;


	input_callback_init(&cb, on_key, NULL, NULL);
	input_register_callback(&cb);
	apply();

	for (;;)
	{
		button_poll(bd->key1);
		button_poll(bd->key2);
		__asm volatile("wfi");
	}
}
