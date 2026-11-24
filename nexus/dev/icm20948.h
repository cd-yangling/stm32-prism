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

#ifndef	__NEXUS_DEV_ICM20948_HEADER__
#define	__NEXUS_DEV_ICM20948_HEADER__

#include <stdint.h>
#include "pal/dev.h"

struct i2c_dev;
struct sensor_imu_axis_map;
struct sensor_ops;
struct icm20948_dat;

#ifdef CONFIG_ICM20948_MAGN
#define	ICM20948_DAT_SIZE	38
#else
#define	ICM20948_DAT_SIZE	26
#endif
#define	ICM20948_DAT(name)	OPAQUE_INST(struct icm20948_dat, name, ICM20948_DAT_SIZE)

struct icm20948_dev
{
	const struct i2c_dev				*	i2c;
	uint8_t									gyro_smplrt_div;
	uint8_t									accel_smplrt_div;
	uint8_t									gyro_dlpf_cfg;
	uint8_t									accel_dlpf_cfg;
	uint8_t									gfs_sel;
	uint8_t									afs_sel;
	struct icm20948_dat					*	dat;
	const struct sensor_imu_axis_map	*	axis_map;
#ifdef CONFIG_ICM20948_MAGN
	uint8_t									mag_mode;
	const struct sensor_imu_axis_map	*	mag_axis_map;
#endif
};

extern const struct sensor_ops icm20948_sensor_ops;

#endif	//	__NEXUS_DEV_ICM20948_HEADER__
