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
#include "pal/pin.h"
#include "pal/button.h"
#include "subsys/input.h"
#include "tick.h"
#include "start_gate.h"

static int key_hit;
static struct input_callback cb;

static void on_key(const struct input_event *ev, void *user)
{
	(void)user;

	if (INPUT_KEY_1 == ev->code && 0 == ev->value)
	{
		key_hit = 1;
	}
}

void start_gate_wait(void)
{
	input_callback_init(&cb, on_key, NULL, NULL);
	input_register_callback(&cb);

	while (0 == pin_get(board.key1->pin))
	{
		__asm volatile ("wfi");
	}

	pin_set(board.buzzer, 1);
	delay_ms(200u);
	pin_set(board.buzzer, 0);

	while (!key_hit)
	{
		button_poll(board.key1);
		__asm volatile ("wfi");
	}
}
