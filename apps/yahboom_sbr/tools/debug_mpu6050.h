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

#ifndef	__APPS_YAHBOOM_SBR_TOOLS_DEBUG_MPU6050_HEADER__
#define	__APPS_YAHBOOM_SBR_TOOLS_DEBUG_MPU6050_HEADER__

//	用于 gyro 零偏校准测量
void debug_mpu6050_gyro_bias(void);

//	用于 观察 x/y/z 方向
void debug_mpu6050_dump_raw(void);

//	用于测试单次 I2C-bitbang 消耗的时间
void debug_mpu6050_sample_fetch_time(void);

//	用于 VOFA 上观察 pitch 角度解算+互补滤波结果的测试
void debug_mpu6050_pitch_resolve(void);

#endif	//	__APPS_YAHBOOM_SBR_TOOLS_DEBUG_MPU6050_HEADER__
