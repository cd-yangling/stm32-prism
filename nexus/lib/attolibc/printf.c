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

/*
 * printf.c - 迷你 printf / snprintf
 *
 * 只实现常用子集
 *
 * 支持:
 *   %c %s %d %u %x %p %%
 *   最小宽度, '0' 补齐, '-' 左对齐
 *
 * 细节:
 *   - '0' 补齐只对数字有效, %05s 还是空格补
 *   - 负数补零时负号提前, 比如 %05d 输出 -0042
 *   - %p 固定输出 "0x" 加 8 位小写十六进制
 *
 * 不支持:
 *   %ld %lu %lx, %X, %o, %f %e %g, %*s, 精度 '.'
 *
 * 注意:
 *   不认识的格式符不报错, 原样输出, 而且不消耗参数.
 *   后面的 va_arg 全部错位, 输出静默乱掉.
 *   要用 long, 先手动 cast 成 int / unsigned int, 再用 %d / %u / %x.
 *
 * 实现:
 *   vformat 是核心, 逐字符格式化, 通过 emit 回调输出.
 *   emit_putc 给 printf 用, 顺便把 \n 展开成 \r\n.
 *   emit_sn 给 snprintf 用, 写缓冲区, 末尾留 1 字节放 '\0'.
 */
#include <stdio.h>
#include <stdarg.h>

static void (*out_putc)(char);

void printf_init(void (*put)(char))
{
	out_putc = put;
}

/* ---- snprintf 的写入上下文 ---- */
typedef struct
{
	char   *buf;
	size_t  len;   /* 已写入字符数 */
	size_t  cap;   /* 缓冲总大小(含留给 NULL 的 1 字节) */
} snctx;

static void emit_putc(void *ctx, char c)
{
	(void)ctx;
	if (out_putc)
	{
		if ('\n' == c)
		{
			out_putc('\r');      /* 串口终端:\n 展开为 \r\n */
		}
		out_putc(c);
	}
}

static void emit_sn(void *ctx, char c)
{
	snctx *s = ctx;

	if (s->len + 1 < s->cap)
	{
		s->buf[s->len++] = c;    /* 留 1 字节给 NULL;满了丢弃(截断) */
	}
}

/* ---- 格式化核心:逐字符 fmt,经 emit 输出,返回输出总数 ---- */
static int vformat(void (*emit)(void *, char), void *ctx,
                   const char *fmt, va_list ap)
{
	unsigned int count = 0;
	char tmp[12];                /* 32 位整数最大 10 位 + 负号 + NULL */

	for (; *fmt; fmt++)
	{
		if ('%' != *fmt)
		{
			emit(ctx, *fmt);
			count++;
			continue;
		}
		fmt++;                   /* 吃掉 '%' */

		/* -- 解析 '-' / '0' 标志与十进制宽度 -- */
		int left = 0, zero = 0, width = 0;

		for (;;)
		{
			if ('-' == *fmt)
			{
				left = 1;
				fmt++;
				continue;
			}
			if ('0' == *fmt && 0 == width)
			{
				zero = 1;
				fmt++;
				continue;
			}
			break;
		}
		while (*fmt >= '0' && *fmt <= '9')
		{
			width = width * 10 + (*fmt - '0');
			if (width > 9999)
			{
				width = 9999;    /* 超宽,防 int 累加溢出(UB) */
			}
			fmt++;
		}

		/* -- 按转换符生成字符串(s/slen)与数值标记(num) -- */
		const char  *s = 0;
		unsigned int slen = 0;
		int num = 0;

		switch (*fmt)
		{
		case '%':
			emit(ctx, '%');
			count++;
			continue;
		case 'c':
		{
			char c = (char)va_arg(ap, int);

			tmp[0] = c;
			s = tmp;
			slen = 1;
			break;
		}
		case 's':
			s = va_arg(ap, const char *);
			if (!s)
			{
				s = "(null)";
			}
			while (s[slen])
			{
				slen++;
			}
			break;
		case 'd':
		{
			int v = va_arg(ap, int);
			unsigned int u = (v < 0) ? -(unsigned int)v : (unsigned int)v;
			int i = (int)sizeof tmp;

			tmp[--i] = '\0';
			do
			{
				tmp[--i] = (char)('0' + u % 10u);
				u /= 10u;
			} while (u);
			if (v < 0)
			{
				tmp[--i] = '-';
			}
			num = 1;
			s = &tmp[i];
			slen = sizeof tmp - 1u - (unsigned int)i;
			break;
		}
		case 'u':
		case 'x':
		case 'p':
		{
			unsigned int u = va_arg(ap, unsigned int);
			int is_hex = (*fmt != 'u');
			int min_digits = ('p' == *fmt) ? 8 : 0;  /* %p:固定 8 位十六进制 */
			int i = (int)sizeof tmp;

			tmp[--i] = '\0';
			do
			{
				unsigned int d = u % (is_hex ? 16u : 10u);

				tmp[--i] = (char)(d < 10u ? '0' + d : 'a' + (d - 10u));
				u /= (is_hex ? 16u : 10u);
			} while (u);
			while ((int)(sizeof tmp - 1u - (unsigned int)i) < min_digits)
			{
				tmp[--i] = '0';
			}
			if ('p' == *fmt && i >= 2)
			{
				tmp[--i] = 'x';
				tmp[--i] = '0';
			}
			num = 1;
			s = &tmp[i];
			slen = sizeof tmp - 1u - (unsigned int)i;
			break;
		}
		default:                 /* 不认识的转换符:原样吐出,不耗参数! */
			emit(ctx, '%');
			count++;
			if (!*fmt)
			{
				return (int)count; /* 孤立 '%' 结尾:吐 '%' 即停,防越过 NULL 越界读 */
			}
			emit(ctx, *fmt);
			count++;
			continue;
		}

		/* -- padding + 输出 -- */
		if (num && zero && !left && '-' == s[0])
		{
			emit(ctx, '-');      /* 补零时负号必须在最前:%05d/-42 -> -0042 */
			count++;
			s++;
			slen--;
			if (width > 0)
			{
				width--;         /* 负号已占掉总宽度的 1 列,防止多补一个 0 */
			}
		}
		if (!left)
		{
			unsigned int pad = ((unsigned int)width > slen)
			                   ? (unsigned int)width - slen : 0;

			for (unsigned int k = 0; k < pad; k++)
			{
				emit(ctx, zero && num ? '0' : ' ');
				count++;
			}
		}
		for (unsigned int k = 0; k < slen; k++)
		{
			emit(ctx, s[k]);
			count++;
		}
		if (left)
		{
			unsigned int pad = ((unsigned int)width > slen)
			                   ? (unsigned int)width - slen : 0;

			for (unsigned int k = 0; k < pad; k++)
			{
				emit(ctx, ' ');
				count++;
			}
		}
	}
	return (int)count;
}

int printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	int n = vformat(emit_putc, 0, fmt, ap);

	va_end(ap);
	return n;
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
	snctx ctx = { buf, 0, size };
	va_list ap;

	va_start(ap, fmt);
	int n = vformat(emit_sn, &ctx, fmt, ap);

	va_end(ap);
	if (size)
	{
		buf[ctx.len] = '\0';
	}
	return n;
}
