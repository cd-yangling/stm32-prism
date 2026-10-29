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
#include "board.h"
#include "app_die.h"
#include "motor.h"
#include "pal/led.h"
#include "tuning.h"
#include "lib/gadget/algo/comp_filter_10us.h"
#include "control/algo/balance_pd.h"
#include "control/algo/turn_pd.h"
#include "control/mt_speed.h"
#include "control/velocity.h"
#include "control/imu.h"
#include "control/control.h"

static const struct control_posture
{
	int16_t	mid_cd;
	int16_t	stop_cd;
	int16_t	resume_cd;
} posture =
{
	.mid_cd		= -450,
	.stop_cd	= 4500,
	.resume_cd	= 1500,
};

static struct cf_angles angles;

static uint8_t fallen;

static void motors_write(int16_t l, int16_t r)
{
	motor_set(board.motor_l, l);
	motor_set(board.motor_r, r);
}

static void attitude_update(struct imu_raw_sample *raw, uint16_t dt_10us)
{
	struct cf_config cfg = CF_CONFIG_500DPS_INIT(dt_10us, 6);

	imu_fetch(raw);

	comp_filter_update(
			raw->ax, raw->ay, raw->az,
			raw->gx, raw->gy, raw->gz, CF_AXIS_PITCH, &cfg, &angles);
}

static uint8_t fall_guard(void)
{
	if (0u != fallen)
	{
		if ((-posture.resume_cd < angles.pitch)
			&& (angles.pitch < posture.resume_cd))
		{
			fallen = 0u;
			return 0u;
		}

		return 1u;
	}
	else
	{
		if ((posture.stop_cd < angles.pitch)
			|| (-posture.stop_cd > angles.pitch))
		{
			fallen = 1u;
			return 1u;
		}

		return 0u;
	}
}

void control_init(void)
{
	int ret = mt_speed_init(board.enc_l, board.enc_r);

	if (0 != ret)
	{
		app_die();
	}

	velocity_init();
	imu_init();
}

void control_poll(void)
{
	uint16_t dt_10us;
	struct imu_raw_sample raw;
	int16_t out;
	int16_t vel_off;
	int16_t turn;

	//	采样到点返回 1 并输出真实 dt(10us); 未到点返回 0, 不修改 *dt_10us
	if (!imu_sample_due(&dt_10us))
		return;

	//	姿态更新
	attitude_update(&raw, dt_10us);

	//	跌倒保护与恢复判断
	if (fall_guard())
	{
		motors_write(0, 0);
		velocity_reset();
		return;
	}

	//	直立环+速度环 总开关
	if (0u == tune_run_enabled())
	{
		motors_write(0, 0);
		velocity_reset();
		return;
	}

	//	速度环开关
	if (0u != tune_vsw_enabled())
	{
		vel_off = velocity_poll();	//	速度环更新
	}
	else
	{
		vel_off = 0;
		velocity_reset();
	}

	//	直立环更新
	out = balance_pd_update(
			angles.pitch - (posture.mid_cd + vel_off), raw.gy,
			&tune_bal);

	{
		if (0 < out)
		{
			led_on(board.led1);
		}
		else if (0 > out)
		{
			led_off(board.led1);
		}
	}

	turn = turn_pd_update(raw.gz, tune_turn.target, &tune_turn);

	motors_write(out + turn, out - turn);

//	tune_feed(angles.pitch, out);
}
