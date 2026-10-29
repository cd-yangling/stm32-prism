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

//	本代码参考 意法半导体(STMicroelectronics) DT0087 文档编写
//	https://www.st.com/resource/en/design_tip/dt0087-coordinate-rotation-digital-computer-algorithm-cordic-test-and-performance-verification-stmicroelectronics.pdf
/**
 *	纯整数 IMU 数学实现: CORDIC atan2, LUT atan2, 整数 sqrt (考虑给无硬件FPU使用)
 *
 * ============================================================
 * 角度单位约定(本库统一使用 "百分之一度" = centidegree)
 *   1 centidegree = 0.01度
 *   例: 4500 = 45.00度,  -9000 = -90.00度,  18000 = 180.00度
 * ============================================================
 *         1: 惯性测量单元的数据结算
 *         2: 互补滤波姿态数据的融合
 *         3: mahony  姿态数据的融合
 *
 *  CORDIC atan2 与 LUT atan2 可以任意选择一个来使用
 *
 * 所有函数: 无 float, 无 double, 无 FPU 指令
 *
 * CORDIC: 额外无除法 (只有加减法 + 算术移位)
 *    LUT: 存在2次除法
 *
 * LUT 和 CORDIC 的本质区别:
 *      LUT: 用 "空间换时间" (查表方式), 但无法彻底消除除法
 *   CORDIC: 用 "时间换空间" (迭代计算), 彻底消除除法和乘法
 */

#include "imu_math.h"
#include <stddef.h>

/* ============================================================
 *  整数平方根 - 逐位确定算法
 * ============================================================
 *
 * 原理: 每次迭代确定结果的一个二进制位
 *       从最高位到最低位逐一试探
 *
 * 完全无除法, 只有移位 + 加减 + 比较
 * 16次迭代 -> 计算32位无符号数的精确整数平方根
 *
 * 例: isqrt32(100) = 10, isqrt32(99) = 9
 */
uint32_t isqrt32(uint32_t n)
{
	uint32_t result = 0;
	/* bit 从最高的 4的幂次 开始 (2^30 = 最高的4的幂次 <= 2^31-1) */
	uint32_t bit = (1UL << 30);

	if (0 == n)
		return 0;

	/* 找到不超过 n 的最高 4 的幂 */
	while (bit > n)
		bit >>= 2;

	while (bit != 0)
	{
		if (n >= result + bit)
		{
			n		-=	(result >> 0) + bit;
			result  =	(result >> 1) + bit;
		}
		else
		{
			result >>= 1;
		}
		bit >>= 2;
	}

	return result;
}

/* ============================================================
 *  CORDIC atan2 - 矢量模式 (Vectoring Mode)
 * ============================================================
 *
 * 核心思想:
 *   给定向量 (x, y), 通过一系列微小旋转把 y 逼近 0
 *   旋转角度 = atan(2^-i) (预存在表中)
 *   最终累计的旋转角 = atan2(y_initial, x_initial)
 *
 * 每步旋转公式 (仅移位+加减, 无乘除):
 *   if (y >= 0)                 // 需要顺时针旋转, 让 y 减小
 *     x_new = x + (y >> i)      // cos 项近似
 *     y_new = y - (x >> i)      // sin 项近似
 *     angle += atan(2^-i)
 *   else
 *     x_new = x - (y >> i)      // 逆时针
 *     y_new = y + (x >> i)
 *     angle -= atan(2^-i)
 *
 * 收敛性:
 *   14次迭代后 y 趋近 0, 误差 < 0.01度
 *   x 趋近 (1/K) * sqrt(x0^2 + y0^2), 其中 1/K ~=1.6468 (放大)
 *
 * 溢出分析 (int16_t 输入):
 *   最大幅值增益 = 1/K ~=1.6468
 *   最大中间值   = sqrt(2) * 32767 * 1.6468 ~= 76300 << INT32_MAX
 */

