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
#include "lib/gadget/fxp_mul.h"
#include "tick.h"
#include "control/velocity.h"
#include "control/move_dist.h"

//	亚博智能小车官方轮径 67mm 减速比 1:30 编码器线数 11 ppr
//	编码器 x4 44*30=1320 cnt/轮圈, 周长 pi*67=210.5mm
//	vsum 1mm 车体位移 = 2*1320/210.5 = 12.54 cnt
#define	MOVE_DIST_CNT_PER_MM	13
#define	MOVE_DIST_TARGET_MAX_MM	2000
#define	MOVE_DIST_DONE_ERR_CNT	130
#define	MOVE_DIST_VSUM_SETTLED	200
#define	MOVE_DIST_ERR_MAX_CNT	(MOVE_DIST_TARGET_MAX_MM * MOVE_DIST_CNT_PER_MM)

static enum move_dist_st	last;
static int32_t			target_cnt;
static int32_t			dist_cnt;
static int32_t			vsum_rem;
static uint32_t			start_tick;
static uint32_t			elapsed_ms;

int move_dist_measuring_start(int32_t mm)
{
	if (MOVE_DIST_RUN == last)
	{
		return -EBUSY;
	}

	dist_cnt = 0;
	vsum_rem = 0;
	target_cnt = mm * MOVE_DIST_CNT_PER_MM;
	start_tick = tick_get();
	last = MOVE_DIST_RUN;

	return 0;
}

void move_dist_measuring_abort(void)
{
	if (MOVE_DIST_RUN == last)
	{
		velocity_set_target(0);
		last = MOVE_DIST_ABORTED;
		elapsed_ms = tick_get() - start_tick;
	}
}

int move_dist_state_get(void)
{
	return last;
}

int32_t move_dist_dist_get(void)
{
	return dist_cnt;
}

uint32_t move_dist_measuring_elapsed_ms(void)
{
	if (MOVE_DIST_RUN == last)
	{
		return tick_get() - start_tick;
	}

	return elapsed_ms;
}

void move_dist_poll(int16_t vsum, const struct move_dist_config *cfg)
{
	int32_t	err;
	int32_t	tgt;

	if (MOVE_DIST_RUN != last)
	{
		return;
	}

	vsum_rem += vsum;
	dist_cnt += vsum_rem / 100;
	vsum_rem %= 100;

	err = target_cnt - dist_cnt;

	if (MOVE_DIST_ERR_MAX_CNT < err)
	{
		err = MOVE_DIST_ERR_MAX_CNT;
	}
	else if (-MOVE_DIST_ERR_MAX_CNT > err)
	{
		err = -MOVE_DIST_ERR_MAX_CNT;
	}

	if ((MOVE_DIST_DONE_ERR_CNT > err && -MOVE_DIST_DONE_ERR_CNT < err)
			&& (MOVE_DIST_VSUM_SETTLED > vsum
				&& -MOVE_DIST_VSUM_SETTLED < vsum))
	{
		velocity_set_target(0);
		last = MOVE_DIST_DONE;
		elapsed_ms = tick_get() - start_tick;
		return;
	}

	if (tick_expired(start_tick + cfg->timeout_ms))
	{
		velocity_set_target(0);
		last = MOVE_DIST_ETIMEOUT;
		elapsed_ms = tick_get() - start_tick;
		return;
	}

	tgt = fxp_mul32(err, cfg->kp_num, cfg->kp_shr);

	if (cfg->out_max < tgt)
	{
		tgt = cfg->out_max;
	}
	else if (-cfg->out_max > tgt)
	{
		tgt = -cfg->out_max;
	}

	velocity_set_target((int16_t)tgt);
}
