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

#include "board.h"
#include "pal/led.h"
#include "subsys/console.h"
#include "subsys/tty.h"
#include "die.h"

//extern void mpu_cf_demo1(void);
extern void mpu_mf_demo2(void);
int main(void)
{
	const struct led_dev *led1 = board.led1;
	struct tty *con = board.console;

	if (led1)
	{
//		led_on(led1);
//		delay_ms(1000);
//		led_off(led1);
	}

	if (con)
	{
		tty_init(con);
		console_attach(con);
	}

//	mpu_cf_demo1();
	mpu_mf_demo2();

	die();

	return 0;
}
