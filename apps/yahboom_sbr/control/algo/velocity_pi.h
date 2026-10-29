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

#ifndef	__APPS_YAHBOOM_SBR_CONTROL_ALGO_VELOCITY_PI_HEADER__
#define	__APPS_YAHBOOM_SBR_CONTROL_ALGO_VELOCITY_PI_HEADER__

#include <stdint.h>

struct velocity_pi_config
{
	int16_t		kp_num;
	uint8_t		kp_shr;
	int16_t		ki_num;
	uint8_t		ki_shr;
	uint16_t	lpf_a;
	int16_t		off_max;
};

struct velocity_pi_state
{
	int32_t		vel_f;
	int32_t		integral;
	int32_t		ki_rem;
};

int16_t velocity_pi_update(int16_t v_cnt, int16_t target,
		struct velocity_pi_state *st,
		const struct velocity_pi_config *cfg);
void velocity_pi_reset(struct velocity_pi_state *st);

/*
 * velocity_pi_update 域与极性:
 *   v_cnt:  单位: cnt/s, 前进为正
 *   target: 同单位, 正 = 目标前进
 *   返回:   pitch 平衡点偏移(centidegree), 串级注入直立环:
 *           err_cd = pitch - (mid_cd + off), 正 = 抬头偏(减速方向)
 *
 *   P: kp_num=256, 256*err_cnt >> 8 = 1.0 cd/(cnt/s)
 *   I: ki_num=26,  26*err_cnt >> 8  = 0.10 cd/(cnt/s), @100Hz = 10 cd/s
 *
 *   lpf_a: 反馈一阶 IIR 低通, alpha = lpf_a/256, 84 -> 0.33
 *   off_max: 输出限幅(+/-off_max), 越限时冻结积分(抗饱和)
 */

#define	VELOCITY_PI_CONFIG_INIT			\
{										\
	.kp_num		= 0,					\
	.kp_shr		= 12,					\
	.ki_num		= 0,					\
	.ki_shr		= 15,					\
	.lpf_a		= 84,					\
	.off_max	= 1000,					\
}

#endif	//	__APPS_YAHBOOM_SBR_CONTROL_ALGO_VELOCITY_PI_HEADER__
