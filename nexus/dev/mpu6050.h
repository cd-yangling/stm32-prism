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

#ifndef	__NEXUS_DEV_MPU6050_HEADER__
#define	__NEXUS_DEV_MPU6050_HEADER__

#include <stdint.h>
#include "pal/dev.h"

struct i2c_dev;
struct pin_desc;
struct sensor_ops;
struct mpu6050_dat;

#define	MPU6050_DAT_SIZE	44
#define	MPU6050_DAT(name)	OPAQUE_INST(struct mpu6050_dat, name, MPU6050_DAT_SIZE)

/*
 * 参考 (https://en.wikipedia.org/wiki/Axes_conventions)
 * 参考 (https://www.mathworks.com/help/uav/ug/coordinate-systems-in-uav-toolbox.html)
 * 参考 (https://px4.gitbook.io/px4-user-guide/robotics/ros/ros1/external_position_estimation)
 *
 * 逻辑轴向定义
 *
 * x箭头指向 = 前方
 * y箭头指向 = 右方
 * z箭头指向 = 下方 
 *
 * 右手定则: 右手拇指指向某个轴的正方向, 四指弯曲的方向就是绕该轴旋转的正方向
 *
 * 姿态角约定
 *
 * pitch(俯仰): 抬头为正,低头为负
 *   yaw(偏航): 向右转为正,向左转为负
 *  roll(横滚): 肩膀(左高右低为正) 肩膀(左低右高为负)
 *
 * 逻辑轴向与芯片的物理安装无关
 * 驱动出口保证输出即此轴序, 后续所有的算法一律按逻辑轴向编码
*
 *	芯片轴向 -> 逻辑轴向 映射表
 *
 *	axis_map = NULL 芯片轴向 = 逻辑轴向 x/y/z 直出, 不做映射
 */
#define	MPU6050_AXIS_X	0u
#define	MPU6050_AXIS_Y	1u
#define	MPU6050_AXIS_Z	2u
struct mpu6050_axis_map
{
	struct
	{
		uint8_t	src;	//	取芯片轴向哪个通道 MPU6050_AXIS_*
		uint8_t neg;	//	非0 表示取反
	} x, y, z;
};

//	芯片 x = 车体右方
//	芯片 y = 车体前方
//	芯片 z = 车体上方
#define	MPU6050_AXIS_MAP_XRIGHT_YFRONT_ZUP_INIT	{	\
	.x = { .src = MPU6050_AXIS_Y, .neg = 0 },	\
	.y = { .src = MPU6050_AXIS_X, .neg = 0 },	\
	.z = { .src = MPU6050_AXIS_Z, .neg = 1 }	\
}

struct mpu6050_dev
{
	const struct i2c_dev			*	i2c;
	const struct pin_desc			*	int_pin;
	uint8_t								smplrt_div;
	uint8_t								dlpf_cfg;
	uint8_t								gfs_sel;
	uint8_t								afs_sel;
	struct mpu6050_dat				*	dat;
	const struct mpu6050_axis_map	*	axis_map;
};

extern const struct sensor_ops mpu6050_sensor_ops;

#endif	//	__NEXUS_DEV_MPU6050_HEADER__
