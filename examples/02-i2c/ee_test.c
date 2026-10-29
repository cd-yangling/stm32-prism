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
#include "pal/eeprom.h"

volatile int g_ee_step = 0;
volatile int g_ee_result = 0;

static void fail(int step)
{
	if (0 == g_ee_result)
	{
		g_ee_result = -1;
		g_ee_step = step;
	}
}

void ee_test(void)
{
	const struct eeprom *ee = board.ee1;
	uint8_t wr[16];
	uint8_t rd[16];
	int i;

	for (i = 0; i < 16; i++)
	{
		wr[i] = (uint8_t)(i * 7 + 3);
	}

	if (0 > eeprom_write(ee, 205, wr, 16))
	{
		fail(1);
	}

	if (0 == g_ee_result && 0 > eeprom_read(ee, 205, rd, 16))
	{
		fail(2);
	}

	for (i = 0; 0 == g_ee_result && i < 16; i++)
	{
		if (wr[i] != rd[i])
		{
			fail(3);
		}
	}

	if (0 == g_ee_result && -EINVAL != eeprom_write(ee, 250, wr, 10))
	{
		fail(4);
	}

	if (0 == g_ee_result && -EINVAL != eeprom_read(ee, 250, rd, 10))
	{
		fail(5);
	}
}
