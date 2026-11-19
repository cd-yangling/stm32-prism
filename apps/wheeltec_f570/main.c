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
#include <errno.h>
#include "tick.h"
#include "board.h"
#include "pal/led.h"
#include "pal/pin.h"
#include "die.h"

static int brd_devices_init(void)
{
	if (0
		|| !board.led1
		|| !board.led2
		|| !board.buzzer
		)
	{
		return -ENOENT;
	}

	return 0;
}

int main(void)
{
	const struct led_dev *led1 = board.led1;
	const struct led_dev *led2 = board.led2;

	if (brd_devices_init())
		die();

	pin_configure(board.buzzer);
	pin_set(board.buzzer, 1);
	delay_ms(200);
	pin_set(board.buzzer, 0);

	for (;;)
	{
		for (uint32_t i = 0; 500 > i; i++)
		{
			delay_us(1000);
		}

		led_toggle(led1);
		led_toggle(led2);
	}
}
