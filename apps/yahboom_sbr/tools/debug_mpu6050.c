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
#include "board.h"
#include "tick.h"
#include "app_die.h"
#include "pal/sensor.h"
#include "lib/gadget/algo/comp_filter_10us.h"
#include "lib/gadget/fixtoa.h"
#include "tools/debug_mpu6050.h"

//	用于 gyro 零偏校准测量
void debug_mpu6050_gyro_bias(void)
{
	const int sample_dt_ms = 2;			//	采样间隔时间
	enum { N_SAMP = 2250 };				//	采样次数
	int32_t sg[3] = { 0, 0, 0 };
	struct sensor_raw_value g_raw[3];
	int16_t	gyro_bias[3];

	if (!board.imu1)
	{
		app_die();
	}
  
	for (int i = 0; i < N_SAMP; i++)
	{
		if (0 == sensor_sample_fetch(board.imu1) &&
			0 == sensor_channel_get_raw(board.imu1, SENSOR_CHAN_GYRO_XYZ, g_raw))
		{
			for (int j = 0; j < 3; j++)
			{
				sg[j] += g_raw[j].data.i16;
			}
		}
		else
		{
			app_die();
		}

		delay_ms(sample_dt_ms);
	}

	for (int j = 0; j < 3; j++)
	{
		gyro_bias[j] = (int16_t)(-sg[j] / N_SAMP);
	}

	printf("bias gyro x/y/z: %d %d %d\n",
		gyro_bias[0], gyro_bias[1], gyro_bias[2]);
}

//	用于 观察 x/y/z 方向 填写axis_map配置数据
void debug_mpu6050_dump_raw(void)
{
	const int sample_dt_ms = 2;			//	采样间隔时间

	struct sensor_raw_value g_raw[3];	//	陀螺仪原始数据
	struct sensor_raw_value a_raw[3];	//	加速度计原始数据

	if (!board.imu1)
	{
		app_die();
	}

	for (;;)
	{
		if ((0 == sensor_sample_fetch(board.imu1)) &&
			(0 == sensor_channel_get_raw(board.imu1, SENSOR_CHAN_GYRO_XYZ, g_raw)) &&
			(0 == sensor_channel_get_raw(board.imu1, SENSOR_CHAN_ACCEL_XYZ, a_raw)))
		{
			//	Send To VOFA
			printf("%d,%d,%d,%d,%d,%d\n",
				g_raw[0].data.i16, g_raw[1].data.i16, g_raw[2].data.i16,
				a_raw[0].data.i16, a_raw[1].data.i16, a_raw[2].data.i16);
		}
		else
		{
			app_die();
		}

		delay_ms(sample_dt_ms);
	}
}

//	用于测试单次 I2C-bitbang 消耗的时间
void debug_mpu6050_sample_fetch_time(void)
{
	enum { N_SAMP = 500 };

	uint32_t t0;

	if (!board.imu1)
	{
		app_die();
	}

	t0 = tick_get();

	for (int i = 0; i < N_SAMP; i++)
	{
		if (0 != sensor_sample_fetch(board.imu1))
		{
			app_die();
		}
	}

	printf("sample fetch avg: %u us\n", (unsigned)((tick_get() - t0) * 1000u / N_SAMP));
}

//	用于 VOFA 上观察 pitch 角度解算+互补滤波结果的测试
void debug_mpu6050_pitch_resolve(void)
{
	const int sample_dt_ms = 2;			//	采样间隔时间
	const struct cf_config cfc = CF_CONFIG_500DPS_INIT(sample_dt_ms * 100, 6);
	struct cf_angles angles;

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

	while (1)
	{
		if (sensor_sample_fetch(board.imu1))
			app_die();

		do
		{
			int16_t raw_ax, raw_ay, raw_az;
			int16_t raw_gx, raw_gy, raw_gz;
			{
				struct sensor_raw_value g_raw[3];	//	陀螺仪原始数据
				struct sensor_raw_value a_raw[3];	//	加速度计原始数据

				if (sensor_channel_get_raw(board.imu1, SENSOR_CHAN_GYRO_XYZ, g_raw) ||
					sensor_channel_get_raw(board.imu1, SENSOR_CHAN_ACCEL_XYZ, a_raw))
				{
					app_die();
				}


				raw_ax = a_raw[0].data.i16; (void)raw_ax;
				raw_ay = a_raw[1].data.i16; (void)raw_ay;
				raw_az = a_raw[2].data.i16; (void)raw_az;

				raw_gx = g_raw[0].data.i16; (void)raw_gx;
				raw_gy = g_raw[1].data.i16; (void)raw_gy;
				raw_gz = g_raw[2].data.i16; (void)raw_gz;
			}
			comp_filter_update(
					raw_ax, raw_ay, raw_az,
					raw_gx, raw_gy, raw_gz, CF_AXIS_PITCH, &cfc, &angles);
		} while(0);

		{
			char buf[32];

			fixtoa(buf, sizeof(buf), angles.pitch, 2);

			//	Send To VOFA
			printf("%s\n", buf);
		}

		delay_ms(sample_dt_ms);
	}
}

//void debug_mpu6050_measure_mechanical_center(void)
