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

#include <stdio.h>
#include <stdint.h>
#include "board.h"
#include "pal/pin.h"
#include "pal/enc.h"
#include "pal/sensor.h"
#include "pal/display.h"
#include "app_die.h"
#include "tick.h"
#include "lib/gadget/fixtoa.h"
#include "subsys/gfx.h"
#include "tools/debug_misc.h"

//	OLED 测试
void debug_misc_test_oled(void)
{
	if (!board.display)
	{
		app_die();
	}

	gfx_text(0, 0, "Hello World");
	gfx_flush(0, 0, board.display->caps->x_res, 16);
}

//	电池电压测试
void debug_misc_test_battery_volotage(void)
{
	if (!board.vbat_sen)
	{
		app_die();
	}

	for (;;)
	{
		if (0 == sensor_sample_fetch(board.vbat_sen))
		{
			struct sensor_value sv;
			char b[16];

			sensor_channel_get(board.vbat_sen, SENSOR_CHAN_VOLTAGE, &sv);
			fixtoa(b, sizeof(b), (int32_t)sensor_value_to_micro(&sv), 6);
			printf("vbat=%s V\n", b);
		}

		delay_ms(1000);
	}
}

//	蜂鸣器测试
void debug_misc_test_buzzer(void)
{
	if (!board.buzzer)
	{
		app_die();
	}

	for (;;)
	{
		pin_set(board.buzzer, 1);
		delay_ms(200);
		pin_set(board.buzzer, 0);
		delay_ms(800);
	}
}

//	编码器测试(手拨车轮看方向与计数, 500ms 窗口)
void debug_misc_test_encoder(void)
{
	int32_t	prev_l;
	int32_t	prev_r;
	int32_t	acc_l;
	int32_t	acc_r;

	if (!board.enc_l || !board.enc_r)
	{
		app_die();
	}

	if (enc_get_count(board.enc_l->dev, &acc_l)
		|| enc_get_count(board.enc_r->dev, &acc_r))
	{
		app_die();
	}

	prev_l = acc_l;
	prev_r = acc_r;

	for (;;)
	{
		delay_ms(500);

		if (enc_get_count(board.enc_l->dev, &acc_l)
			|| enc_get_count(board.enc_r->dev, &acc_r))
		{
			app_die();
		}

		printf("enc_l=%d dl=%d enc_r=%d dr=%d\n",
				(int)acc_l, (int)(acc_l - prev_l),
				(int)acc_r, (int)(acc_r - prev_r));

		prev_l = acc_l;
		prev_r = acc_r;
	}
}

