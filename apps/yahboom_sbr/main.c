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
#include <stdio.h>
#include "tick.h"
#include "board.h"
#include "app_die.h"
#include "start_gate.h"
#include "pal/button.h"
#include "pal/pin.h"
#include "pal/display.h"
#include "pal/sensor.h"
#include "pal/enc.h"
#include "subsys/console.h"
#include "subsys/tty.h"
#include "subsys/gfx.h"
#include "motor.h"
#include "tuning.h"
#include "tools/debug_entry.h"
#include "control/control.h"
#include "tasks/battery.h"
#include "tasks/sonar.h"

//	使用 BLE 蓝牙调参 or 有线串口调参数
//	0:	BLE 蓝牙调参
//	1:	有线串口调参
#define	TUNE_ON_CONSOLE	0

#define	POLL_MAX_STAT	1

static int brd_devices_init(void)
{
	int ret;

	if (0
		|| !board.console
//		|| !board.display
		|| !board.imu1
		|| !board.vbat_sen
		|| !board.at8236_l
		|| !board.at8236_r
		|| !board.motor_l
		|| !board.motor_r
		|| !board.buzzer
		|| !board.key1
		|| !board.bt
		|| !board.enc_l
		|| !board.enc_r
		)
	{
		return -ENOENT;
	}

	ret = tty_init(board.console);
	if (ret)
		return ret;

	ret = console_attach(board.console);
	if (ret)
		return ret;

#if 0
	ret = display_init(board.display);
	printf("display_init: %d\n", ret);
	if (ret)
		return ret;

	ret = gfx_attach(board.display);
	printf("gfx_attach: %d\n", ret);
	if (ret)
		return ret;
#endif

	ret = sensor_init(board.imu1);
	printf("imu_init: %d\n", ret);
	if (ret)
		return ret;

	ret = sensor_init(board.vbat_sen);
	printf("vbat_init: %d\n", ret);
	if (ret)
		return ret;

	ret = motor_init(board.motor_l);
	printf("motor_l_init: %d\n", ret);
	if (ret)
	{
		return ret;
	}

	ret = motor_init(board.motor_r);
	printf("motor_r_init: %d\n", ret);
	if (ret)
	{
		return ret;
	}

	ret = enc_init(board.enc_l);
	printf("enc_l_init: %d\n", ret);
	if (ret)
	{
		return ret;
	}

	ret = enc_init(board.enc_r);
	printf("enc_l_init: %d\n", ret);
	if (ret)
	{
		return ret;
	}

	pin_configure(board.buzzer);

#if defined(CONFIG_SONAR)
	pin_configure(board.sonar_trig);
	pin_configure(board.sonar_echo);
#endif

	button_init(board.key1);

#if TUNE_ON_CONSOLE
	tuning_init(board.console);
#else
	ret = tty_init(board.bt);
	if (ret)
		return ret;

	tuning_init(board.bt);
#endif

	return ret;
}

#if POLL_MAX_STAT
static volatile uint32_t poll_max_cyc;
#endif

int main(void)
{
	if (brd_devices_init())
		app_die();

	battery_init();

#if defined(CONFIG_SONAR)
	sonar_init();
#endif

	debug_entry();

	start_gate_wait();

	control_init();

	for(;;)
	{
#if POLL_MAX_STAT
		uint32_t poll_t0_cyc = tick_get_cyc();
#endif
		tuning_poll();
		battery_poll();
		control_poll();
#if defined(CONFIG_SONAR)
		sonar_poll();
#endif
#if POLL_MAX_STAT
		{
			uint32_t poll_diff_cyc = tick_get_cyc() - poll_t0_cyc;

			if (poll_diff_cyc > poll_max_cyc)
			{
				poll_max_cyc = poll_diff_cyc;
			}

			if (2000u * tick_cyc_per_us() < poll_diff_cyc)
			{
				app_die();
			}
		}
#endif
	}
}
