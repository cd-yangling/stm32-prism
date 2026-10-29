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
#include "tick.h"
#include "board.h"
#include "app_die.h"
#include "pal/pin.h"
#include "pal/sensor.h"
#include "motor.h"
#include "tasks/battery.h"

///////////////////////////////////////////////////////////////////////////////
// 电池检测为非紧急任务: 全程非阻塞, 使用状态机编程, 禁止任何阻塞函数使用
//
// 状态转换图:
//
//	SUBMIT:	sensor_submit (返回值忽略) -> WAIT, +BAT_SMP_MS
//	WAIT:	sensor_poll 一次:
//			成功: sum_mv += sv.val1*1000 + sv.val2/1000; n_ok++
//			(无论成败) n_try++
//		n_try <  BAT_NSAMP -> SUBMIT (立即)
//		n_try == BAT_NSAMP -> 判定:
//			n_ok == 0            -> app_die()
//			sum_mv/n_ok < BAT_VBAT_LOW_MV -> BEEP_ON (立即, +BAT_BEEP_MS)
//			否则                 -> SLEEP (+BAT_SLEEP_MS)
//	SLEEP:	到点 -> 组清零 -> SUBMIT
//	BEEP_ON:	buzzer=1 -> BEEP_OFF, +BAT_BEEP_MS
//	BEEP_OFF:	buzzer=0 -> BEEP_ON,  +BAT_PAUSE_MS

#define	BAT_VBAT_LOW_MV		9600u
#define	BAT_NSAMP			8u
#define	BAT_SMP_MS			100u
#define	BAT_SLEEP_MS		10000u
#define	BAT_BEEP_MS			200u
#define	BAT_PAUSE_MS		800u

enum battery_st
{
	BAT_SLEEP,			//	检测态: 组间休眠
	BAT_SUBMIT,			//	检测态: 发起转换
	BAT_WAIT,			//	检测态: 等待并收取单样本
	BAT_BEEP_ON,		//	低电量态: 响
	BAT_BEEP_OFF,		//	低电量态: 停
};

static enum battery_st st;
static uint32_t deadline;
static uint32_t sum_mv;
static uint8_t n_try;
static uint8_t n_ok;

static void group_clear(void)
{
	sum_mv = 0u;
	n_try = 0u;
	n_ok = 0u;
}

void battery_init(void)
{
	st = BAT_SUBMIT;
	deadline = tick_get();
	group_clear();
}

void battery_poll(void)
{
	struct sensor_value sv;

	if (!tick_expired(deadline))
	{
		return;
	}

	switch (st)
	{
	case BAT_SUBMIT:
		(void)sensor_submit(board.vbat_sen);
		st = BAT_WAIT;
		deadline = tick_get() + BAT_SMP_MS;
		break;

	case BAT_WAIT:
		n_try++;

		if (0 == sensor_poll(board.vbat_sen)
			&& 0 == sensor_channel_get(board.vbat_sen, SENSOR_CHAN_VOLTAGE, &sv))
		{
			sum_mv += (uint32_t)sv.val1 * 1000u + (uint32_t)(sv.val2 / 1000);
			n_ok++;
		}

		if (BAT_NSAMP > n_try)
		{
			st = BAT_SUBMIT;
			deadline = tick_get();
		}
		else if (0u == n_ok)
		{
			app_die();
		}
		else if (sum_mv / n_ok < BAT_VBAT_LOW_MV)
		{
			motor_output_enable(0u);
			st = BAT_BEEP_ON;
			deadline = tick_get();
		}
		else
		{
			motor_output_enable(1u);
			st = BAT_SLEEP;
			deadline = tick_get() + BAT_SLEEP_MS;
		}
		break;

	case BAT_SLEEP:
		group_clear();
		st = BAT_SUBMIT;
		deadline = tick_get();
		break;

	case BAT_BEEP_ON:
		pin_set(board.buzzer, 1);
		st = BAT_BEEP_OFF;
		deadline = tick_get() + BAT_BEEP_MS;
		break;

	case BAT_BEEP_OFF:
		pin_set(board.buzzer, 0);
		st = BAT_BEEP_ON;
		deadline = tick_get() + BAT_PAUSE_MS;
		break;
	}
}
