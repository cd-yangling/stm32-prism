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

#include "fixtoa.h"
#include <errno.h>

static const uint32_t pow10_tab[7] =
{
	1u, 10u, 100u, 1000u, 10000u, 100000u, 1000000u
};

static int udigits(uint32_t v)
{
	int n = 1;

	while (v >= 10u)
	{
		v /= 10u;
		n++;
	}
	return n;
}

static char *put_u32(char *p, uint32_t v, int minwidth)
{
	char tmp[10];
	int n = 0;

	do
	{
		tmp[n++] = (char)('0' + v % 10u);
		v /= 10u;
	} while (0 != v);

	while (n < minwidth)
	{
		tmp[n++] = '0';
	}
	while (0 != n)
	{
		*p++ = tmp[--n];
	}
	return p;
}

int fixtoa(char *out, size_t outsz, int32_t value, int decimals)
{
	uint32_t mag;
	uint32_t ipart;
	uint32_t fpart;
	size_t need;
	char *p;

	if (0 > decimals || 6 < decimals)
	{
		if (0 != outsz)
		{
			out[0] = '\0';
		}
		return -EINVAL;
	}

	/* INT32_MIN 直接取负会溢出, 无符号减法绕开 */
	mag = (0 > value) ? (uint32_t)0u - (uint32_t)value : (uint32_t)value;
	ipart = mag / pow10_tab[decimals];
	fpart = mag % pow10_tab[decimals];

	need = (size_t)udigits(ipart) + 1u;
	if (0 > value)
	{
		need++;
	}
	if (0 != decimals)
	{
		need += (size_t)decimals + 1u;
	}

	if (need > outsz)
	{
		if (0 != outsz)
		{
			out[0] = '\0';
		}
		return -ENOSPC;
	}

	p = out;
	if (0 > value)
	{
		*p++ = '-';
	}
	p = put_u32(p, ipart, 1);
	if (0 != decimals)
	{
		*p++ = '.';
		p = put_u32(p, fpart, decimals);
	}
	*p = '\0';
	return 0;
}
