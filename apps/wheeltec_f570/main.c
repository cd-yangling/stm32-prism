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
#include "pal/uart.h"
#include "dev/sensor/lc307.h"
#include "dev/sensor/stp23l.h"
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

#if 0
int main(void)
{
	const struct led_dev *led1 = board.led1;
	const struct led_dev *led2 = board.led2;
	struct sensor_value sv[3];
	struct sensor_raw_value srv[3];

	if (brd_devices_init())
		die();

	pin_configure(board.buzzer);
	pin_set(board.buzzer, 1);
	delay_ms(200);
	pin_set(board.buzzer, 0);

	printf("wheeltec f570 console up\r\n");

	srv[0].type = SENSOR_RAW_INT16;
	srv[0].data.i16 = 105;
	srv[1].type = SENSOR_RAW_INT16;
	srv[1].data.i16 = 95;
	srv[2].type = SENSOR_RAW_INT16;
	srv[2].data.i16 = -290;
	sensor_attr_set_raw(board.imu, SENSOR_CHAN_MAGN_XYZ, SENSOR_ATTR_OFFSET, srv);

	for (uint8_t i = 0u; 10u > i; i++)
	{
		sensor_sample_fetch(board.vbat_sen);
		sensor_channel_get(board.vbat_sen, SENSOR_CHAN_VOLTAGE, &sv[0]);
		printf("vbat=%dmV\r\n", sv[0].val1 * 1000 + sv[0].val2 / 1000);
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

			if (0 == sensor_channel_get(board.imu, SENSOR_CHAN_ACCEL_XYZ, sv))
			{
				printf("asi %d %d %d\r\n",
					sv[0].val1 * 1000 + sv[0].val2 / 1000,
					sv[1].val1 * 1000 + sv[1].val2 / 1000,
					sv[2].val1 * 1000 + sv[2].val2 / 1000);
			}

			if (0 == sensor_channel_get_raw(board.imu, SENSOR_CHAN_GYRO_XYZ, srv))
			{
				printf("g %d %d %d\r\n",
					srv[0].data.i16, srv[1].data.i16, srv[2].data.i16);
			}

			if (0 == sensor_channel_get(board.imu, SENSOR_CHAN_GYRO_XYZ, sv))
			{
				printf("gsi %d %d %d\r\n",
					sv[0].val1 * 1000 + sv[0].val2 / 1000,
					sv[1].val1 * 1000 + sv[1].val2 / 1000,
					sv[2].val1 * 1000 + sv[2].val2 / 1000);
			}

			if (0 == sensor_channel_get_raw(board.imu, SENSOR_CHAN_DIE_TEMP, srv))
			{
				printf("t %d\r\n", srv[0].data.i16);
			}

			if (0 == sensor_channel_get_raw(board.imu, SENSOR_CHAN_MAGN_XYZ, srv))
			{
				printf("m %d %d %d\r\n",
					srv[0].data.i16, srv[1].data.i16, srv[2].data.i16);
			}

			if (0 == sensor_channel_get(board.imu, SENSOR_CHAN_MAGN_XYZ, sv))
			{
				printf("msi %d %d %d\r\n",
					sv[0].val1 * 1000 + sv[0].val2 / 1000,
					sv[1].val1 * 1000 + sv[1].val2 / 1000,
					sv[2].val1 * 1000 + sv[2].val2 / 1000);
			}
		}
	}
}
#endif

static unsigned int oflow_cnt;
static unsigned int laser_cnt;
static unsigned int stat_deadline;

int main(void)
{
	const struct lc307_rec *orec;
	const struct stp23l_rec *lrec;

	if (brd_devices_init())
		die();

	pin_configure(board.buzzer);
	pin_set(board.buzzer, 1);
	delay_ms(200);
	pin_set(board.buzzer, 0);

	printf("wheeltec f570 stream sensors up\r\n");

	if (NULL != board.oflow)
	{
		printf("lc307 init %d\r\n", lc307_init(board.oflow));
	}

	if (NULL != board.laser)
	{
		printf("laser init %d\r\n", stp23l_init(board.laser));
	}

	stat_deadline = tick_get() + 1000u;

	for (;;)
	{
		delay_us(1000);

		if (NULL != board.oflow)
		{
			lc307_feed(board.oflow);

			while (NULL != (orec = lc307_peek(board.oflow)))
			{
				printf("oflow fx=%d fy=%d dt=%u v=%x ver=%u v1m=%d/%d cyc=%u\r\n",
						orec->flowx, orec->flowy,
						(unsigned int)orec->dt_us,
						orec->valid, orec->ver,
						(int)(lc307_flow_rad(orec->flowx, orec->dt_us) * 1000.0f),
						(int)(lc307_flow_rad(orec->flowy, orec->dt_us) * 1000.0f),
						orec->ts);
				lc307_advance(board.oflow);
				oflow_cnt++;
			}
		}

		if (NULL != board.laser)
		{
			stp23l_feed(board.laser);

			while (NULL != (lrec = stp23l_peek(board.laser)))
			{
				short mn = 32767;
				short mx = -32768;
				long sum = 0;
				unsigned int n;

				for (n = 0u; 12u > n; n++)
				{
					short dv = lrec->dist_mm[n];

					sum += dv;

					if (dv < mn)
					{
						mn = dv;
					}

					if (dv > mx)
					{
						mx = dv;
					}
				}

				printf("laser d=%d spr=%d c=%u cyc=%u\r\n",
						(int)(sum / 12), (int)(mx - mn),
						lrec->quality[0], lrec->ts);
				stp23l_advance(board.laser);
				laser_cnt++;
			}
		}

		if (tick_expired(stat_deadline))
		{
			stat_deadline = tick_get() + 1000u;
#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
			printf("stat oflow=%u/s laser=%u/s drop=%u/%u fd=%u/%u\r\n",
					oflow_cnt, laser_cnt,
					(NULL != board.oflow) ? lc307_rx_drop_get(board.oflow) : 0u,
					(NULL != board.laser) ? stp23l_rx_drop_get(board.laser) : 0u,
					(NULL != board.oflow) ? lc307_frame_drop_get(board.oflow) : 0u,
					(NULL != board.laser) ? stp23l_frame_drop_get(board.laser) : 0u);
#else
			printf("stat oflow=%u/s laser=%u/s fd=%u/%u\r\n",
					oflow_cnt, laser_cnt,
					(NULL != board.oflow) ? lc307_frame_drop_get(board.oflow) : 0u,
					(NULL != board.laser) ? stp23l_frame_drop_get(board.laser) : 0u);
#endif
			oflow_cnt = 0u;
			laser_cnt = 0u;
		}
	}
}
