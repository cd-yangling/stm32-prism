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
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tick.h"
#include "board.h"
#include "motor.h"
#include "pal/led.h"
#include "pal/pin.h"
#include "subsys/tty.h"
#include "lib/gadget/split.h"
#include "lib/gadget/fixtoa.h"
#include "control/algo/balance_pd.h"
#include "control/algo/pid_ctrl.h"
#include "control/velocity.h"
#include "control/turn.h"
#include "control/turn_angle.h"
#include "control/move_dist.h"
#include "telemetry.h"
#include "tasks/avoid.h"
#include "tasks/battery.h"
#include "tuning.h"

#define	BALANCE_PD_CONFIG_INIT		\
{									\
	.kp_num		= 220,				\
	.kp_shr		= 10,				\
	.kd_num		= 2800,				\
	.kd_shr		= 16,				\
	.out_max	= 1000,				\
	.out_min	= 35,				\
}

#define	VELOCITY_PID_CONFIG_INIT		\
{									\
	.kp_num		= 1500,				\
	.kp_shr		= 12,				\
	.ki_num		= 4,				\
	.ki_shr		= 13,				\
	.kd_num		= 1200,				\
	.kd_shr		= 10,				\
	.lpf_a		= 256,				\
	.out_max	= 3500,				\
}

#define	TURN_PID_CONFIG_INIT		\
{									\
	.kp_num		= 1600,				\
	.kp_shr		= 12,				\
	.ki_num		= 16,				\
	.ki_shr		= 13,				\
	.kd_num		= 0,				\
	.kd_shr		= 10,				\
	.lpf_a		= 256,				\
	.out_max	= 1000,				\
}

#define	TURN_ANGLE_CONFIG_INIT		\
{									\
	.kp_num		= 5500,				\
	.kp_shr		= 13,				\
	.out_max	= 8000,				\
	.timeout_ms	= 4000,				\
}

#define	MOVE_DIST_CONFIG_INIT		\
{									\
	.kp_num		= 150,				\
	.kp_shr		= 6,				\
	.out_max	= 600,				\
	.timeout_ms	= 10000,			\
}

TUNE_RO struct balance_pd_config tune_bal = BALANCE_PD_CONFIG_INIT;
TUNE_RO struct pid_ctrl_config tune_vel = VELOCITY_PID_CONFIG_INIT;
TUNE_RO struct pid_ctrl_config tune_turn = TURN_PID_CONFIG_INIT;
TUNE_RO struct turn_angle_config tune_tang = TURN_ANGLE_CONFIG_INIT;
TUNE_RO struct move_dist_config tune_move = MOVE_DIST_CONFIG_INIT;

static struct tty *bt;

static int bt_puts(const char *s)
{
	return tty_write(bt, s, (unsigned int)strlen(s));
}

static char bt_line[64];

static uint8_t tune_run_en = 1;
static uint8_t tune_vsw_en = 1u;
static uint8_t tune_tsw_en = 1u;
static uint8_t tune_mode;
static uint8_t tune_mode_locked;

static int cmd_help(int argc, char *argv[]);
static int cmd_dump(int argc, char *argv[]);

#ifndef TUNE_LOCK
static int parse_num(const char *s, int32_t *out)
{
	char *end;
	long v;

	v = strtol(s, &end, 10);

	if (end == s || '\0' != *end)
	{
		return -EINVAL;
	}

	*out = (int32_t)v;

	return 0;
}

static int cmd_set_num(int32_t v, int32_t lo, int32_t hi, int16_t *num)
{
	if (lo > v || hi < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	*num = (int16_t)v;

	return 0;
}

static int cmd_bkp(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: bkp <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_bal.kp_num);
}

static int cmd_bkd(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: bkd <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_bal.kd_num);
}

static int cmd_vkp(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: vkp <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_vel.kp_num);
}

static int cmd_vki(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: vki <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_vel.ki_num);
}

static int cmd_vkd(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: vkd <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_vel.kd_num);
}

static int cmd_tkp(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: tkp <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_turn.kp_num);
}

static int cmd_tki(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: tki <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_turn.ki_num);
}

static int cmd_tkd(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: tkd <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_turn.kd_num);
}

static int cmd_vlpfa(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: vlpfa <num>\r\n");
		return -EINVAL;
	}

	if (0 > v || 256 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_vel.lpf_a = (uint16_t)v;
	return 0;
}

