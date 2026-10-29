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
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "subsys/console.h"
#include "subsys/tty.h"
#include "die.h"

#define	LINE_SZ	64u

static uint32_t bench_target;
static uint32_t bench_next;

static void handle_line(struct tty *con, const char *l)
{
	if (0 == strcmp(l, "help"))
	{
		printf("stat / txbench N\r\n");
		return;
	}

	if (0 == strcmp(l, "stat"))
	{
		printf("drop=%u bench=%u\r\n",
				tty_rx_drop(con) + tty_tx_drop(con), bench_next);
		return;
	}

	if (0 == strncmp(l, "txbench ", 8u))
	{
		const char *a = &l[8];
		uint32_t v = 0;

		while (('0' <= *a) && ('9' >= *a))
		{
			v = (v * 10u) + (uint32_t)(*a - '0');
			a++;
		}

		if ((0u != v) && (999999u >= v))
		{
			bench_next = 0;
			bench_target = v;
		}
	}
}

int main(void)
{
	struct tty *con = board.console;
	char line[LINE_SZ];

	if (!con)
		die();

	tty_init(con);
	tty_lflag_set(con, TTY_LFLAG_ECHO);
	console_attach(con);

	printf("async up; help\r\n");

	for (;;)
	{
		if (0 == tty_getline(con, line, sizeof(line)))
		{
			handle_line(con, line);
		}

		if ((0u != bench_target) && (0 != tty_idle(con)))
		{
			printf("%06u\r\n", bench_next);
			bench_next++;

			if (bench_next >= bench_target)
			{
				bench_target = 0u;
			}
		}

		if ((0u == bench_target) && (0 != tty_idle(con)))
		{
			__asm volatile("wfi");
		}
	}
}
