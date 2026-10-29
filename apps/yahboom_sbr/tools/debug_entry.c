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

#include "die.h"
#include "tools/debug_misc.h"
#include "tools/debug_mpu6050.h"
#include "tools/debug_entry.h"
#include "tools/debug_motor.h"

void debug_entry(void)
{
//	用于测试单次 I2C-bitbang 消耗的时间
//	debug_mpu6050_sample_fetch_time();

//	用于 gyro 零偏校准测量
//	debug_mpu6050_gyro_bias();

//	用于 观察 x/y/z 方向
//	debug_mpu6050_dump_raw();

//	用于 VOFA 上观察 pitch 角度解算+互补滤波结果的测试
//	debug_mpu6050_pitch_resolve();

//	OLED 测试
//	debug_misc_test_oled();

//	电池电压测试
//	debug_misc_test_battery_volotage();

//	蜂鸣器测试
//	debug_misc_test_buzzer();

//	编码器测试(手拨车轮看方向与计数, 定 enc_dir)
//	debug_misc_test_encoder();

//	电机基本测试
//	debug_motor_basic_test();

// 1/2: 方向标定测试(标定完毕,写board.c的配置)
//	debug_motor_calibrate_dir();

// 3:死区标定,轮始动瞬间记下(标定完毕,写board.c的配置)
//	debug_motor_calibrate_deadband();

//	4:满占空极速测试(危险: key1 确认后满转 3s 自动刹停, 定极速/守卫常数)
//	debug_motor_full_speed();

	die();
}
