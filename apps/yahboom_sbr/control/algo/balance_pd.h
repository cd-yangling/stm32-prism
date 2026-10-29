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

#ifndef	__APPS_YAHBOOM_SBR_CONTROL_ALGO_BALANCE_PD_HEADER__
#define	__APPS_YAHBOOM_SBR_CONTROL_ALGO_BALANCE_PD_HEADER__

#include <stdint.h>

struct balance_pd_config
{
	int16_t		kp_num;
	uint8_t		kp_shr;
	int16_t		kd_num;
	uint8_t		kd_shr;
	int16_t		out_max;
	int16_t		out_min;
};

int16_t balance_pd_update(int32_t err_cd, int16_t gyro_lsb, const struct balance_pd_config *cfg);

/*
 * balance_pd_update 域与极性:
 *   err_cd:   单位: centidegree, pitch 抬头为正
 *   gyro_lsb: raw LSB (deg/s), 抬头角速率为正
 *   返回:     电机千分数(motor_set, 满量程 1000), 正 = 前进
 *
 * 姿态到电机的极性翻转在函数内统一处理
 *
 *   P: kp_num=300,  300*err_cd >> 10   = 0.293 千分/cd = 29.3 千分/deg
 *   D: kd_num=2000, 2000*gyro_lsb >> 16 = 0.0305 千分/LSB
 *                    @ 65.5 LSB/(deg/s) = 2.0 千分/(deg/s)
 *
 * 换量程 kd_num 按满量程等比缩放(kd_num*灵敏度)
 *   250dps:1000  500dps:2000  1000dps:4000  2000dps:8000
 */
#define	BALANCE_PD_CONFIG_INIT				\
{											\
	.kp_num		= 0,						\
	.kp_shr		= 10,						\
	.kd_num		= 0,						\
	.kd_shr		= 16,						\
	.out_max	= 1000,						\
	.out_min	= 15,						\
}

#endif	//	__APPS_YAHBOOM_SBR_CONTROL_ALGO_BALANCE_PD_HEADER__
