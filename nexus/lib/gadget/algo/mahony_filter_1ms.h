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

#ifndef __NEXUS_LIB_GADGET_ALGO_MAHONY_FILTER_1MS_HEADER__
#define __NEXUS_LIB_GADGET_ALGO_MAHONY_FILTER_1MS_HEADER__
/**
 *	纯整数 Mahony 四元数姿态滤波器 (无 float, 无 FPU)
 *
 *	算法原理:
 *		1. 使用定点数 (Q30 格式: 1.0 = 2^30 = 1073741824) 表示四元数 q0, q1, q2, q3
 *		2. 提取当前姿态下估计的重力向量 v = [vx, vy, vz]
 *		3. 将重力加速度计实测值 a 与 估计重力 v 作叉积, 得到误差向量 e = cross(a, v)
 *		4. 通过 PI 控制器反馈修正陀螺仪角速度: gyro_corr = gyro + Kp * e + Ki * integral_e
 *		5. 四元数微分方程更新: dq/dt = 0.5 * quat_mul(q, quat(0, gyro_corr))
 *		6. 四元数单位化: 使用 isqrt32 做整数平方根近似，将定点 q 缩放到单位模长
 */


#include <stdint.h>
#include "lib/gadget/imu_math.h"

#define	MAHONY_Q30_ONE		(1 << 30)
#define	MAHONY_Q14_ONE		(1 << 14)
#define	MAHONY_Q14_MUL_ONE	(1 << 28)

/* ============================================================
 *  陀螺仪积分
 * ============================================================
 *
 * raw: 陀螺仪角速度 LSB (含 PI 修正项, 超 int16)
 *  dt: 采样周期 ms (uint16_t)
 * num: 缩放分子
 * shr: 缩放移位数
 *
 * 返回: half angle in Q19 radian
 *   (0.5 * omega * dt, 四元数微分方程 0.5 系数
 *   与 角度->弧度 pi/18000 转换折入缩放表)
 *
 * 中间量 int64: 常规参数域无溢出(注意: kp/dt/num 不同的组合仍可能超过 2^63)
 *
 * +2^(shr-1) 对称舍入, 消负侧 floor 单向偏置
 *   (否则 (-1*10*572)>>19 = -1 而 (+1*10*572)>>19 = 0)
 */
#define GYRO_INTEGRATE(raw, dt, num, shr) \
	(((shr) >= 1 && (shr) <= 63) \
		? (int32_t)(((int64_t)(raw) * (int64_t)(dt) * (int64_t)(num) + \
			((int64_t)1 << ((shr) - 1))) >> (shr)) \
		: 0)

/* ============================================================
 *  陀螺仪积分缩放因子
 *  单位转换: LSB/(度/s) * ms -> Q19 radian (含 0.5 系数)
 *
 *  half_q19 = raw * dt_ms * 0.5 * (PI/180) / (1000 * sens) * 2^19
 *  sens = 32768 / 满量程
 *  除数非 2 的幂, 配平 num * div = 2^19
 * ============================================================
 */
/* +/- 250度/s: sens = 32768/250 = 131.072, 除数 28.64 */
#define GYRO_SCALE_NUM_250DPS   (18301)
#define GYRO_SCALE_SHR_250DPS   (19)

/* +/- 500度/s: sens = 32768/500 = 65.536, 除数 14.32 */
#define GYRO_SCALE_NUM_500DPS   (36602)
#define GYRO_SCALE_SHR_500DPS   (19)

/* +/- 1000度/s: sens = 32768/1000 = 32.768, 除数 7.16 */
#define GYRO_SCALE_NUM_1000DPS  (36602)
#define GYRO_SCALE_SHR_1000DPS  (18)

/* +/- 2000度/s: sens = 32768/2000 = 16.384, 除数 3.58 */
#define GYRO_SCALE_NUM_2000DPS  (36602)
#define GYRO_SCALE_SHR_2000DPS  (17)

/*
 * ============================================================
 *  滤波器配置
 * ============================================================
 */
struct mf_config
{
	/* 陀螺仪量程对应的缩放系数 */
	uint16_t gyro_scale_num;
	uint8_t  gyro_scale_shr;

	/* 采样周期 单位: ms*/
	uint16_t dt_ms;

