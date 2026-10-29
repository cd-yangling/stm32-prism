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
#include "tick.h"
#include "die.h"
#include "board.h"
#include "pal/sensor.h"
#include "pal/display.h"
#include "subsys/gfx.h"
#include "lib/gadget/algo/mahony_filter_1ms.h"
#include "lib/gadget/fixtoa.h"

static volatile uint32_t drdy_flag;
static uint32_t irq_mode;
static const struct sensor_trigger drdy_trig =
{
	.type	= SENSOR_TRIG_DATA_READY,
	.chan	= SENSOR_CHAN_ALL,
};

static void drdy_isr(const struct sensor *s, const struct sensor_trigger *trig)
{
    (void)s;
    (void)trig;
    drdy_flag = 1u;
}

static const struct display * oled_init(void)
{
	const struct display *disp = board.display;

	if (NULL == disp)
		return NULL;

	if (display_init(disp))
		return NULL;

	if (gfx_attach(disp))
		return NULL;

	return disp;
}

static void gyro_calibrate(const struct sensor *imu, int16_t offset[3])
{
#define	NR_CALIBRATE_BIAS	100
	int32_t sum[3] = { 0 };
	struct sensor_raw_value g_raw[3];

	for (int i = 0; i < NR_CALIBRATE_BIAS; i++)
	{
		if (0 == sensor_sample_fetch(imu) &&
			0 == sensor_channel_get_raw(imu, SENSOR_CHAN_GYRO_XYZ, g_raw))
		{
			sum[0] += g_raw[0].data.i16;
			sum[1] += g_raw[1].data.i16;
			sum[2] += g_raw[2].data.i16;
		}

		delay_ms(10);
	}

	for (int i = 0; i < 3; i++)
	{
		offset[i] = -(int16_t)(sum[i] / NR_CALIBRATE_BIAS);
	}
}

void mpu_mf_demo2(void)
{
	int16_t offset[3];
	const struct sensor *imu = board.imu1;
	const struct display *disp = oled_init();

	if (NULL == imu)
		die();

	if (0 != sensor_init(imu))
	{
		die();
	}

	gyro_calibrate(imu, offset);
	{
		struct sensor_raw_value ofs[3];

		for (int i = 0; i < 3; i++)
		{
			ofs[i].type		= SENSOR_RAW_INT16;
			ofs[i].data.i16	= offset[i];
		}
		sensor_attr_set_raw(imu, SENSOR_CHAN_GYRO_XYZ, SENSOR_ATTR_OFFSET, ofs);
	}

	if (disp)
	{
		gfx_fill(0);
		gfx_text(0,  0, "   roll   pitch");
		gfx_text(0, 16, "               ");
		gfx_flush(0, 0, disp->caps->x_res, disp->caps->y_res);
	}

	struct mahony_filter mf;
	struct mf_config cfg = MF_CONFIG_250DPS_INIT(10, MF_CONFIG_KP_DEFAULT, MF_CONFIG_KI_DEFAULT);
	struct mf_angles angles = { 0 };
	struct sensor_raw_value g_raw[3];
	struct sensor_raw_value a_raw[3];

	mahony_filter_init(&mf);

	irq_mode = (0 == sensor_trigger_set(imu, &drdy_trig, drdy_isr));

	for (;;)
	{
		if (irq_mode)
		{
			if (!drdy_flag)
			{
				__asm volatile("wfi");
				continue;
			}
			drdy_flag = 0u;
		}
		else
		{
			delay_ms(cfg.dt_ms);
		}

		if (0 != sensor_sample_fetch(imu) ||
			0 != sensor_channel_get_raw(imu, SENSOR_CHAN_GYRO_XYZ, g_raw) ||
			0 != sensor_channel_get_raw(imu, SENSOR_CHAN_ACCEL_XYZ, a_raw))
		{
			die();
		}

		uint32_t t0 = tick_get_cyc();
		mahony_filter_update(&mf,
			a_raw[0].data.i16, a_raw[1].data.i16, a_raw[2].data.i16,
			g_raw[0].data.i16, g_raw[1].data.i16, g_raw[2].data.i16,
			&cfg);
		uint32_t t1 = tick_get_cyc();

		mahony_filter_get_euler(&mf,
			MF_AXIS_ROLL | MF_AXIS_PITCH | MF_AXIS_YAW, &angles);
		uint32_t t2 = tick_get_cyc();

		(void)t0;
		(void)t1;
		(void)t2;
//		printf("upd %u cyc %u us, eul %u cyc %u us\n",
//				(unsigned)(t1 - t0), (unsigned)((t1 - t0) / tick_cyc_per_us()),
//				(unsigned)(t2 - t1), (unsigned)((t2 - t1) / tick_cyc_per_us()));

		{
			char b1[16];
			char b2[16];
			char b3[16];

			fixtoa(b1, sizeof(b1), angles.roll, 2);
			fixtoa(b2, sizeof(b2), angles.pitch, 2);
			fixtoa(b3, sizeof(b3), angles.yaw, 2);

			printf("%s,%s,%s\n", b1, b2, b3);

			if (disp)
			{
				char li[32];

				snprintf(li, sizeof(li), "%7s %7s", b1, b2);
				gfx_text(0, 16, li);
				gfx_flush(0, 16, disp->caps->x_res, 16);
			}
		}
	}
}