static int cmd_tlpfa(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: tlpfa <num>\r\n");
		return -EINVAL;
	}

	if (0 > v || 256 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_turn.lpf_a = (uint16_t)v;
	return 0;
}

static int cmd_bomax(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: bomax <num>\r\n");
		return -EINVAL;
	}

	if (1 > v || 1000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_bal.out_max = (int16_t)v;
	return 0;
}

static int cmd_bomin(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: bomin <num> 0=off\r\n");
		return -EINVAL;
	}

	if (0 > v || 999 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_bal.out_min = (int16_t)v;
	return 0;
}

static int cmd_vomax(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: vomax <num>\r\n");
		return -EINVAL;
	}

	if (1 > v || 4500 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_vel.out_max = (int16_t)v;
	return 0;
}

static int cmd_tomax(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: tomax <num>\r\n");
		return -EINVAL;
	}

	if (1 > v || 1000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_turn.out_max = (int16_t)v;
	return 0;
}

static int cmd_takp(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: takp <num> -32768..32767\r\n");
		return -EINVAL;
	}

	if (-32768 > v || 32767 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_tang.kp_num = (int16_t)v;
	return 0;
}

static int cmd_taomax(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: taomax <num> 1..8000\r\n");
		return -EINVAL;
	}

	if (1 > v || 8000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_tang.out_max = (int16_t)v;
	return 0;
}

static int cmd_tatmout(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: tatmout <ms> 500..60000\r\n");
		return -EINVAL;
	}

	if (500 > v || 60000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_tang.timeout_ms = (uint16_t)v;
	return 0;
}

static int cmd_mdkp(int argc, char *argv[])
{
	int32_t	v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: mdkp <num> -32768..32767\r\n");
		return -EINVAL;
	}

	if (-32768 > v || 32767 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_move.kp_num = (int16_t)v;
	return 0;
}

static int cmd_mdomax(int argc, char *argv[])
{
	int32_t	v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: mdomax <num> 1..8000\r\n");
		return -EINVAL;
	}

	if (1 > v || 8000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_move.out_max = (int16_t)v;
	return 0;
}

static int cmd_mdtmout(int argc, char *argv[])
{
	int32_t	v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: mdtmout <ms> 500..60000\r\n");
		return -EINVAL;
	}

	if (500 > v || 60000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_move.timeout_ms = (uint16_t)v;
	return 0;
}
#endif

static int cmd_run(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	tune_run_en = 1u;
	led_on(board.led1);
	bt_puts("OK run=1\r\n");

	return 0;
}

static int cmd_stop(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	tune_run_en = 0u;
	led_off(board.led1);
	turn_angle_measuring_abort();
	move_dist_measuring_abort();
	motor_set(board.motor_l, 0);
	motor_set(board.motor_r, 0);
	bt_puts("OK run=0\r\n");

	return 0;
}

static int cmd_up(int argc, char *argv[])
{
	if (2 != argc || ('0' != argv[1][0] && '1' != argv[1][0])
			|| '\0' != argv[1][1])
	{
		bt_puts("ERR usage: up <0|1>\r\n");
		return -EINVAL;
	}

	telemetry_set_up((uint8_t)(argv[1][0] - '0'));
	bt_puts(telemetry_get_up() ? "OK up=1\r\n" : "OK up=0\r\n");

	return 0;
}

static int cmd_tmask(int argc, char *argv[])
{
	char	*end;
	char	b[20];
	unsigned long	v;

	if (2 != argc)
	{
		bt_puts("ERR usage: tmask <hex>\r\n");
		return -EINVAL;
	}

	v = strtoul(argv[1], &end, 16);

	if (end == argv[1] || '\0' != *end)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	if (0 != telemetry_set_mask((uint32_t)v))
	{
		bt_puts("ERR tmask\r\n");
		return -EINVAL;
	}

	snprintf(b, sizeof(b), "OK tmask=0x%x\r\n",
			(unsigned)telemetry_get_mask());
	bt_puts(b);

	return 0;
}

static int cmd_vsw(int argc, char *argv[])
{
	if (2 != argc || ('0' != argv[1][0] && '1' != argv[1][0])
			|| '\0' != argv[1][1])
	{
		bt_puts("ERR usage: vsw <0|1>\r\n");
		return -EINVAL;
	}

	tune_vsw_en = (uint8_t)(argv[1][0] - '0');
	bt_puts(tune_vsw_en ? "OK vsw=1\r\n" : "OK vsw=0\r\n");

	return 0;
}