/* atan(2^-i) 预算表, 单位: centidegree (0.01度)
 * 计算方法: atan(2^-i) * 180/PI * 100
 * i=0:  atan(1)     = 45.0000度-> 4500
 * i=1:  atan(0.5)   = 26.5651度-> 2657 (精确值2656.5, 取整2657)
 * i=2:  atan(0.25)  = 14.0362度-> 1404
 * ...
 */
static const int32_t s_cordic_atan_tbl[14] =
{
	4500,  /* i=0:  atan(2^0)   = 45.0000度 */
	2657,  /* i=1:  atan(2^-1)  = 26.5651度 */
	1404,  /* i=2:  atan(2^-2)  = 14.0362度 */
	713,   /* i=3:  atan(2^-3)  =  7.1250度 */
	358,   /* i=4:  atan(2^-4)  =  3.5763度 */
	179,   /* i=5:  atan(2^-5)  =  1.7899度 */
	90,    /* i=6:  atan(2^-6)  =  0.8952度 */
	45,    /* i=7:  atan(2^-7)  =  0.4476度 */
	22,    /* i=8:  atan(2^-8)  =  0.2238度 */
	11,    /* i=9:  atan(2^-9)  =  0.1119度 */
	6,     /* i=10: atan(2^-10) =  0.0560度 */
	3,     /* i=11: atan(2^-11) =  0.0280度 */
	1,     /* i=12: atan(2^-12) =  0.0140度 */
	1,     /* i=13: atan(2^-13) =  0.0070度 */
};

int32_t cordic_atan2_with_mag(int32_t y, int32_t x, int32_t *magnitude_out)
{
	int32_t angle = 0;

	/* ---- 象限预处理 ----
	 * CORDIC 只在 x > 0 时收敛.
	 * x < 0 时: 先旋转 +/- 180度 映射到右半平面, 再迭代
	 */
	if (x < 0)
	{
		/* 第2, 3象限: 旋转 +/- 180度 */
		angle = (y >= 0) ? ANGLE_180DEG : -ANGLE_180DEG;
		x = -x;
		y = -y;
	}
	else if (x == 0)
	{
		/* 零向量: 无方向, 与 lut_atan2 对齐返 0 */
		if (0 == y)
		{
			if (magnitude_out)
			{
				*magnitude_out = 0;
			}

			return 0;
		}

		/* 纯虚轴: 直接返回 +/- 90度 */
		if (magnitude_out)
		{
			*magnitude_out = (y < 0) ? -y : y;
		}

		return (y >= 0) ? ANGLE_90DEG : -ANGLE_90DEG;
	}

	/* ---- CORDIC 迭代 14次 ---- */
	for (int i = 0; i < 14; i++)
	{
		int32_t xn, yn;

		if (y < 0)
		{
			/* y < 0: 需要逆时针旋转让 y 增大 */
			xn = x - (y >> i);
			yn = y + (x >> i);
			angle -= s_cordic_atan_tbl[i];
		}
		else
		{
			/* y >= 0: 顺时针旋转让 y 减小 */
			xn = x + (y >> i);
			yn = y - (x >> i);
			angle += s_cordic_atan_tbl[i];
		}

		x = xn;
		y = yn;
	}

	/* 迭代结束:
	 *   x ~= (1/K) * sqrt(x0^2 + y0^2)  (1/K ~= 1.6468, 放大)
	 *   y ~= 0
	 *   angle = atan2(y0, x0)
	 */
	if (magnitude_out)
	{
		*magnitude_out = x;
	}

	return angle;
}

int32_t cordic_atan2(int32_t y, int32_t x)
{
	return cordic_atan2_with_mag(y, x, NULL);
}

