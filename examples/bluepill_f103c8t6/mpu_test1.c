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
#include "lib/gadget/algo/comp_filter_1ms.h"
#include "lib/gadget/fixtoa.h"

static volatile uint32_t drdy_flag;
static uint32_t irq_mode;
static const struct sensor_trigger drdy_trig =
{
    .type   = SENSOR_TRIG_DATA_READY,
    .chan   = SENSOR_CHAN_ALL,
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

static void calibrate_offset(const struct sensor *imu, int16_t offset[6])
{
#define	NR_CALIBRATE_BIAS	100
	int32_t sum_ax = 0, sum_ay = 0, sum_az = 0;
	int32_t sum_gx = 0, sum_gy = 0, sum_gz = 0;
	struct sensor_raw_value g_raw[3];	//	陀螺仪原始数据
	struct sensor_raw_value a_raw[3];	//	加速度计原始数据

	for (int i = 0; i < NR_CALIBRATE_BIAS; i++)
	{
		if (0 == sensor_sample_fetch(imu) &&
			0 == sensor_channel_get_raw(imu, SENSOR_CHAN_GYRO_XYZ, g_raw) &&
			0 == sensor_channel_get_raw(imu, SENSOR_CHAN_ACCEL_XYZ, a_raw))
		{
			sum_gx += g_raw[0].data.i16; sum_gy += g_raw[1].data.i16; sum_gz += g_raw[2].data.i16;
			sum_ax += a_raw[0].data.i16; sum_ay += a_raw[1].data.i16; sum_az += a_raw[2].data.i16;
		}
		delay_ms(10);
	}

	(void)sum_ax;
	(void)sum_ay;
	(void)sum_az;

	(void)sum_gx;
	(void)sum_gy;
	(void)sum_gz;

	/* offset = -静止均值(az 目标重力 16384, +/- 2g 量程 1g) */
//	offset[0] = -(int16_t)(sum_ax / NR_CALIBRATE_BIAS);
//	offset[1] = -(int16_t)(sum_ay / NR_CALIBRATE_BIAS);
//	offset[2] = (int16_t)(16384 - sum_az / NR_CALIBRATE_BIAS);
	offset[3] = -(int16_t)(sum_gx / NR_CALIBRATE_BIAS);
	offset[4] = -(int16_t)(sum_gy / NR_CALIBRATE_BIAS);
	offset[5] = -(int16_t)(sum_gz / NR_CALIBRATE_BIAS);
}

void mpu_cf_demo1(void)
{
	int16_t offset[6] = { 0 };
	const struct sensor *imu = board.imu1;
	const struct display *disp = oled_init();

	if (NULL == imu)
		die();

	if (0 != sensor_init(imu))
	{
		die();
	}

	calibrate_offset(imu, offset);
	{
		struct sensor_raw_value ofs[3];

		for (int i = 0; i < 3; i++)
		{
			ofs[i].type		= SENSOR_RAW_INT16;
			ofs[i].data.i16	= offset[i];
		}
		sensor_attr_set_raw(imu, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_OFFSET, ofs);

		for (int i = 0; i < 3; i++)
		{
			ofs[i].data.i16	= offset[3 + i];
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

	struct cf_angles angles = { 0 };
	struct sensor_raw_value g_raw[3];	//	陀螺仪原始数据
	struct sensor_raw_value a_raw[3];	//	加速度计原始数据

	irq_mode = (0 == sensor_trigger_set(imu, &drdy_trig, drdy_isr));

	for(;;)
	{
		int16_t raw_gx, raw_gy, raw_gz;
		int16_t raw_ax, raw_ay, raw_az;

		struct cf_config cfg = CF_CONFIG_250DPS_INIT(10, 6);

		if (0 != irq_mode)
		{
			if (0 == drdy_flag)
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

		raw_gx = g_raw[0].data.i16; (void)raw_gx;
		raw_gy = g_raw[1].data.i16; (void)raw_gy;
		raw_gz = g_raw[2].data.i16; (void)raw_gz;

		raw_ax = a_raw[0].data.i16; (void)raw_ax;
		raw_ay = a_raw[1].data.i16; (void)raw_ay;
		raw_az = a_raw[2].data.i16; (void)raw_az;

		comp_filter_update(
			raw_ax, raw_ay, raw_az,
			raw_gx, raw_gy, raw_gz,
			CF_AXIS_PITCH | CF_AXIS_ROLL,
			&cfg, &angles);

		if (1)	//	send to VOFA
		{
			char b1[16];
			char b2[16];

			fixtoa(b1, sizeof(b1), angles.roll, 2);
			fixtoa(b2, sizeof(b2), angles.pitch, 2);

			printf("%s,%s\n", b1, b2);
		}

		if (disp)
		{
			char b1[16];
			char b2[16];
			char li[32];

			fixtoa(b1, sizeof(b1), angles.roll, 2);
			fixtoa(b2, sizeof(b2), angles.pitch, 2);

			snprintf(li, sizeof(li), "%7s %7s", b1, b2);
			gfx_text(0, 16, li);
			gfx_flush(0, 16, disp->caps->x_res, 16);
		}
	}
}
