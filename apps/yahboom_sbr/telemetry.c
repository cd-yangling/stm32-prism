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
#include <string.h>
#include "tick.h"
#include "subsys/tty.h"
#include "telemetry.h"

#ifndef TUNE_WIRED
static const uint8_t	tele_period_ms[5] = { 20u, 8u, 12u, 16u, 20u };
#endif

static struct tty	*bt;
static int16_t		snap[TELE_CH_N];
static uint32_t	tele_mask;
static uint8_t		tele_up;
#ifndef TUNE_WIRED
static uint32_t	up_next;
#endif
static uint8_t		active_ch[4];
static uint8_t		active_n;

void telemetry_init(struct tty *t)
{
	bt = t;
}

void telemetry_feed(uint8_t ch, int16_t val)
{
	if (TELE_CH_N > ch)
	{
		snap[ch] = val;
	}
}

int telemetry_set_mask(uint32_t mask)
{
	uint8_t	n;
	uint8_t	k;
	int		i;

	n = (uint8_t)__builtin_popcount(mask & ((1u << TELE_CH_N) - 1u));

	if (4 < n)
	{
		return -EINVAL;
	}

	tele_mask = mask;
	active_n = n;
	k = 0u;

	for (i = 0; TELE_CH_N > i; i++)
	{
		if (0 != (mask & (1u << i)))
		{
			active_ch[k] = (uint8_t)i;
			k++;
		}
	}

#ifndef TUNE_WIRED
	up_next = 0u;
#endif

	return 0;
}

uint32_t telemetry_get_mask(void)
{
	return tele_mask;
}

void telemetry_set_up(uint8_t on)
{
	tele_up = on;
}

uint8_t telemetry_get_up(void)
{
	return tele_up;
}

void telemetry_reset(void)
{
	int	ch;

	for (ch = 0; TELE_CH_N > ch; ch++)
	{
		if (TELE_CH_PITCH != ch && TELE_CH_GY != ch)
		{
			snap[ch] = 0;
		}
	}
}

void telemetry_poll(void)
{
	char	b[10];
	uint8_t	i;

	if (0u == tele_up || 0u == active_n)
	{
		return;
	}

#ifndef TUNE_WIRED
	if (0u == up_next)
	{
		up_next = tick_get() + tele_period_ms[active_n];
		return;
	}

	if (0 == tick_expired(up_next))
	{
		return;
	}

	up_next = tick_get() + tele_period_ms[active_n];
#endif

	b[0] = (char)0xA5;

	for (i = 0u; active_n > i; i++)
	{
		(void)memcpy(&b[1u + 2u * i], &snap[active_ch[i]], 2);
	}

	b[1u + 2u * active_n] = (char)0xCC;
	(void)tty_write(bt, b, (unsigned int)(2u + 2u * active_n));
}