/* ============================================================
 *  CORDIC sin / cos - 旋转模式 (Rotation Mode)
 * ============================================================
 *
 * 原理:
 *   从初始向量 (x=CORDIC_K, y=0) 开始旋转指定角度 angle
 *   CORDIC_K ~= 0.60725293500888125
 *   在 Q14 下, 1.0 = 16384, K_Q14 = 0.6072529 * 16384 ~=9949
 *
 * 每次迭代:
 *   if (angle >= 0)  // 逆时针旋转
 *     x_new = x - (y >> i)
 *     y_new = y + (x >> i)
 *     angle -= atan(2^-i)
 *   else             // 顺时针旋转
 *     x_new = x + (y >> i)
 *     y_new = y - (x >> i)
 *     angle += atan(2^-i)
 *
 * 迭代 14 次后:
 *   x ~=cos(angle) * 16384 (Q14)
 *   y ~=sin(angle) * 16384 (Q14)
 */
void cordic_sincos(int32_t angle_centideg, int32_t *sin_q14, int32_t *cos_q14)
{
	/* 规范化角度到 [-18000, 18000] (即 [-180度, +180度]) */
	while (angle_centideg > ANGLE_180DEG)
		angle_centideg -= ANGLE_360DEG;

	while (angle_centideg < -ANGLE_180DEG)
		angle_centideg += ANGLE_360DEG;

	/* 处理第2,3象限 (|angle| > 90度)
	 * 将角度折射到第1,4象限 [-90度, +90度], 旋转结束后再翻转符号 */
	int sign_cos = 1;

	if (angle_centideg > ANGLE_90DEG)
	{
		angle_centideg = ANGLE_180DEG - angle_centideg;
		sign_cos = -1;
	}
	else if (angle_centideg < -ANGLE_90DEG)
	{
		angle_centideg = -ANGLE_180DEG - angle_centideg;
		sign_cos = -1;
	}

	/* 初始向量: x0 = CORDIC_K (Q14 格式: 0.6072529 * 16384 = 9949), y0 = 0 */
	int32_t x = 9949;
	int32_t y = 0;
	int32_t current_angle = angle_centideg;

	for (int i = 0; i < 14; i++)
	{
		int32_t xn, yn;

		if (current_angle >= 0)
		{
			xn = x - (y >> i);
			yn = y + (x >> i);
			current_angle -= s_cordic_atan_tbl[i];
		}
		else
		{
			xn = x + (y >> i);
			yn = y - (x >> i);
			current_angle += s_cordic_atan_tbl[i];
		}

		x = xn;
		y = yn;
	}

	if (cos_q14)
		*cos_q14 = sign_cos * x;

	if (sin_q14)
		*sin_q14 = y;
}

/* ============================================================
 *  LUT atan2 - 查表 + 线性插值
 * ============================================================
 *
 * 原理:
 *   atan2(y, x) 可分解为:
 *     1. 确定象限 (符号判断)
 *
 *     2. 计算 r = min(|y|,|x|) / max(|y|,|x|) in [0,1]
 *        -> 此步需要 1次除法 <-
 *
 *     3. 查表 atan_lut[r * 32] 并线性插值
 *        -> 插值需要 1次除法 <-
 *
 *     4. 如果 |y| > |x|, 用 90度 - atan(r) 换算
 *
 *     5. 根据象限调整符号
 *
 * atan(i/32) 表, i=0..32, 单位 centidegree
 * 步长 = 1/32 = 0.03125, 对应最大角度步长约 1.8度
 * 线性插值后误差 < 0.06度
 */
