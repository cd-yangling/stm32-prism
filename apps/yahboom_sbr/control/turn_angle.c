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
#include <errno.h>
#include "control/algo/mul_shr.h"
#include "tick.h"
#include "control/turn.h"
#include "control/turn_angle.h"

#define	TURN_ANGLE_DONE_CD		300
#define	TURN_ANGLE_ERR_MAX		18000

static uint8_t			active;
static enum turn_angle_st	last;
static int32_t			target_cd;
static int32_t			yaw_cd;
static int32_t			yaw_rem;
static uint8_t			gz_still;
static uint32_t			start_tick;
static uint32_t			elapsed_ms;

uint8_t turn_angle_is_measuring(void)
{
	return active;
}

void turn_angle_update_yaw(int16_t gz, uint16_t dt_10us,
		int32_t num, int shr)
{
	int32_t	still_th;

	if (!active)
	{
		return;
	}

	yaw_cd += mul_shr_carry(&yaw_rem, (int32_t)gz * (int32_t)dt_10us,
			num, shr);

	still_th = (1 << shr) / 1000;
	gz_still = (gz < still_th && -still_th < gz);
}

int turn_angle_measuring_start(int32_t cd)
{
	if (active)
	{
		return -EBUSY;
	}

	yaw_cd = 0;
	yaw_rem = 0;
	target_cd = cd;
	start_tick = tick_get();
	active = 1u;
	last = TURN_ANGLE_RUN;

	return 0;
}

void turn_angle_measuring_abort(void)
{
	if (active)
	{
		turn_set_target(0);
		active = 0u;
		last = TURN_ANGLE_ABORTED;
		elapsed_ms = tick_get() - start_tick;
	}
}

int32_t turn_angle_yaw_get(void)
{
	return yaw_cd;
}

uint32_t turn_angle_measuring_elapsed_ms(void)
{
	if (active)
	{
		return tick_get() - start_tick;
	}

	return elapsed_ms;
}

int turn_angle_poll(const struct turn_angle_config *cfg)
{
	int32_t	err;
	int32_t	tgt;

	if (!active)
	{
		return last;
	}

	err = target_cd - yaw_cd;

	if (TURN_ANGLE_ERR_MAX < err)
	{
		err = TURN_ANGLE_ERR_MAX;
	}
	else if (-TURN_ANGLE_ERR_MAX > err)
	{
		err = -TURN_ANGLE_ERR_MAX;
	}

	if ((TURN_ANGLE_DONE_CD > err && -TURN_ANGLE_DONE_CD < err) && gz_still)
	{
		turn_set_target(0);
		active = 0u;
		last = TURN_ANGLE_DONE;
		elapsed_ms = tick_get() - start_tick;
		return TURN_ANGLE_DONE;
	}

	if (tick_expired(start_tick + cfg->timeout_ms))
	{
		turn_set_target(0);
		active = 0u;
		last = TURN_ANGLE_ETIMEOUT;
		elapsed_ms = tick_get() - start_tick;
		return TURN_ANGLE_ETIMEOUT;
	}

	tgt = mul_shr(err, cfg->kp_num, cfg->kp_shr);

	if (cfg->out_max < tgt)
	{
		tgt = cfg->out_max;
	}
	else if (-cfg->out_max > tgt)
	{
		tgt = -cfg->out_max;
	}

	turn_set_target((int16_t)tgt);

	return TURN_ANGLE_RUN;
}
