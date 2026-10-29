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
#include "subsys/tty.h"
#include "lib/gadget/split.h"
#include "lib/gadget/fixtoa.h"
#include "control/algo/balance_pd.h"
#include "control/algo/velocity_pi.h"
#include "control/algo/turn_pd.h"
#include "control/velocity.h"
#include "tuning.h"

TUNE_RO struct balance_pd_config tune_bal = BALANCE_PD_CONFIG_INIT;
TUNE_RO struct velocity_pi_config tune_vel = VELOCITY_PI_CONFIG_INIT;
TUNE_RO struct turn_pd_config tune_turn = TURN_PD_CONFIG_INIT;

static struct tty *bt;

static int bt_puts(const char *s)
{
	return tty_write(bt, s, (unsigned int)strlen(s));
}

static char bt_line[64];

static uint8_t tune_run_en = 1;
static uint8_t tune_vsw_en = 1u;
static uint8_t tune_up_on;

static uint32_t up_next;
static int16_t last_pitch;
static int16_t last_out;

static int cmd_help(int argc, char *argv[]);
static int cmd_dump(int argc, char *argv[]);

static void bt_put_bal(const char *tag)
{
	char b[16];
	int32_t v;

	bt_puts(tag);

	v = ((int32_t)tune_bal.kp_num * 10000 + (1 << (tune_bal.kp_shr - 1))) >> tune_bal.kp_shr;
	fixtoa(b, sizeof(b), v, 4);
	bt_puts(" kp=");
	bt_puts(b);

	v = ((int32_t)tune_bal.kd_num * 10000 + (1 << (tune_bal.kd_shr - 1))) >> tune_bal.kd_shr;
	fixtoa(b, sizeof(b), v, 4);
	bt_puts(" kd=");
	bt_puts(b);

	bt_puts("\r\n");
}

static void bt_put_vel(const char *tag)
{
	char b[16];
	int32_t v;

	bt_puts(tag);

	v = ((int32_t)tune_vel.kp_num * 10000 + (1 << (tune_vel.kp_shr - 1))) >> tune_vel.kp_shr;
	fixtoa(b, sizeof(b), v, 4);
	bt_puts(" kp=");
	bt_puts(b);

	v = ((int32_t)tune_vel.ki_num * 10000 + (1 << (tune_vel.ki_shr - 1))) >> tune_vel.ki_shr;
	fixtoa(b, sizeof(b), v, 4);
	bt_puts(" ki=");
	bt_puts(b);

	bt_puts("\r\n");
}

static void bt_put_turn(const char *tag)
{
	char b[16];
	int32_t v;

	bt_puts(tag);

	v = ((int32_t)tune_turn.kd_num * 10000 + (1 << (tune_turn.kd_shr - 1))) >> tune_turn.kd_shr;
	fixtoa(b, sizeof(b), v, 4);
	bt_puts(" kd=");
	bt_puts(b);

	bt_puts("\r\n");
}

#ifndef TUNE_LOCK
static void put_ok(const char *tag, int32_t v)
{
	char b[16];

	bt_puts("OK ");
	bt_puts(tag);
	bt_puts("=");
	snprintf(b, sizeof(b), "%d", (int)v);
	bt_puts(b);
	bt_puts("\r\n");
}

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

static int cmd_set_num(int32_t v, int32_t lo, int32_t hi, int16_t *num,
		const char *tag)
{
	if (lo > v || hi < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	*num = (int16_t)v;
	put_ok(tag, v);

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

	return cmd_set_num(v, 0, 32767, &tune_bal.kp_num, "bkp");
}

static int cmd_bkd(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: bkd <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_bal.kd_num, "bkd");
}

static int cmd_vkp(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: vkp <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_vel.kp_num, "vkp");
}

static int cmd_vki(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: vki <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_vel.ki_num, "vki");
}

static int cmd_tkd(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: tkd <num>\r\n");
		return -EINVAL;
	}

	return cmd_set_num(v, 0, 32767, &tune_turn.kd_num, "tkd");
}

static int cmd_lpfa(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: lpfa <num>\r\n");
		return -EINVAL;
	}

	if (0 > v || 256 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_vel.lpf_a = (uint16_t)v;
	put_ok("lpfa", v);

	return 0;
}

static int cmd_omax(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: omax <num>\r\n");
		return -EINVAL;
	}

	if (1 > v || 1000 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_bal.out_max = (int16_t)v;
	put_ok("omax", v);

	return 0;
}

