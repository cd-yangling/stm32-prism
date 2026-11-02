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
#include "tuning.h"
#include "control/algo/pi_ctrl.h"
#include "control/velocity.h"

static int16_t	off_cd;
static int16_t	vel_target;
static struct pi_ctrl_state pi_st;

void velocity_init(void)
{
	velocity_reset();
}

int16_t velocity_poll(int16_t v_l, int16_t v_r)
{
	off_cd = pi_ctrl_update(v_l + v_r, vel_target, &pi_st, &tune_vel);

	return off_cd;
}

void velocity_reset(void)
{
	vel_target = 0;
	off_cd = 0;
	pi_ctrl_reset(&pi_st);
}

void velocity_set_target(int16_t target)
{
	vel_target = target;
}

int16_t velocity_get_target(void)
{
	return vel_target;
}
