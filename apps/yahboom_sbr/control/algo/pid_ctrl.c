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

#include <stdint.h>
#include "lib/gadget/fxp_mul.h"
#include "control/algo/pid_ctrl.h"

#define	PID_D_LPF_A	64

static int32_t med3(int32_t a, int32_t b, int32_t c)
{
	if (a < b)
	{
		if (b < c)
		{
			return b;
		}

		if (a < c)
		{
			return c;
		}

		return a;
	}

	if (a < c)
	{
		return a;
	}

	if (b < c)
	{
		return c;
	}

	return b;
}

static void d_update(int16_t meas, struct pid_ctrl_state *pcs,
			const struct pid_ctrl_config *cfg)
{
	if (0u != pcs->d_init)
	{
		int32_t	d_e0;
		int32_t	med;

		d_e0 = meas - pcs->d_prev;
		med = med3(pcs->d_e2, pcs->d_e1, d_e0);
		pcs->d_f += fxp_mul32_rem(&pcs->d_f_rem,
				fxp_mul32(med, cfg->kd_num, cfg->kd_shr) - pcs->d_f,
				PID_D_LPF_A, 8);
		pcs->d_e2 = pcs->d_e1;
		pcs->d_e1 = d_e0;
		pcs->d_prev = meas;
	}
	else
	{
		pcs->d_init = 1u;
		pcs->d_prev = meas;
		pcs->d_e1 = 0;
		pcs->d_e2 = 0;
	}
}

int16_t pid_ctrl_update(int16_t meas, int16_t target,
		struct pid_ctrl_state *pcs,
		const struct pid_ctrl_config *cfg)
{
	int32_t	err;
	int32_t	i_delta;
	int32_t	out;

	//	微分先行: 差分取自 meas
	d_update(meas, pcs, cfg);

	//	一阶 IIR 低通: meas_f = (1-a/256)*meas_f + (a/256)*meas
	//	lpf_a=84 -> alpha~0.33
	//	lpf_a   alpha   meas     延迟@10ms
	//	    8   0.031     32        320 ms
	//	   16   0.063     16        160 ms
	//	   32   0.125      8         80 ms
	//	   64   0.250      4         40 ms
	//	   84   0.328      4         40 ms
	//	  128   0.500      2         20 ms
	//	  192   0.750      2         20 ms
	//	  255   0.996      2         20 ms
	pcs->meas_f += fxp_mul32_rem(&pcs->meas_f_rem,
				meas - pcs->meas_f, cfg->lpf_a, 8);

	err = pcs->meas_f - target;

	//	本次积分增量
	i_delta = fxp_mul32_rem(&pcs->i_rem, err, cfg->ki_num, cfg->ki_shr);

	//	P + 历史积分 + 本次增量积分
	out = fxp_mul32(err, cfg->kp_num, cfg->kp_shr)
		+ pcs->i_sum + i_delta + pcs->d_f;

	//	条件积分抗饱和
	if (-cfg->out_max <= out && out <= cfg->out_max)
	{
		pcs->i_sum += i_delta;
	}

	//	输出限幅
	if (out < -cfg->out_max)
	{
		out = -cfg->out_max;
	}

	if (out > cfg->out_max)
	{
		out = cfg->out_max;
	}

	return (int16_t)out;
}

void pid_ctrl_reset(struct pid_ctrl_state *pcs)
{
	pcs->meas_f = 0;
	pcs->meas_f_rem = 0;
	pcs->i_sum = 0;
	pcs->i_rem = 0;
	pcs->d_f = 0;
	pcs->d_f_rem = 0;
	pcs->d_e1 = 0;
	pcs->d_e2 = 0;
	pcs->d_prev = 0;
	pcs->d_init = 0u;
}