	/**
	 *	PI 比例与积分参数 (放大1000倍存储)
	 *	Kp = 2.0 -> kp_x1000 = 2000
	 *	Ki = 0.005 -> ki_x1000 = 5
	 */
	int32_t kp_x1000;
	int32_t ki_x1000;
};

#define	MF_CONFIG_KP_DEFAULT	2000u
#define	MF_CONFIG_KI_DEFAULT	5u

/* 积分器抗饱和 (LSB, 两倍陀螺满量程) */
#define	MF_E_INT_MAX			65535

/* Q19 radian, 0.5 rad: 单次转角上限 1 rad */
#define	MF_HALF_DT_MAX			(1 << 18)

#define	MF_Q_GUARD				((1 << 30) + (1 << 20))

#define MF_CONFIG_250DPS_INIT(_dt_ms, _kp, _ki)		\
{													\
	.gyro_scale_num = GYRO_SCALE_NUM_250DPS,		\
	.gyro_scale_shr = GYRO_SCALE_SHR_250DPS,		\
	.dt_ms          = (_dt_ms),						\
	.kp_x1000       = (_kp),						\
	.ki_x1000       = (_ki),						\
}

#define MF_CONFIG_500DPS_INIT(_dt_ms, _kp, _ki)		\
{													\
	.gyro_scale_num = GYRO_SCALE_NUM_500DPS,		\
	.gyro_scale_shr = GYRO_SCALE_SHR_500DPS,		\
	.dt_ms          = (_dt_ms),						\
	.kp_x1000       = (_kp),						\
	.ki_x1000       = (_ki),						\
}

#define MF_CONFIG_1000DPS_INIT(_dt_ms, _kp, _ki)	\
{													\
	.gyro_scale_num = GYRO_SCALE_NUM_1000DPS,		\
	.gyro_scale_shr = GYRO_SCALE_SHR_1000DPS,		\
	.dt_ms          = (_dt_ms),						\
	.kp_x1000       = (_kp),						\
	.ki_x1000       = (_ki),						\
}

#define MF_CONFIG_2000DPS_INIT(_dt_ms, _kp, _ki)	\
{													\
	.gyro_scale_num = GYRO_SCALE_NUM_2000DPS,		\
	.gyro_scale_shr = GYRO_SCALE_SHR_2000DPS,		\
	.dt_ms          = (_dt_ms),						\
	.kp_x1000       = (_kp),						\
	.ki_x1000       = (_ki),						\
}

/*
 * ============================================================
 *  滤波器角度 (角度单位: centidegree = 0.01度)
 * ============================================================
 */
struct mf_angles
{
    int32_t roll;    /* 滚转角 单位: centidegree*/
    int32_t pitch;   /* 俯仰角 单位: centidegree*/
    int32_t yaw;     /* 偏航角 单位: centidegree*/
};

#define MF_AXIS_PITCH  (1u << 0)
#define MF_AXIS_ROLL   (1u << 1)
#define MF_AXIS_YAW    (1u << 2)

/**
 * ============================================================
 * Mahony 滤波器结构体 (纯整数定点数)
 * ============================================================
 */
struct mahony_filter
{
	/* 四元数 q0, q1, q2, q3 (Q30 定点数, 初始化为 [1.0, 0, 0, 0]) */
	int32_t q0;
	int32_t q1;
	int32_t q2;
	int32_t q3;

	/* 积分误差累加器 (Q16 定点数) */
	int32_t e_int_x;
	int32_t e_int_y;
	int32_t e_int_z;
};

static inline void mahony_filter_init(struct mahony_filter *mf)
{
	mf->q0 = MAHONY_Q30_ONE;
	mf->q1 = 0;
	mf->q2 = 0;
	mf->q3 = 0;

	mf->e_int_x = 0;
	mf->e_int_y = 0;
	mf->e_int_z = 0;
}


