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

#ifndef	__APPS_YAHBOOM_SBR_CONTROL_ALGO_TURN_PD_HEADER__
#define	__APPS_YAHBOOM_SBR_CONTROL_ALGO_TURN_PD_HEADER__

#include <stdint.h>

struct turn_pd_config
{
	int16_t		kd_num;
	uint8_t		kd_shr;
	int16_t		target;
	int16_t		out_max;
};

int16_t turn_pd_update(int16_t gyro_lsb, int16_t target_lsb,
		const struct turn_pd_config *cfg);

#define	TURN_PD_CONFIG_INIT			\
{									\
	.kd_num		= 0,				\
	.kd_shr		= 16,				\
	.target		= 0,				\
	.out_max	= 1000,				\
}

#endif	//	__APPS_YAHBOOM_SBR_CONTROL_ALGO_TURN_PD_HEADER__
