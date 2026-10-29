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
#include "board.h"
#include "pal/flash.h"

volatile int g_step = 0;
volatile int g_result = 0;
static uint8_t wbuf[16];
static uint8_t rbuf[16];
static uint8_t ff[16];

static void fail(int step)
{
	if (0 == g_result)
	{
		g_result = -1;
		g_step = step;
	}
}

void flash_test(void)
{
	const struct flash *fl = board.flash1;
	uint8_t id[3];
	uint32_t sec_base = fl->caps->size - fl->caps->sector;
	int i;

	if (0 != flash_read_jedec_id(fl, id)
		|| fl->caps->id != (((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | id[2]))
	{
		fail(1);
	}

	if (0 == g_result && 0 != flash_erase(fl, sec_base, fl->caps->sector))
	{
		fail(2);
	}

	if (0 == g_result)
	{
		if (0 != flash_read(fl, sec_base + 0xF8, ff, 16))
		{
			fail(3);
		}
		else
		{
			for (i = 0; i < 16; i++)
			{
				if (0xFF != ff[i])
				{
					fail(3);
					break;
				}
			}
		}
	}

	if (0 == g_result)
	{
		for (i = 0; i < 16; i++)
		{
			wbuf[i] = (uint8_t)(i * 7 + 3);
		}

		if (0 != flash_write(fl, sec_base + 0xF8, wbuf, 16))
		{
			fail(4);
		}
	}

	if (0 == g_result)
	{
		if (0 != flash_read(fl, sec_base + 0xF8, rbuf, 16))
		{
			fail(5);
		}
		else
		{
			for (i = 0; i < 16; i++)
			{
				if (wbuf[i] != rbuf[i])
				{
					fail(5);
					break;
				}
			}
		}
	}

	if (0 == g_result && -EINVAL != flash_erase(fl, sec_base + 1, fl->caps->sector))
	{
		fail(6);
	}

	if (0 == g_result && -EINVAL != flash_read(fl, fl->caps->size - 4, rbuf, 8))
	{
		fail(7);
	}

	if (0 == g_result && -EINVAL != flash_write(fl, fl->caps->size - 4, wbuf, 8))
	{
		fail(7);
	}
}
