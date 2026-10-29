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

#ifndef	__NEXUS_LIB_GADGET_ALGO_COMP_FILTER_10US_HEADER__
#define	__NEXUS_LIB_GADGET_ALGO_COMP_FILTER_10US_HEADER__

#include <stdint.h>
#include "lib/gadget/imu_math.h"

/* ============================================================
 *  陀螺仪积分
 * ============================================================
 *
 * raw: 陀螺仪原始 LSB 值 (int16_t)
 *  dt: 采样周期, 单位 10us (uint16_t)
 * num: 缩放分子
 * shr: 缩放移位数
 *
 * 返回: delta centidegree
 *
 * 溢出判据: |raw * dt * num| + 2^(shr-1) <= INT32_MAX
 *
 * 边界: 除数恰为纯 2 的幂, 四档 num=1,
 *   dt 全 uint16 范围安全 (上限 65535 * 10us = 655.35ms)
 *
 * +2^(shr-1) 对称舍入, 消负侧 floor 单向偏置
 *   (否则 (-1*200)>>16 = -1 而 (+1*200)>>16 = 0)
 */
#define GYRO_INTEGRATE(raw, dt, num, shr) \
	(((int32_t)(raw) * (int32_t)(dt) * (int32_t)(num) + (1 << ((shr) - 1))) >> (shr))

/* ============================================================
 *  陀螺仪积分缩放因子
 *  单位转换: LSB/(度/s) * 10us -> centidegree
 *
 *  delta [centideg] = raw * dt_10us / (sens * 1000)
 *  sens = 32768 / 满量程
 *  除数 sens * 1000 = 2^17..2^14 纯 2 的幂, num=1 移位精确换算
 * ============================================================
 */
/* +/- 250度/s: sens = 32768/250 = 131.072, 除数 2^17 */
#define GYRO_SCALE_NUM_250DPS   (1)
#define GYRO_SCALE_SHR_250DPS   (17)

/* +/- 500度/s: sens = 32768/500 = 65.536, 除数 2^16 */
#define GYRO_SCALE_NUM_500DPS   (1)
#define GYRO_SCALE_SHR_500DPS   (16)

/* +/- 1000度/s: sens = 32768/1000 = 32.768, 除数 2^15 */
#define GYRO_SCALE_NUM_1000DPS  (1)
#define GYRO_SCALE_SHR_1000DPS  (15)

/* +/- 2000度/s: sens = 32768/2000 = 16.384, 除数 2^14 */
#define GYRO_SCALE_NUM_2000DPS  (1)
#define GYRO_SCALE_SHR_2000DPS  (14)

/*
 * ============================================================
 *  滤波器配置
 * ============================================================
 */
struct cf_config
{
    /* 陀螺仪量程对应的缩放系数 */
    uint16_t gyro_scale_num;
    uint8_t  gyro_scale_shr;

    /* 采样周期 单位: 10us */
    uint16_t dt_10us;

    /* 互补滤波系数 alpha:
     *   alpha = 陀螺仪权重, (1-alpha) = 加速度计权重
     *   alpha = (2^alpha_shr - 1) / 2^alpha_shr
     *
     * alpha_shr = 6 -> alpha = 63/64 = 0.984375

	 * alpha_shr = 7 -> alpha = 127/128 = 0.992188

	 * alpha_shr = 5 -> alpha = 31/32 = 0.96875
     *
     */
    uint8_t  alpha_shr;
};

#define CF_CONFIG_250DPS_INIT(_dt_10us, _alpha_shr)	\
{													\
	.gyro_scale_num = GYRO_SCALE_NUM_250DPS,		\
	.gyro_scale_shr = GYRO_SCALE_SHR_250DPS,		\
	.dt_10us        = (_dt_10us),					\
	.alpha_shr      = (_alpha_shr),					\
}