static int cmd_tsw(int argc, char *argv[])
{
	if (2 != argc || ('0' != argv[1][0] && '1' != argv[1][0])
			|| '\0' != argv[1][1])
	{
		bt_puts("ERR usage: tsw <0|1>\r\n");
		return -EINVAL;
	}

	tune_tsw_en = (uint8_t)(argv[1][0] - '0');
	bt_puts(tune_tsw_en ? "OK tsw=1\r\n" : "OK tsw=0\r\n");

	return 0;
}

static int cmd_vt(int argc, char *argv[])
{
	char *end;
	long v;

	if (2 != argc)
	{
		bt_puts("ERR usage: vt <cnt> -8000..8000\r\n");
		return -EINVAL;
	}

	v = strtol(argv[1], &end, 10);

	if (end == argv[1] || '\0' != *end || -8000 > v || 8000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	velocity_set_target((int16_t)v);

	return 0;
}

static int cmd_tt(int argc, char *argv[])
{
	char *end;
	long v;

	if (2 != argc)
	{
		bt_puts("ERR usage: tt <cnt> -8000..8000\r\n");
		return -EINVAL;
	}

	v = strtol(argv[1], &end, 10);

	if (end == argv[1] || '\0' != *end || -8000 > v || 8000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	turn_set_target((int16_t)v);

	return 0;
}

static int cmd_tams(int argc, char *argv[])
{
	char	*end;
	long	v;

	if (2 != argc)
	{
		bt_puts("ERR usage: tams <cd> -18000..18000\r\n");
		return -EINVAL;
	}

	v = strtol(argv[1], &end, 10);

	if (end == argv[1] || '\0' != *end || -18000 > v || 18000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	if (0u == tune_run_en)
	{
		bt_puts("ERR run=0\r\n");
		return -EACCES;
	}

	if (0 != turn_angle_measuring_start(v))
	{
		bt_puts("ERR busy\r\n");
		return -EBUSY;
	}

	return 0;
}

static int cmd_tama(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	turn_angle_measuring_abort();

	return 0;
}

static int cmd_tast(int argc, char *argv[])
{
	static const char *names[] =
	{
		"IDLE", "RUN", "DONE", "ETIMEOUT", "ABORTED",
	};

	char	b[16];
	int		st;

	(void)argc;
	(void)argv;
	st = turn_angle_state_get();
	bt_puts("ta=");
	bt_puts(names[st]);
	bt_puts(" yaw=");
	snprintf(b, sizeof(b), "%d", (int)turn_angle_yaw_get());
	bt_puts(b);
	bt_puts(" t=");
	snprintf(b, sizeof(b), "%ums",
			(unsigned)turn_angle_measuring_elapsed_ms());
	bt_puts(b);
	bt_puts("\r\n");

	return 0;
}

static int cmd_mdms(int argc, char *argv[])
{
	char	*end;
	long	v;

	if (2 != argc)
	{
		bt_puts("ERR usage: mdms <mm> -2000..2000\r\n");
		return -EINVAL;
	}

	v = strtol(argv[1], &end, 10);

	if (end == argv[1] || '\0' != *end || -2000 > v || 2000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	if (0u == tune_run_en)
	{
		bt_puts("ERR run=0\r\n");
		return -EACCES;
	}

	if (0 != move_dist_measuring_start(v))
	{
		bt_puts("ERR busy\r\n");
		return -EBUSY;
	}

	return 0;
}

static int cmd_mdma(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	move_dist_measuring_abort();

	return 0;
}

static int cmd_mdst(int argc, char *argv[])
{
	static const char *names[] =
	{
		"IDLE", "RUN", "DONE", "ETIMEOUT", "ABORTED",
	};

	char	b[16];
	int		st;

	(void)argc;
	(void)argv;
	st = move_dist_state_get();
	bt_puts("md=");
	bt_puts(names[st]);
	bt_puts(" dist=");
	snprintf(b, sizeof(b), "%d", (int)move_dist_dist_get());
	bt_puts(b);
	bt_puts(" t=");
	snprintf(b, sizeof(b), "%ums",
			(unsigned)move_dist_measuring_elapsed_ms());
	bt_puts(b);
	bt_puts("\r\n");

	return 0;
}

static int cmd_avrst(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	if (!avoid_halt_resume())
	{
		bt_puts("ERR not halted\r\n");
		return -EPERM;
	}

	bt_puts("OK\r\n");

	return 0;
}

static int cmd_dump(int argc, char *argv[])
{
	char b[16];

	(void)argc;
	(void)argv;
	bt_puts("bal kp_num=");
	snprintf(b, sizeof(b), "%d", tune_bal.kp_num);
	bt_puts(b);
	bt_puts(" kp_shr=");
	snprintf(b, sizeof(b), "%u", tune_bal.kp_shr);
	bt_puts(b);
	bt_puts(" kd_num=");
	snprintf(b, sizeof(b), "%d", tune_bal.kd_num);
	bt_puts(b);
	bt_puts(" kd_shr=");
	snprintf(b, sizeof(b), "%u", tune_bal.kd_shr);
	bt_puts(b);
	bt_puts(" out_max=");
	snprintf(b, sizeof(b), "%d", tune_bal.out_max);
	bt_puts(b);
	bt_puts(" out_min=");
	snprintf(b, sizeof(b), "%d", tune_bal.out_min);
	bt_puts(b);
	bt_puts("\r\n");

	bt_puts("vel kp_num=");
	snprintf(b, sizeof(b), "%d", tune_vel.kp_num);
	bt_puts(b);
	bt_puts(" kp_shr=");
	snprintf(b, sizeof(b), "%u", tune_vel.kp_shr);
	bt_puts(b);
	bt_puts(" ki_num=");
	snprintf(b, sizeof(b), "%d", tune_vel.ki_num);
	bt_puts(b);
	bt_puts(" ki_shr=");
	snprintf(b, sizeof(b), "%u", tune_vel.ki_shr);
	bt_puts(b);
	bt_puts(" kd_num=");
	snprintf(b, sizeof(b), "%d", tune_vel.kd_num);
	bt_puts(b);
	bt_puts(" kd_shr=");
	snprintf(b, sizeof(b), "%u", tune_vel.kd_shr);
	bt_puts(b);
	bt_puts(" lpf_a=");
	snprintf(b, sizeof(b), "%u", tune_vel.lpf_a);
	bt_puts(b);
	bt_puts(" out_max=");
	snprintf(b, sizeof(b), "%d", tune_vel.out_max);
	bt_puts(b);
	bt_puts("\r\n");

	bt_puts("turn kp_num=");
	snprintf(b, sizeof(b), "%d", tune_turn.kp_num);
	bt_puts(b);
	bt_puts(" kp_shr=");
	snprintf(b, sizeof(b), "%u", tune_turn.kp_shr);
	bt_puts(b);
	bt_puts(" ki_num=");
	snprintf(b, sizeof(b), "%d", tune_turn.ki_num);
	bt_puts(b);
	bt_puts(" ki_shr=");
	snprintf(b, sizeof(b), "%u", tune_turn.ki_shr);
	bt_puts(b);
	bt_puts(" kd_num=");
	snprintf(b, sizeof(b), "%d", tune_turn.kd_num);
	bt_puts(b);
	bt_puts(" kd_shr=");
	snprintf(b, sizeof(b), "%u", tune_turn.kd_shr);
	bt_puts(b);
	bt_puts(" lpf_a=");
	snprintf(b, sizeof(b), "%u", tune_turn.lpf_a);
	bt_puts(b);
	bt_puts(" out_max=");
	snprintf(b, sizeof(b), "%d", tune_turn.out_max);
	bt_puts(b);
	bt_puts("\r\n");

	bt_puts("turn angle kp_num=");
	snprintf(b, sizeof(b), "%d", tune_tang.kp_num);
	bt_puts(b);
	bt_puts(" kp_shr=");
	snprintf(b, sizeof(b), "%u", tune_tang.kp_shr);
	bt_puts(b);
	bt_puts(" out_max=");
	snprintf(b, sizeof(b), "%d", tune_tang.out_max);
	bt_puts(b);
	bt_puts(" timeout_ms=");
	snprintf(b, sizeof(b), "%u", tune_tang.timeout_ms);
	bt_puts(b);
	bt_puts("\r\n");

	bt_puts("move dist kp_num=");
	snprintf(b, sizeof(b), "%d", tune_move.kp_num);
	bt_puts(b);
	bt_puts(" kp_shr=");
	snprintf(b, sizeof(b), "%u", tune_move.kp_shr);
	bt_puts(b);
	bt_puts(" out_max=");
	snprintf(b, sizeof(b), "%d", tune_move.out_max);
	bt_puts(b);
	bt_puts(" timeout_ms=");
	snprintf(b, sizeof(b), "%u", tune_move.timeout_ms);
	bt_puts(b);
	bt_puts("\r\n");

	return 0;
}

static int cmd_mode(int argc, char *argv[])
{
	char b[16];

	if (2 == argc)
	{
		char	*end;
		long	v;

		v = strtol(argv[1], &end, 10);

		if (end == argv[1] || '\0' != *end
			|| TUNE_MODE_REMOTE > v || TUNE_MODE_SONAR < v)
		{
			bt_puts("ERR range\r\n");
			return -EINVAL;
		}

		if (tune_mode_locked)
		{
			bt_puts("ERR locked\r\n");
			return -EPERM;
		}

		tune_mode = (uint8_t)v;
		pin_set(board.buzzer, 1);
		delay_ms(100u);
		pin_set(board.buzzer, 0);
	}
	else if (2 < argc)
	{
		bt_puts("ERR usage: mode <0|1>, no arg = query\r\n");
		return -EINVAL;
	}

	bt_puts("OK mode=");
	snprintf(b, sizeof(b), "%u", tune_mode);
	bt_puts(b);
	bt_puts(" lock=");
	snprintf(b, sizeof(b), "%u", tune_mode_locked);
	bt_puts(b);
	bt_puts("\r\n");

	return 0;
}

uint8_t tune_mode_get(void)
{
	return tune_mode;
}

void tuning_mode_lock(void)
{
	tune_mode_locked = 1u;
}

static int cmd_stat(int argc, char *argv[])
{
	char b[16];

	(void)argc;
	(void)argv;
	(void)cmd_dump(argc, argv);
	bt_puts("run=");
	snprintf(b, sizeof(b), "%u", tune_run_en);
	bt_puts(b);
	bt_puts(" up=");
	snprintf(b, sizeof(b), "%u", telemetry_get_up());
	bt_puts(b);
	bt_puts(" vsw=");
	snprintf(b, sizeof(b), "%u", tune_vsw_en);
	bt_puts(b);
	bt_puts(" vt=");
	snprintf(b, sizeof(b), "%d", velocity_get_target());
	bt_puts(b);
	bt_puts(" tsw=");
	snprintf(b, sizeof(b), "%u", tune_tsw_en);
	bt_puts(b);
	bt_puts(" ts=");
	snprintf(b, sizeof(b), "%d", turn_get_target());
	bt_puts(b);
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	bt_puts(" rdrop=");
	snprintf(b, sizeof(b), "%u", (unsigned)tty_rx_drop(bt));
	bt_puts(b);
	bt_puts(" tdrop=");
	snprintf(b, sizeof(b), "%u", (unsigned)tty_tx_drop(bt));
	bt_puts(b);
#endif
	bt_puts(" mask=0x");
	snprintf(b, sizeof(b), "%x", (unsigned)telemetry_get_mask());
	bt_puts(b);
	bt_puts("\r\n");

	return 0;
}

static int cmd_vbat(int argc, char *argv[])
{
	char b[16];

	(void)argc;
	(void)argv;
	fixtoa(b, sizeof(b), (int32_t)battery_get_mv(), 3);
	bt_puts("vbat=");
	bt_puts(b);
	bt_puts(" V\r\n");

	return 0;
}

struct tune_cmd
{
	const char	*name;
	int			(*fn)(int argc, char *argv[]);
	const char	*help;
};

static const struct tune_cmd tune_cmds[] =
{
	{ "help",	cmd_help,	"list commands" },
#ifndef TUNE_LOCK
	{ "bkp",	cmd_bkp,	"balance kp <num>" },
	{ "bkd",	cmd_bkd,	"balance kd <num>" },
	{ "bomax",	cmd_bomax,	"balance out limit <num> 1..1000" },
	{ "bomin",	cmd_bomin,	"balance out mute <num> 0=off" },
	{ "vkp",	cmd_vkp,	"velocity kp <num>" },
	{ "vki",	cmd_vki,	"velocity ki <num>" },
	{ "vkd",	cmd_vkd,	"velocity kd <num>" },
	{ "vlpfa",	cmd_vlpfa,	"velocity lpf a <num> 0..256" },
	{ "vomax",	cmd_vomax,	"velocity out limit <num> 1..4500" },
	{ "tkp",	cmd_tkp,	"turn kp <num>" },
	{ "tki",	cmd_tki,	"turn ki <num>" },
	{ "tkd",	cmd_tkd,	"turn kd <num>" },
	{ "tlpfa",	cmd_tlpfa,	"turn lpf a <num> 0..256" },
	{ "tomax",	cmd_tomax,	"turn out limit <num> 1..1000" },
	{ "takp",	cmd_takp,	"turn angle kp <num> -32768..32767" },
	{ "taomax",	cmd_taomax,	"turn angle out limit <num> 1..8000" },
	{ "tatmout",	cmd_tatmout,	"turn angle timeout <ms> 500..60000" },
	{ "mdkp",	cmd_mdkp,	"move dist kp <num> -32768..32767" },
	{ "mdomax",	cmd_mdomax,	"move dist out limit <num> 1..8000" },
	{ "mdtmout",	cmd_mdtmout,	"move dist timeout <ms> 500..60000" },
#endif
	{ "stat",	cmd_stat,	"params + channel status" },
	{ "vbat",	cmd_vbat,	"battery voltage" },
	{ "run",	cmd_run,	"enable motors" },
	{ "stop",	cmd_stop,	"motors off (default)" },
	{ "mode",	cmd_mode,	"run mode <0|1>, no arg = query" },
	{ "vsw",	cmd_vsw,	"velocity loop 0|1" },
	{ "vt",		cmd_vt,		"velocity target <cnt> -8000..8000" },
	{ "tsw",	cmd_tsw,	"turn loop 0|1" },
	{ "tt",		cmd_tt,		"turn target <cnt> -8000..8000" },
	{ "tams",	cmd_tams,	"turn angle measuring start <cd> -18000..18000" },
	{ "tama",	cmd_tama,	"turn angle measuring abort" },
	{ "tast",	cmd_tast,	"turn angle state" },
	{ "mdms",	cmd_mdms,	"move dist measuring start <mm> -2000..2000" },
	{ "mdma",	cmd_mdma,	"move dist measuring abort" },
	{ "mdst",	cmd_mdst,	"move dist state" },
	{ "avrst",	cmd_avrst,	"avoid halt resume + state" },
	{ "up",		cmd_up,		"telemetry 0|1" },
	{ "tmask",	cmd_tmask,	"telemetry channel mask <hex> 4 bits max" },
};

static int cmd_help(int argc, char *argv[])
{
	unsigned int i;

	(void)argc;
	(void)argv;

	for (i = 0; i < (sizeof(tune_cmds) / sizeof(tune_cmds[0])); i++)
	{
		bt_puts(tune_cmds[i].name);
		bt_puts(" - ");
		bt_puts(tune_cmds[i].help);
		bt_puts("\r\n");
	}

	return 0;
}

void tuning_init(struct tty *t)
{
	bt = t;
	telemetry_init(t);
	tty_lflag_clr(t, TTY_LFLAG_ECHO);
	bt_puts("tuning ready\r\n");
}

void tuning_poll(void)
{
	char *argv[8];
	int argc;
	unsigned int i;

	if (0 != tty_getline(bt, bt_line, sizeof(bt_line)))
	{
		return;
	}

	argc = split_argv(bt_line, argv, 8);

	if (1 > argc)
	{
		return;
	}

	for (i = 0; i < (sizeof(tune_cmds) / sizeof(tune_cmds[0])); i++)
	{
		if (0 == strcmp(tune_cmds[i].name, argv[0]))
		{
			if (NULL == tune_cmds[i].fn)
			{
				bt_puts("ERR reserved\r\n");
			}
			else
			{
				(void)tune_cmds[i].fn(argc, argv);
			}

			return;
		}
	}

	bt_puts("ERR unknown cmd\r\n");
}

uint8_t tune_run_enabled(void)
{
	return tune_run_en;
}

uint8_t tune_tsw_enabled(void)
{
	return tune_tsw_en;
}

uint8_t tune_vsw_enabled(void)
{
	return tune_vsw_en;
}