static inline void mahony_filter_init_from_angles(struct mahony_filter *mf, const struct mf_angles *angles)
{
	mf->e_int_x = 0;
	mf->e_int_y = 0;
	mf->e_int_z = 0;

	int32_t s_r, c_r, s_p, c_p, s_y, c_y;

	/* CORDIC 求半角的 sin/cos (Q14 格式: 16384 = 1.0) */
	cordic_sincos(angles->roll / 2,  &s_r, &c_r);
	cordic_sincos(angles->pitch / 2, &s_p, &c_p);
	cordic_sincos(angles->yaw / 2,   &s_y, &c_y);

	/**
	 *	欧拉角 -> 四元数 (Q14 * Q14 * Q14 = Q42 -> 移位到 Q30)
	 *	q0 = c_r*c_p*c_y + s_r*s_p*s_y
	 *	q1 = s_r*c_p*c_y - c_r*s_p*s_y
	 *	q2 = c_r*s_p*c_y + s_r*c_p*s_y
	 *	q3 = c_r*c_p*s_y - s_r*s_p*c_y
	 */
	int64_t q0_q42 = (int64_t)c_r * c_p * c_y + (int64_t)s_r * s_p * s_y;
	int64_t q1_q42 = (int64_t)s_r * c_p * c_y - (int64_t)c_r * s_p * s_y;
	int64_t q2_q42 = (int64_t)c_r * s_p * c_y + (int64_t)s_r * c_p * s_y;
	int64_t q3_q42 = (int64_t)c_r * c_p * s_y - (int64_t)s_r * s_p * c_y;

	mf->q0 = (int32_t)(q0_q42 >> 12);
	mf->q1 = (int32_t)(q1_q42 >> 12);
	mf->q2 = (int32_t)(q2_q42 >> 12);
	mf->q3 = (int32_t)(q3_q42 >> 12);
}

static inline __attribute__((always_inline)) void mahony_filter_update(struct mahony_filter *mf,
	int16_t raw_ax, int16_t raw_ay, int16_t raw_az,
	int16_t raw_gx, int16_t raw_gy, int16_t raw_gz, const struct mf_config * cfg)

