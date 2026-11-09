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

#ifndef	__APPS_YAHBOOM_SBR_CONTROL_ALGO_PID_CTRL_HEADER__
#define	__APPS_YAHBOOM_SBR_CONTROL_ALGO_PID_CTRL_HEADER__

#include <stdint.h>

struct pid_ctrl_config
{
	int16_t		kp_num;
	uint8_t		kp_shr;
	int16_t		ki_num;
	uint8_t		ki_shr;
	int16_t		kd_num;
	uint8_t		kd_shr;
	uint16_t	lpf_a;
	int16_t		out_max;
};

struct pid_ctrl_state
{
	int32_t		meas_f;
	int32_t		meas_f_rem;
	int32_t		i_sum;
	int32_t		i_rem;
	int32_t		d_f;
	int32_t		d_f_rem;
	int32_t		d_e1;
	int32_t		d_e2;
	int16_t		d_prev;
	uint8_t		d_init;
};

int16_t pid_ctrl_update(int16_t meas, int16_t target,
		struct pid_ctrl_state *pcs,
		const struct pid_ctrl_config *cfg);
void pid_ctrl_reset(struct pid_ctrl_state *pcs);

#endif	//	__APPS_YAHBOOM_SBR_CONTROL_ALGO_PID_CTRL_HEADER__
