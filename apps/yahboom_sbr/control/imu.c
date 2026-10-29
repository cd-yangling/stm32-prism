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
#include "tick.h"
#include "board.h"
#include "app_die.h"
#include "pal/sensor.h"
#include "control/imu.h"

#if defined(CONFIG_MPU6050_INT)
static volatile uint32_t drdy_flag;
static volatile uint32_t drdy_ts_cyc;
static uint32_t drdy_prev_cyc;
static uint8_t drdy_have_prev;
static const struct sensor_trigger drdy_trig =
{
	.type   = SENSOR_TRIG_DATA_READY,
	.chan   = SENSOR_CHAN_ALL,
};

static void drdy_isr(const struct sensor *s, const struct sensor_trigger *trig)
{
	(void)s; (void)trig;
	drdy_ts_cyc = tick_get_cyc();
	drdy_flag = 1u;
}
#else
static uint32_t sample_deadline_cyc;
//	注意: 这里的时间务必保持和 board.c 配置的 mpu6050 参数一致
#define SAMPLE_PERIOD_MS 2u
#endif

static void imu_bias_set(void)
{
	//	设置 零偏 参数
	struct sensor_raw_value a_bias[3];
	struct sensor_raw_value g_bias[3];

	//	加速度计(零偏参数)
	for (int i = 0; i < 3; i++)
	{
		a_bias[i].type	= SENSOR_RAW_INT16;
	}

	a_bias[0].data.i16	= 0;
	a_bias[1].data.i16	= 0;
	a_bias[2].data.i16	= 0;
	sensor_attr_set_raw(board.imu1, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_OFFSET, a_bias);

	//	陀螺仪(零偏参数)
	for (int i = 0; i < 3; i++)
	{
		g_bias[i].type	= SENSOR_RAW_INT16;
	}

	g_bias[0].data.i16	= -63;
	g_bias[1].data.i16	= 2;
	g_bias[2].data.i16	= 142;
	//	倒立姿态零偏
	//	g_bias[0].data.i16	= -65;
	//	g_bias[1].data.i16	= 0;
	//	g_bias[2].data.i16	= 139;
	sensor_attr_set_raw(board.imu1, SENSOR_CHAN_GYRO_XYZ, SENSOR_ATTR_OFFSET, g_bias);
}

void imu_init(void)
{
	imu_bias_set();

#if defined(CONFIG_MPU6050_INT)
	sensor_trigger_set(board.imu1, &drdy_trig, drdy_isr);
#else
	sample_deadline_cyc = tick_get_cyc() + SAMPLE_PERIOD_MS * 1000u * tick_cyc_per_us();
#endif
}

void imu_fetch(struct imu_raw_sample * raw)
{
	struct sensor_raw_value g_raw[3];
	struct sensor_raw_value a_raw[3];

	if (sensor_sample_fetch(board.imu1) ||
		sensor_channel_get_raw(board.imu1, SENSOR_CHAN_GYRO_XYZ, g_raw) ||
		sensor_channel_get_raw(board.imu1, SENSOR_CHAN_ACCEL_XYZ, a_raw))
	{
		app_die();
	}

	raw->ax = a_raw[0].data.i16;
	raw->ay = a_raw[1].data.i16;
	raw->az = a_raw[2].data.i16;
	raw->gx = g_raw[0].data.i16;
	raw->gy = g_raw[1].data.i16;
	raw->gz = g_raw[2].data.i16;
}

//	采样触发判断: 并返回 dt 单位: 10us
int imu_sample_due(uint16_t *dt_10us)
{
#if defined(CONFIG_MPU6050_INT)
	if (0 == drdy_flag)
		return 0;

	{
		uint32_t ts = drdy_ts_cyc;

		drdy_flag = 0u;

		if (0u == drdy_have_prev)
		{
			drdy_prev_cyc = ts;
			drdy_have_prev = 1u;
			return 0;
		}

		{
			uint32_t diff_cyc = ts - drdy_prev_cyc;
			uint32_t diff_us = diff_cyc / tick_cyc_per_us();

			drdy_prev_cyc = ts;

			if (0u == diff_cyc || 655350u < diff_us)
			{
				app_die();
			}

			*dt_10us = (uint16_t)(diff_us / 10u);
			return 1;
		}
	}
#else
	if (!tick_expired_cyc(sample_deadline_cyc))
		return 0;

	sample_deadline_cyc = tick_get_cyc() + SAMPLE_PERIOD_MS * 1000u * tick_cyc_per_us();
	*dt_10us = SAMPLE_PERIOD_MS * 100;
	return 1;
#endif

}