static const int16_t s_atan_lut32[33] =
{
	0,     /* atan(0/32)  =  0.0000度 */
	179,   /* atan(1/32)  =  1.7899度 */
	358,   /* atan(2/32)  =  3.5763度 */
	536,   /* atan(3/32)  =  5.3570度 */
	713,   /* atan(4/32)  =  7.1250度 */
	888,   /* atan(5/32)  =  8.8809度 */
	1062,  /* atan(6/32)  = 10.6200度 */
	1234,  /* atan(7/32)  = 12.3430度 */
	1404,  /* atan(8/32)  = 14.0362度 */
	1570,  /* atan(9/32)  = 15.7004度 */
	1735,  /* atan(10/32) = 17.3541度 */
	1899,  /* atan(11/32) = 18.9854度 */
	2056,  /* atan(12/32) = 20.5560度 */
	2211,  /* atan(13/32) = 22.1095度 */
	2363,  /* atan(14/32) = 23.6296度 */
	2510,  /* atan(15/32) = 25.0992度 */
	2657,  /* atan(16/32) = 26.5651度 */
	2798,  /* atan(17/32) = 27.9834度 */
	2937,  /* atan(18/32) = 29.3558度 */
	3070,  /* atan(19/32) = 30.6866度 */
	3201,  /* atan(20/32) = 32.0054度 */
	3327,  /* atan(21/32) = 33.2701度 */
	3451,  /* atan(22/32) = 34.5050度 */
	3570,  /* atan(23/32) = 35.7044度 */
	3687,  /* atan(24/32) = 36.8699度 */
	3800,  /* atan(25/32) = 38.0000度 (tan38度~=0.7813~=25/32) */
	3909,  /* atan(26/32) = 39.0926度 */
	4015,  /* atan(27/32) = 40.1542度 */
	4118,  /* atan(28/32) = 41.1836度 */
	4218,  /* atan(29/32) = 42.1823度 */
	4315,  /* atan(30/32) = 43.1501度 */
	4409,  /* atan(31/32) = 44.0893度 */
	4500,  /* atan(32/32) = 45.0000度 */
};

int32_t lut_atan2(int32_t y, int32_t x)
{
	/* 处理特殊情况 */
	if (x == 0 && y == 0)
		return 0;

	if (x == 0)
		return (y > 0) ? ANGLE_90DEG : -ANGLE_90DEG;

	if (y == 0)
		return (x > 0) ? ANGLE_0DEG  : ANGLE_180DEG;

	int32_t ay = (y < 0) ? -y : y;  /* |y| */
	int32_t ax = (x < 0) ? -x : x;  /* |x| */

	/* 保证 ax >= ay, 使比值 r = ay/ax in [0, 1]
	 * 若原来 ay > ax, 记 swapped=1, 最后用 90度 - angle 换算 */
	int swapped = 0;

	if (ay > ax)
	{
		int32_t tmp = ay; ay = ax; ax = tmp;
		swapped = 1;
	}

	/* --- 第1次除法: 求 LUT 索引 ---
	 * idx = floor(ay/ax * 32) = floor((ay << 5) / ax)
	 * remainder = (ay << 5) - idx * ax  (用于插值)
	 */
#ifdef LUT_ATAN2_INPUT_16BIT
	int32_t num   = ay << 5;          /* ay * 32 */
	int32_t idx   = num / ax;         /* 0 .. 32 (第1次除法) */
	int32_t rem   = num - idx * ax;   /* 余数 [0, ax) */
#else
	int64_t num   = (int64_t)ay << 5;   /* ay * 32 */
	int32_t idx   = (int32_t)(num / ax);  /* 0 .. 32 (第1次除法) */
	int32_t rem   = (int32_t)(num - (int64_t)idx * ax);  /* 余数 [0, ax) */
#endif

	/* --- 线性插值 ---
	 * 当 idx >= 32 时 (即 |y| == |x|),直接等于 45.00度 (4500),无需插值也不越界
	 */
	int32_t angle;
	if (idx >= 32)
	{
		angle = 4500;
	}
	else
	{
		int32_t a0 = s_atan_lut32[idx];
		int32_t a1 = s_atan_lut32[idx + 1];
#ifdef LUT_ATAN2_INPUT_16BIT
		angle = a0 + ((a1 - a0) * rem) / ax;  /* 第2次除法 */
#else
		angle = a0 + (int32_t)(((int64_t)(a1 - a0) * rem) / ax);  /* 第2次除法 */
#endif
	}

	/* 如果做了交换, 用 90度 - angle 还原 */
	if (swapped)
		angle = ANGLE_90DEG - angle;

	/* 象限修正 */
	if (x < 0)
		angle = ANGLE_180DEG - angle;

	if (y < 0)
		angle = -angle;

	return angle;
}