static int cmd_omin(int argc, char *argv[])
{
	int32_t v;

	if (2 != argc || 0 != parse_num(argv[1], &v))
	{
		bt_puts("ERR usage: omin <num> 0=off\r\n");
		return -EINVAL;
	}

	if (0 > v || 999 < v)
	{
		bt_puts("ERR range\r\n");
		return -EINVAL;
	}

	tune_bal.out_min = (int16_t)v;
	put_ok("omin", v);

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

	tune_up_on = (uint8_t)(argv[1][0] - '0');
	bt_puts(tune_up_on ? "OK up=1\r\n" : "OK up=0\r\n");

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

static int cmd_vt(int argc, char *argv[])
{
	char *end;
	char b[16];
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
	bt_puts("OK vt=");
	snprintf(b, sizeof(b), "%d", (int)v);
	bt_puts(b);
	bt_puts("\r\n");

	return 0;
}

static int cmd_dump(int argc, char *argv[])
{
	char b[16];

	(void)argc;
	(void)argv;
	bt_put_bal("bal");
	bt_put_vel("vel");
	bt_put_turn("turn");

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
	bt_puts(" lpf_a=");
	snprintf(b, sizeof(b), "%u", tune_vel.lpf_a);
	bt_puts(b);
	bt_puts(" off_max=");
	snprintf(b, sizeof(b), "%d", tune_vel.off_max);
	bt_puts(b);
	bt_puts("\r\n");

	bt_puts("turn kd_num=");
	snprintf(b, sizeof(b), "%d", tune_turn.kd_num);
	bt_puts(b);
	bt_puts(" kd_shr=");
	snprintf(b, sizeof(b), "%u", tune_turn.kd_shr);
	bt_puts(b);
	bt_puts(" target=");
	snprintf(b, sizeof(b), "%d", tune_turn.target);
	bt_puts(b);
	bt_puts(" out_max=");
	snprintf(b, sizeof(b), "%d", tune_turn.out_max);
	bt_puts(b);
	bt_puts("\r\n");

	return 0;
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
	snprintf(b, sizeof(b), "%u", tune_up_on);
	bt_puts(b);
	bt_puts(" vsw=");
	snprintf(b, sizeof(b), "%u", tune_vsw_en);
	bt_puts(b);
	bt_puts(" vt=");
	snprintf(b, sizeof(b), "%d", velocity_get_target());
	bt_puts(b);
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	bt_puts(" rdrop=");
	snprintf(b, sizeof(b), "%u", (unsigned)tty_rx_drop(bt));
	bt_puts(b);
	bt_puts(" tdrop=");
	snprintf(b, sizeof(b), "%u", (unsigned)tty_tx_drop(bt));
	bt_puts(b);
#endif
	bt_puts("\r\n");

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
	{ "vkp",	cmd_vkp,	"velocity kp <num>" },
	{ "vki",	cmd_vki,	"velocity ki <num>" },
	{ "tkd",	cmd_tkd,	"turn kd <num>" },
	{ "lpfa",	cmd_lpfa,	"velocity lpf a <num> 0..256" },
	{ "omax",	cmd_omax,	"balance out limit <num> 1..1000" },
	{ "omin",	cmd_omin,	"balance out mute <num> 0=off" },
#endif
	{ "stat",	cmd_stat,	"params + channel status" },
	{ "run",	cmd_run,	"enable motors" },
	{ "stop",	cmd_stop,	"motors off (default)" },
	{ "vsw",	cmd_vsw,	"velocity loop 0|1" },
	{ "vt",		cmd_vt,		"velocity target <cnt> -8000..8000" },
	{ "up",		cmd_up,		"telemetry 0|1" },
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

uint8_t tune_vsw_enabled(void)
{
	return tune_vsw_en;
}

void tune_feed(int16_t pitch_cd, int16_t out)
{
	char b[24];

	last_pitch = pitch_cd;
	last_out = out;

	if (0 == tune_up_on)
	{
		return;
	}

	if (0 == up_next)
	{
		up_next = tick_get() + 50u;
		return;
	}

	if (0 == tick_expired(up_next))
	{
		return;
	}

	up_next = tick_get() + 50u;
	snprintf(b, sizeof(b), "%d,%d\n", (int)last_pitch, (int)last_out);
	(void)tty_write(bt, b, (unsigned int)strlen(b));
}