#define CF_CONFIG_500DPS_INIT(_dt_10us, _alpha_shr)	\
{													\
	.gyro_scale_num = GYRO_SCALE_NUM_500DPS,		\
	.gyro_scale_shr = GYRO_SCALE_SHR_500DPS,		\
	.dt_10us        = (_dt_10us),					\
	.alpha_shr      = (_alpha_shr),					\
}

#define CF_CONFIG_1000DPS_INIT(_dt_10us, _alpha_shr)	\
{													\
	.gyro_scale_num = GYRO_SCALE_NUM_1000DPS,		\
	.gyro_scale_shr = GYRO_SCALE_SHR_1000DPS,		\
	.dt_10us        = (_dt_10us),					\
	.alpha_shr      = (_alpha_shr),					\
}

#define CF_CONFIG_2000DPS_INIT(_dt_10us, _alpha_shr)	\
{													\
	.gyro_scale_num = GYRO_SCALE_NUM_2000DPS,		\
	.gyro_scale_shr = GYRO_SCALE_SHR_2000DPS,		\
	.dt_10us        = (_dt_10us),					\
	.alpha_shr      = (_alpha_shr),					\
}

/*
 * ============================================================
 *  滤波器状态 (角度单位: centidegree = 0.01度)
 * ============================================================
 */
struct cf_angles
{
    int32_t roll;    /* 滚转角 单位: centidegree*/
    int32_t pitch;   /* 俯仰角 单位: centidegree*/
};

#define CF_AXIS_PITCH  (1u << 0)
#define CF_AXIS_ROLL   (1u << 1)

static inline __attribute__((always_inline)) void comp_filter_update(
	int16_t raw_ax, int16_t raw_ay, int16_t raw_az,
	int16_t raw_gx, int16_t raw_gy, int16_t raw_gz,
	uint8_t axes,
	const struct cf_config * cfg, struct cf_angles * angles)
{
	(void)raw_gz;

	if (axes & CF_AXIS_PITCH)
	{
		int32_t a_pitch;
		int32_t g_pitch;

		/* ---------- Step 1: 加速度计角度 ---------- */

		/*
		 * pitch(俯仰角): sqrt(ay^2 + az^2) 作为 atan2 的 x 分量
		 */
		{
			uint32_t hor_sq  = (uint32_t)((int32_t)raw_ay * raw_ay)
			                 + (uint32_t)((int32_t)raw_az * raw_az);

			uint32_t hor_mag = isqrt32(hor_sq);

			/* atan2(ax, sqrt(ay^2+az^2)): 向前抬头为正 pitch */
			a_pitch = cordic_atan2((int32_t)raw_ax, (int32_t)hor_mag);
		}

		/* ---------- Step 2: 陀螺仪积分 ----------
		 * gy: pitch 角速度
		 */
		{
			int32_t delta_pitch = GYRO_INTEGRATE(raw_gy,
					cfg->dt_10us, cfg->gyro_scale_num, cfg->gyro_scale_shr);

			g_pitch = angles->pitch + delta_pitch;
		}

		/* ---------- Step 3: 互补合并 ---------- */
		{
			angles->pitch = g_pitch + ((a_pitch - g_pitch) >> cfg->alpha_shr);
		}
	}

	if (axes & CF_AXIS_ROLL)
	{
		int32_t a_roll;
		int32_t g_roll;

		{
			uint32_t ver_sq  = (uint32_t)((int32_t)raw_ax * raw_ax)
			                 + (uint32_t)((int32_t)raw_az * raw_az);

			uint32_t ver_mag = isqrt32(ver_sq);

			a_roll = cordic_atan2((int32_t)-raw_ay, (int32_t)ver_mag);
		}

		{
			int32_t delta_roll = GYRO_INTEGRATE(raw_gx,
					cfg->dt_10us, cfg->gyro_scale_num, cfg->gyro_scale_shr);

			g_roll = angles->roll + delta_roll;
		}

		{
			angles->roll = g_roll + ((a_roll - g_roll) >> cfg->alpha_shr);
		}
	}
}

#endif	//	__NEXUS_LIB_GADGET_ALGO_COMP_FILTER_10US_HEADER__