{
	if (mf->q0 > MF_Q_GUARD || mf->q0 < -MF_Q_GUARD ||
		mf->q1 > MF_Q_GUARD || mf->q1 < -MF_Q_GUARD ||
		mf->q2 > MF_Q_GUARD || mf->q2 < -MF_Q_GUARD ||
		mf->q3 > MF_Q_GUARD || mf->q3 < -MF_Q_GUARD)
	{
		mahony_filter_init(mf);
	}

	/* 1. 加速度归并处理 (使用 isqrt32) */
	uint32_t a_sq = (uint32_t)raw_ax * raw_ax + (uint32_t)raw_ay * raw_ay + (uint32_t)raw_az * raw_az;
	if (a_sq == 0)
		return;

	uint32_t a_norm = isqrt32(a_sq);

	/* 将加速度放大到 Q14 (16384 = 1.0) */
	int32_t ax_q14 = ((int32_t)raw_ax * MAHONY_Q14_ONE) / (int32_t)a_norm;
	int32_t ay_q14 = ((int32_t)raw_ay * MAHONY_Q14_ONE) / (int32_t)a_norm;
	int32_t az_q14 = ((int32_t)raw_az * MAHONY_Q14_ONE) / (int32_t)a_norm;

	/**
	 *	2. 提取估计重力向量 v = [vx, vy, vz] (由四元数 Q30 导出 Q14 向量)
	 *	vx = 2 * (q1*q3 - q0*q2)
	 *	vy = 2 * (q0*q1 + q2*q3)
	 *	vz = q0^2 - q1^2 - q2^2 + q3^2
	 */
	int64_t q0 = mf->q0, q1 = mf->q1, q2 = mf->q2, q3 = mf->q3;

	int32_t vx = (int32_t)((2 * (q0 * q2 - q1 * q3)) >> 46);
	int32_t vy = (int32_t)((2 * (q2 * q3 - q0 * q1)) >> 46);
	int32_t vz = (int32_t)((q1 * q1 + q2 * q2 - q0 * q0 - q3 * q3) >> 46);

	/* 3. 误差向量 e = a × v (叉积, 结果为 Q28, 缩放到 Q14) */
	int32_t ex = (int32_t)(((int64_t)ay_q14 * vz - (int64_t)az_q14 * vy) >> 14);
	int32_t ey = (int32_t)(((int64_t)az_q14 * vx - (int64_t)ax_q14 * vz) >> 14);
	int32_t ez = (int32_t)(((int64_t)ax_q14 * vy - (int64_t)ay_q14 * vx) >> 14);

	/* 4. PI 修正 */
	if (cfg->ki_x1000 > 0)
	{
		int64_t e_new;

		e_new = (int64_t)mf->e_int_x + (int64_t)ex * cfg->ki_x1000 * cfg->dt_ms;

		if (e_new > MF_E_INT_MAX * 1000)
			e_new = MF_E_INT_MAX * 1000;

		if (e_new < -MF_E_INT_MAX * 1000)
			e_new = -MF_E_INT_MAX * 1000;

		mf->e_int_x = (int32_t)e_new;

		e_new = (int64_t)mf->e_int_y + (int64_t)ey * cfg->ki_x1000 * cfg->dt_ms;

		if (e_new > MF_E_INT_MAX * 1000)
			e_new = MF_E_INT_MAX * 1000;

		if (e_new < -MF_E_INT_MAX * 1000)
			e_new = -MF_E_INT_MAX * 1000;

		mf->e_int_y = (int32_t)e_new;

		e_new = (int64_t)mf->e_int_z + (int64_t)ez * cfg->ki_x1000 * cfg->dt_ms;

		if (e_new > MF_E_INT_MAX * 1000)
			e_new = MF_E_INT_MAX * 1000;

		if (e_new < -MF_E_INT_MAX * 1000)
			e_new = -MF_E_INT_MAX * 1000;

		mf->e_int_z = (int32_t)e_new;
	}

	/* 陀螺仪 LSB 修正 */
	int64_t gx_corr = (int64_t)raw_gx + ((int64_t)ex * cfg->kp_x1000) / 1000 + mf->e_int_x / 1000;
	int64_t gy_corr = (int64_t)raw_gy + ((int64_t)ey * cfg->kp_x1000) / 1000 + mf->e_int_y / 1000;
	int64_t gz_corr = (int64_t)raw_gz + ((int64_t)ez * cfg->kp_x1000) / 1000 + mf->e_int_z / 1000;

	/* 5. 四元数微分积分dt 弧度增量 = gyro_corr * dt / scale*/
	int32_t half_dt_gx = GYRO_INTEGRATE(gx_corr, cfg->dt_ms, cfg->gyro_scale_num, cfg->gyro_scale_shr);
	int32_t half_dt_gy = GYRO_INTEGRATE(gy_corr, cfg->dt_ms, cfg->gyro_scale_num, cfg->gyro_scale_shr);
	int32_t half_dt_gz = GYRO_INTEGRATE(gz_corr, cfg->dt_ms, cfg->gyro_scale_num, cfg->gyro_scale_shr);

	if (half_dt_gx > MF_HALF_DT_MAX)
		half_dt_gx = MF_HALF_DT_MAX;

	if (half_dt_gy > MF_HALF_DT_MAX)
		half_dt_gy = MF_HALF_DT_MAX;

	if (half_dt_gz > MF_HALF_DT_MAX)
		half_dt_gz = MF_HALF_DT_MAX;

	if (half_dt_gx < -MF_HALF_DT_MAX)
		half_dt_gx = -MF_HALF_DT_MAX;

	if (half_dt_gy < -MF_HALF_DT_MAX)
		half_dt_gy = -MF_HALF_DT_MAX;

	if (half_dt_gz < -MF_HALF_DT_MAX)
		half_dt_gz = -MF_HALF_DT_MAX;

	int32_t dq0 = (int32_t)((-q1 * half_dt_gx - q2 * half_dt_gy - q3 * half_dt_gz) >> 19);
	int32_t dq1 = (int32_t)(( q0 * half_dt_gx + q2 * half_dt_gz - q3 * half_dt_gy) >> 19);
	int32_t dq2 = (int32_t)(( q0 * half_dt_gy - q1 * half_dt_gz + q3 * half_dt_gx) >> 19);
	int32_t dq3 = (int32_t)(( q0 * half_dt_gz + q1 * half_dt_gy - q2 * half_dt_gx) >> 19);

	q0 += dq0; q1 += dq1; q2 += dq2; q3 += dq3;

	/* 6. 四元数单位化 (防止定点数漂移溢出) */
	int64_t q_sq = (q0*q0 + q1*q1 + q2*q2 + q3*q3) >> 30;

	if (q_sq > 0 && q_sq < ((int64_t)1 << 32))
	{
		uint32_t q_norm = isqrt32((uint32_t)q_sq);
		int32_t recip = (int32_t)(((1u << 30) + q_norm / 2) / q_norm);

		mf->q0 = (int32_t)(((int64_t)q0 * recip) >> 15);
		mf->q1 = (int32_t)(((int64_t)q1 * recip) >> 15);
		mf->q2 = (int32_t)(((int64_t)q2 * recip) >> 15);
		mf->q3 = (int32_t)(((int64_t)q3 * recip) >> 15);
	}
}

