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

#include "string.h"

size_t strlen(const char *s)
{
	const char *p = s;

	while (*p)
	{
		p++;
	}
	return (size_t)(p - s);
}

int strcmp(const char *a, const char *b)
{
	while (*a && *a == *b)
	{
		a++;
		b++;
	}
	return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
	while (n && *a && *a == *b)
	{
		a++;
		b++;
		n--;
	}
	if (0 == n)
	{
		return 0;
	}
	return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

char *strcpy(char *dst, const char *src)
{
	char *d = dst;

	while ((*d++ = *src++))
	{
	}
	return dst;
}

char *strcat(char *dst, const char *src)
{
	char *d = dst;

	while (*d)
	{
		d++;
	}
	while ((*d++ = *src++))
	{
	}
	return dst;
}

char *strchr(const char *s, int c)
{
	char cc = (char)c;

	while (*s != cc)
	{
		if ('\0' == *s)
		{
			return NULL;
		}
		s++;
	}
	return (char *)s;
}

char *strrchr(const char *s, int c)
{
	const char *last = NULL;
	char cc = (char)c;

	while (1)
	{
		if (*s == cc)
		{
			last = s;
		}
		if ('\0' == *s)
		{
			return (char *)last;
		}
		s++;
	}
}

char *strstr(const char *hay, const char *needle)
{
	if ('\0' == *needle)
	{
		return (char *)hay;
	}
	while (*hay)
	{
		const char *h = hay;
		const char *n = needle;

		while (*h && *n && *h == *n)
		{
			h++;
			n++;
		}
		if ('\0' == *n)
		{
			return (char *)hay;
		}
		hay++;
	}
	return NULL;
}

char *strtok(char *s, const char *delim)
{
	static char *save;

	if (NULL == s)
	{
		s = save;
		if (NULL == s)
		{
			return NULL;
		}
	}

	while (*s && strchr(delim, *s))
	{
		s++;
	}
	if ('\0' == *s)
	{
		save = NULL;
		return NULL;
	}

	{
		char *tok = s;

		while (*s && !strchr(delim, *s))
		{
			s++;
		}
		if (*s)
		{
			*s++ = '\0';
		}
		save = s;
		return tok;
	}
}

typedef unsigned int __attribute__((may_alias, aligned(1))) memword_t;

void *memcpy(void *dst, const void *src, size_t n)
{
	unsigned char *d = dst;
	const unsigned char *s = src;

	while (n && ((unsigned long)d & 3u))
	{
		*d++ = *s++;
		n--;
	}
	{
		memword_t *dw = (memword_t *)(void *)d;
		const memword_t *sw = (const memword_t *)(const void *)s;

		while (n >= sizeof *dw)
		{
			*dw++ = *sw++;
			n -= sizeof *dw;
		}
		d = (unsigned char *)dw;
		s = (const unsigned char *)sw;
	}
	while (n)
	{
		*d++ = *s++;
		n--;
	}
	return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
	unsigned char *d = dst;
	const unsigned char *s = src;

	if (d < s || d >= s + n)
	{
		return memcpy(dst, src, n);
	}
	{
		unsigned char *de = d + n;
		const unsigned char *se = s + n;

		while (n && ((unsigned long)de & 3u))
		{
			*--de = *--se;
			n--;
		}
		{
			memword_t *dw = (memword_t *)(void *)de;
			const memword_t *sw = (const memword_t *)(const void *)se;

			while (n >= sizeof *dw)
			{
				*--dw = *--sw;
				n -= sizeof *dw;
			}
			de = (unsigned char *)dw;
			se = (const unsigned char *)sw;
		}
		while (n)
		{
			*--de = *--se;
			n--;
		}
	}
	return dst;
}

void *memset(void *dst, int c, size_t n)
{
	unsigned char *d = dst;
	unsigned char b = (unsigned char)c;

	while (n && ((unsigned long)d & 3u))
	{
		*d++ = b;
		n--;
	}
	{
		memword_t pat = b;

		memword_t *dw = (memword_t *)(void *)d;

		pat |= pat << 8;
		pat |= pat << 16;
		while (n >= sizeof pat)
		{
			*dw++ = pat;
			n -= sizeof pat;
		}
		d = (unsigned char *)dw;
	}
	while (n)
	{
		*d++ = b;
		n--;
	}
	return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
	const unsigned char *pa = a;
	const unsigned char *pb = b;

	while (n && (((unsigned long)pa | (unsigned long)pb) & 3u))
	{
		if (*pa != *pb)
		{
			return (int)*pa - (int)*pb;
		}
		pa++;
		pb++;
		n--;
	}
	{
		const memword_t *wa = (const memword_t *)(const void *)pa;
		const memword_t *wb = (const memword_t *)(const void *)pb;

		while (n >= sizeof *wa)
		{
			if (*wa != *wb)
			{
				break;
			}
			wa++;
			wb++;
			n -= sizeof *wa;
		}
		pa = (const unsigned char *)wa;
		pb = (const unsigned char *)wb;
	}
	while (n && *pa == *pb)
	{
		pa++;
		pb++;
		n--;
	}
	if (0 == n)
	{
		return 0;
	}
	return (int)*pa - (int)*pb;
}
