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
 * sscanf.c - 迷你 sscanf 实现, 纯字符串操作, 零硬件依赖
 *
 * 输入来自内存字符串, 比如收到的命令行.
 * 格式子集: %d %u %x %s %c %%
 * 格式串里的空格会跳过输入中的任意空白, 普通字符要求精确匹配.
 * 返回值是成功转换的字段数.
 *
 * 宽度:
 *   只有 %s 按照 %Ns, 调用方缓冲区必须 >= N+1.
 *   数值和 %c 的宽度会被解析但忽略, 比如 %3d 仍然读满整个数字, %3c 仍然只读 1 个字符.
 *
 * 各转换符行为:
 *   %d/%u/%x 先跳空白, 可选 +/- 号, %u/%x 会吃掉负号并按绝对值存.
 *   %x 认 0x/0X 前缀.
 *   %c 不跳空白, 读当前字符.
 *   %s 读到空白为止, 没有宽度时假定调用方缓冲区足够大
 *   32 位溢出不做检测
 *
 * 不支持, 遇到就当场停止解析, 不是报错:
 *   %* 赋值抑制, %ld 等长度修饰, %f, %o, %i, 扫描集 %[, %n.
 *   停止时返回已转换字段数, 这个数会小于期望值.
 *   如果不查返回值, 后面的输出变量就是未初始化读.
 */
#include <stdio.h>
#include <stdarg.h>

/* 输入指针 p 跳过空白(空格/Tab/\r/\n) */
static const char *skip_ws(const char *p)
{
	while (' ' == *p || '\t' == *p || '\r' == *p || '\n' == *p)
	{
		p++;
	}
	return p;
}

int sscanf(const char *str, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	const char *p = str;
	int nconv = 0;

	for (; *fmt; fmt++)
	{
		if ('%' != *fmt)
		{
			if (' ' == *fmt)
			{
				p = skip_ws(p);         /* 格式串空格:输入跳任意空白 */
			}
			else if (*p == *fmt)
			{
				p++;                    /* 普通字符:精确匹配 */
			}
			else
			{
				break;                  /* 不匹配:到此为止 */
			}
			continue;
		}
		fmt++;                          /* 吃掉 '%' */

		int width = 0;                  /* %Ns 的 N(0 = 未指定) */

		while (*fmt >= '0' && *fmt <= '9')
		{
			width = width * 10 + (*fmt - '0');
			if (width > 9999)
			{
				width = 9999;           /* 超宽,防 int 累加溢出 */
			}
			fmt++;
		}

		switch (*fmt)
		{
		case '%':
			if ('%' == *p)
			{
				p++;
			}
			else
			{
				goto done;
			}
			break;
		case 'd':
		case 'u':
		case 'x':
		{
			p = skip_ws(p);
			int neg = 0;

			if ('-' == *p || '+' == *p)
			{
				neg = ('-' == *p);
				p++;
			}
			unsigned int base = ('x' == *fmt) ? 16u : 10u;

			if (16u == base && '0' == p[0] && ('x' == p[1] || 'X' == p[1]))
			{
				p += 2;                 /* 跳 0x 前缀 */
			}
			unsigned int val = 0;
			int got = 0;

			for (;;)
			{
				char ch = *p;
				unsigned int d;

				if (ch >= '0' && ch <= '9')
				{
					d = (unsigned int)(ch - '0');
				}
				else if (16u == base && ch >= 'a' && ch <= 'f')
				{
					d = (unsigned int)(ch - 'a' + 10);
				}
				else if (16u == base && ch >= 'A' && ch <= 'F')
				{
					d = (unsigned int)(ch - 'A' + 10);
				}
				else
				{
					break;
				}
				val = val * base + d;
				p++;
				got = 1;
			}
			if (!got)
			{
				goto done;
			}
			if ('d' == *fmt)
			{
				*(va_arg(ap, int *)) = neg ? -(int)val : (int)val;
			}
			else
			{
				*(va_arg(ap, unsigned int *)) = val;
			}
			nconv++;
			break;
		}
		case 's':
		{
			p = skip_ws(p);
			if (!*p)
			{
				goto done;
			}
			char *out = va_arg(ap, char *);
			int k = 0;

			while (*p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
			{
				if (width > 0 && k >= width)
				{
					break;              /* %Ns:最多写 N 个字符(+NUL) */
				}
				out[k++] = *p++;
			}
			out[k] = '\0';
			nconv++;
			break;
		}
		case 'c':
			if (!*p)
			{
				goto done;              /* %c 不跳空白,读当前字符 */
			}
			*(va_arg(ap, char *)) = *p++;
			nconv++;
			break;
		default:
			goto done;                  /* 不认识的转换:停止(含 %*) */
		}
	}
done:
	va_end(ap);
	return nconv;
}
