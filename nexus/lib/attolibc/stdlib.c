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

#include <stddef.h>
#include <limits.h>

static int is_space(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

static int digit_val(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	if (c >= 'a' && c <= 'z')
	{
		return c - 'a' + 10;
	}
	if (c >= 'A' && c <= 'Z')
	{
		return c - 'A' + 10;
	}
	return -1;
}

static long strtox(const char *s, char **endptr, int base, int is_unsigned)
{
	const char *start = s;
	int neg = 0;

	while (is_space(*s))
	{
		s++;
	}
	if (*s == '+' || *s == '-')
	{
		neg = (*s == '-');
		s++;
	}

	if ((base == 0 || base == 16) && s[0] == '0' && (s[1] | 32) == 'x'
		&& digit_val(s[2]) >= 0 && digit_val(s[2]) < 16)
	{
		s += 2;
		base = 16;
	}
	else if (base == 0)
	{
		base = (*s == '0') ? 8 : 10;
	}
	if (base < 2 || base > 36)
	{
		base = 10;
	}

	{
		unsigned long acc = 0;
		unsigned long lim;
		int any = 0;
		int d;
		int over = 0;

		if (is_unsigned)
		{
			lim = ULONG_MAX;
		}
		else
		{
			lim = neg ? (unsigned long)LONG_MAX + 1u : (unsigned long)LONG_MAX;
		}

		while ((d = digit_val(*s)) >= 0 && d < base)
		{
			if (!over)
			{
				if (acc > (lim - (unsigned long)d) / (unsigned long)base)
				{
					acc = lim;
					over = 1;
				}
				else
				{
					acc = acc * (unsigned long)base + (unsigned long)d;
				}
			}
			s++;
			any = 1;
		}
		if (!any)
		{
			if (endptr)
			{
				*endptr = (char *)start;
			}
			return 0;
		}
		if (endptr)
		{
			*endptr = (char *)s;
		}
		return neg ? (long)(0ul - acc) : (long)acc;
	}
}

long strtol(const char *s, char **endptr, int base)
{
	return strtox(s, endptr, base, 0);
}

unsigned long strtoul(const char *s, char **endptr, int base)
{
	return (unsigned long)strtox(s, endptr, base, 1);
}

int atoi(const char *s)
{
	return (int)strtol(s, NULL, 10);
}
