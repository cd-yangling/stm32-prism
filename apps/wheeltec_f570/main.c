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
#include <stdio.h>
#include <errno.h>
#include "tick.h"
#include "board.h"
#include "subsys/tty.h"
#include "subsys/console.h"
#include "pal/led.h"
#include "pal/pin.h"
#include "pal/sensor.h"
#include "die.h"

static int brd_devices_init(void)
{
	int ret;

	if (0
		|| !board.led1
		|| !board.led2
		|| !board.buzzer
		|| !board.vbat_sen
		|| !board.imu
		|| !board.console
		)
	{
		return -ENOENT;
	}

	ret = tty_init(board.console);
	if (0 != ret)
		return ret;

	ret = console_attach(board.console);
	if (0 != ret)
		return ret;

	ret = sensor_init(board.vbat_sen);
	if (0 != ret)
		return ret;

	ret = sensor_init(board.imu);
	if (0 != ret)
		return ret;

	return 0;
}

int main(void)
{
	const struct led_dev *led1 = board.led1;
	const struct led_dev *led2 = board.led2;
	struct sensor_value sv;
	struct sensor_raw_value srv[3];

	if (brd_devices_init())
		die();

	pin_configure(board.buzzer);
	pin_set(board.buzzer, 1);
	delay_ms(200);
	pin_set(board.buzzer, 0);

	printf("wheeltec f570 console up\r\n");

	for (uint8_t i = 0u; 10u > i; i++)
	{
		sensor_sample_fetch(board.vbat_sen);
		sensor_channel_get(board.vbat_sen, SENSOR_CHAN_VOLTAGE, &sv);
		printf("vbat=%dmV\r\n", sv.val1 * 1000 + sv.val2 / 1000);
		delay_ms(500);
	}

	for (;;)
	{
		int c = tty_getc(board.console);

		if (0 > c)
		{
			continue;
		}

		char b = (char)c;

		tty_write(board.console, &b, 1u);

		if ('y' == c)
		{
			break;
		}
	}

	for (;;)
	{
		for (uint32_t i = 0; 500 > i; i++)
		{
			delay_us(1000);
		}

		led_toggle(led1);
		led_toggle(led2);

		if (0 == sensor_sample_fetch(board.imu))
		{
			if (0 == sensor_channel_get_raw(board.imu, SENSOR_CHAN_ACCEL_XYZ, srv))
			{
				printf("a %d %d %d\r\n",
					srv[0].data.i16, srv[1].data.i16, srv[2].data.i16);
			}

			if (0 == sensor_channel_get_raw(board.imu, SENSOR_CHAN_GYRO_XYZ, srv))
			{
				printf("g %d %d %d\r\n",
					srv[0].data.i16, srv[1].data.i16, srv[2].data.i16);
			}

			if (0 == sensor_channel_get_raw(board.imu, SENSOR_CHAN_DIE_TEMP, srv))
			{
				printf("t %d\r\n", srv[0].data.i16);
			}
		}
	}
}
