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

#include "tick.h"
#include "board.h"
#include "pal/led.h"
#include "die.h"

int main(void)
{
	const struct led_dev *led_r = board.led_r;
	const struct led_dev *led_g = board.led_g;
	const struct led_dev *led_b = board.led_b;

	if (!led_r || !led_g || !led_b)
		die();


	led_on(led_r);
	delay_ms(1000);
	led_off(led_r);

	led_on(led_g);
	delay_ms(1000);
	led_off(led_g);

	led_on(led_b);
	delay_ms(1000);
	led_off(led_b);

	for (;;) {
		led_toggle(led_r);
		delay_ms(1000);
		led_off(led_r);

		led_toggle(led_g);
		delay_ms(1000);
		led_off(led_g);

		led_toggle(led_b);
		delay_ms(1000);
		led_off(led_b);
	}
}
