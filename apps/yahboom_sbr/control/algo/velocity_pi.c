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
#include "control/algo/mul_shr.h"
#include "control/algo/velocity_pi.h"

int16_t velocity_pi_update(int16_t v_cnt, int16_t target,
		struct velocity_pi_state *st,
		const struct velocity_pi_config *cfg)
{
	int32_t	err;
	int32_t	i_term;
	int32_t	out;

	//	一阶 IIR 低通: vel_f = (1-a/256)*vel_f + (a/256)*v_cnt
	//	lpf_a=84 -> alpha~0.33
	//	lpf_a  alpha  v_cnt     延迟@10ms 
	//	   8   0.031     32        320 ms
	//	  16   0.063     16        160 ms
	//	  32   0.125      8         80 ms
	//	  64   0.250      4         40 ms
	//	  84   0.328      4         40 ms
	//	 128   0.500      2         20 ms
	//	 192   0.750      2         20 ms
	//	 255   0.996      2         20 ms
	st->vel_f += mul_shr(v_cnt - st->vel_f, cfg->lpf_a, 8);

	err = st->vel_f - target;

	//	本次积分增量
	i_term = mul_shr_carry(&st->ki_rem, err, cfg->ki_num, cfg->ki_shr);
//	i_term = mul_shr_carry(NULL, err, cfg->ki_num, cfg->ki_shr);

	//	P + 历史积分 + 本次增量积分
	out = mul_shr(err, cfg->kp_num, cfg->kp_shr)
		+ st->integral + i_term;

	//	条件积分抗饱和
	if (-cfg->off_max <= out && out <= cfg->off_max)
	{
		st->integral += i_term;
	}

	//	输出限幅
	if (out < -cfg->off_max)
	{
		out = -cfg->off_max;
	}

	if (out > cfg->off_max)
	{
		out = cfg->off_max;
	}

	return (int16_t)out;
}

void velocity_pi_reset(struct velocity_pi_state *st)
{
	st->vel_f = 0;
	st->integral = 0;
	st->ki_rem = 0;
}
