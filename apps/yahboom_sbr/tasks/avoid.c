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

#include "tick.h"
#include "tuning.h"
#include "tasks/sonar.h"
#include "control/move_dist.h"
#include "control/turn_angle.h"
#include "control/velocity.h"
#include "tasks/avoid.h"

///////////////////////////////////////////////////////////////////////////////
// 超声波避障任务: 全程非阻塞状态机
//
// 状态转换图:
//
//	IDLE:	-> CRUISE, vt=CRUISE_VT
//	CRUISE:	sonar OK 且 mm<=AVOID_TRIG_MM -> BRAKE, vt=0, 记录 state_tick
//	BRAKE:	延时 AVOID_BRAKE_WAIT_MS -> BACKUP, move_dist_measuring_start(-AVOID_BACKUP_MM)
//	BACKUP:	move_dist 状态:
//			RUN  -> 等待
//			DONE -> TURN, turn_angle_measuring_start(AVOID_TURN_CD)
//			其他 -> HALT (超时或中止)
//	TURN:	turn_angle 状态:
//			RUN  -> 等待
//			DONE -> CRUISE, vt=CRUISE_VT
//			其他 -> HALT
//	HALT:	死等 avrst 放行 -> IDLE

#define	AVOID_CRUISE_VT			2200
#define	AVOID_TRIG_MM			250u
#define	AVOID_BACKUP_MM			200
#define	AVOID_TURN_CD			13500
#define	AVOID_BRAKE_WAIT_MS		300u

enum avoid_st
{
	AVOID_IDLE,			//	待命态: avrst 放行后即走
	AVOID_CRUISE,		//	巡航态: 前进+测距
	AVOID_BRAKE,		//	刹车态: 纯延时
	AVOID_BACKUP,		//	倒退态: 定距后退
	AVOID_TURN,			//	转向态: 定角旋转
	AVOID_HALT,			//	停机态: 死等放行
};

static enum avoid_st	state = AVOID_HALT;
static uint32_t			state_tick;

void avoid_poll(void)
{
	uint16_t		mm;
	enum sonar_status	ss;
	uint32_t		ms;

	if (TUNE_MODE_SONAR != tune_mode_get())
	{
		state = AVOID_HALT;
		return;
	}

	switch (state)
	{
	case AVOID_IDLE:
		state = AVOID_CRUISE;
		velocity_set_target(AVOID_CRUISE_VT);
		break;

	case AVOID_CRUISE:
		sonar_get(&mm, &ss);

		if (SONAR_OK == ss && mm <= AVOID_TRIG_MM)
		{
			state_tick = tick_get();
			state = AVOID_BRAKE;
			velocity_set_target(0);
		}
		break;

	case AVOID_BRAKE:
		ms = tick_get() - state_tick;

		if (AVOID_BRAKE_WAIT_MS <= ms)
		{
			state = AVOID_BACKUP;
			move_dist_measuring_start(-AVOID_BACKUP_MM);
		}
		break;

	case AVOID_BACKUP:
		switch (move_dist_state_get())
		{
		case MOVE_DIST_RUN:
			break;
		case MOVE_DIST_DONE:
			state = AVOID_TURN;
			turn_angle_measuring_start(AVOID_TURN_CD);
			break;
		default:
			state = AVOID_HALT;
			break;
		}
		break;

	case AVOID_TURN:
		switch (turn_angle_state_get())
		{
		case TURN_ANGLE_RUN:
			break;
		case TURN_ANGLE_DONE:
			state = AVOID_CRUISE;
			velocity_set_target(AVOID_CRUISE_VT);
			break;
		default:
			state = AVOID_HALT;
			break;
		}
		break;

	case AVOID_HALT:
		break;
	}
}

uint8_t avoid_halt_resume(void)
{
	if (AVOID_HALT != state)
	{
		return 0;
	}

	state = AVOID_IDLE;
	return 1;
}