#if 0	//	目前没有设备测试所以代码尚未编写,
static inline __attribute__((always_inline)) void mahony_filter_update_9dof(struct mahony_filter *mf,
	int16_t raw_ax, int16_t raw_ay, int16_t raw_az,
	int16_t raw_gx, int16_t raw_gy, int16_t raw_gz,
	int16_t raw_mx, int16_t raw_my, int16_t raw_mz, const struct mf_config * cfg)
{
}
#endif

//	注意: 如果要获取yaw, 请确保使用 update_9dof, 否则 yaw 会漂移
static inline void mahony_filter_get_euler(
	const struct mahony_filter *mf, uint8_t axes, struct mf_angles *angles)
{
	int64_t q0 = mf->q0, q1 = mf->q1, q2 = mf->q2, q3 = mf->q3;

	/* 转换 Q30 到 Q14 进行 32位 CORDIC atan2 计算 */
	int32_t q0_q14 = (int32_t)(q0 >> 16);
	int32_t q1_q14 = (int32_t)(q1 >> 16);
	int32_t q2_q14 = (int32_t)(q2 >> 16);
	int32_t q3_q14 = (int32_t)(q3 >> 16);

	if (axes & MF_AXIS_ROLL)
	{
		/* 1. Roll  = atan2(2*(q0*q1 + q2*q3), q0^2 - q1^2 - q2^2 + q3^2) */
		int32_t r_num = (int32_t)((2 * ((int64_t)q0_q14 * q1_q14 + (int64_t)q2_q14 * q3_q14)) >> 14);
		int32_t r_den = (int32_t)(((int64_t)q0_q14 * q0_q14 - (int64_t)q1_q14 * q1_q14 - (int64_t)q2_q14 * q2_q14 + (int64_t)q3_q14 * q3_q14) >> 14);
		angles->roll = cordic_atan2(r_num, r_den);
	}

	if (axes & MF_AXIS_PITCH)
	{
		/* 2. Pitch = atan2(-2*(q1*q3 - q0*q2), sqrt(1 - 4*(q1*q3 - q0*q2)^2)) */
		int32_t p_sin = (int32_t)((-2 * ((int64_t)q1_q14 * q3_q14 - (int64_t)q0_q14 * q2_q14)) >> 14);
		/* 限制在 [-1.0, 1.0] 范围内 */
		if (p_sin > MAHONY_Q14_ONE)
			p_sin = MAHONY_Q14_ONE;

		if (p_sin < -MAHONY_Q14_ONE)
			p_sin = -MAHONY_Q14_ONE;

		/* cos = sqrt(16384^2 - p_sin^2) */
		uint32_t p_cos_sq = MAHONY_Q14_MUL_ONE - (uint32_t)(p_sin * p_sin);
		int32_t p_cos = (int32_t)isqrt32(p_cos_sq);

		angles->pitch = cordic_atan2(p_sin, p_cos);
	}

	if (axes & MF_AXIS_YAW)
	{
		/* 3. Yaw   = atan2(2*(q0*q3 + q1*q2), q0^2 + q1^2 - q2^2 - q3^2) */
		int32_t y_num = (int32_t)((2 * ((int64_t)q0_q14 * q3_q14 + (int64_t)q1_q14 * q2_q14)) >> 14);
		int32_t y_den = (int32_t)(((int64_t)q0_q14 * q0_q14 + (int64_t)q1_q14 * q1_q14 - (int64_t)q2_q14 * q2_q14 - (int64_t)q3_q14 * q3_q14) >> 14);
		angles->yaw = cordic_atan2(y_num, y_den);
	}
}

#endif	//	__NEXUS_LIB_GADGET_ALGO_MAHONY_FILTER_1MS_HEADER__
