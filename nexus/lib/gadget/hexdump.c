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

#include "hexdump.h"
#include <stdio.h>

static int printable(unsigned char c)
{
	return c >= 0x20 && c <= 0x7E;
}

void hexdump_sn(char *out, size_t outsz, const unsigned char *buf,
                size_t len, size_t addr)
{
	size_t pos = 0;

	if (0 == len || outsz < 79)
	{
		if (outsz)
		{
			out[0] = '\0';
		}
		return;
	}

	/* 地址(8 位十六进制补零) */
	pos += (size_t)snprintf(out + pos, outsz - pos, "%08x", (unsigned int)addr);

	/* hex 高 8 字节(或不足 8 就打到 len) */
	pos += (size_t)snprintf(out + pos, outsz - pos, "  ");
	for (size_t i = 0; i < 8 && i < len; i++)
	{
		pos += (size_t)snprintf(out + pos, outsz - pos, "%02x ", buf[i]);
	}

	if (len > 8)
	{
		pos += (size_t)snprintf(out + pos, outsz - pos, " ");
		for (size_t i = 8; i < 16 && i < len; i++)
		{
			pos += (size_t)snprintf(out + pos, outsz - pos, "%02x ", buf[i]);
		}
	}

	{
		size_t want = 60;

		while (pos < want && pos + 1 < outsz)
		{
			out[pos++] = ' ';
		}
	}

	/* ASCII 区 */
	pos += (size_t)snprintf(out + pos, outsz - pos, "|");
	for (size_t i = 0; i < len; i++)
	{
		pos += (size_t)snprintf(out + pos, outsz - pos, "%c",
		                printable(buf[i]) ? buf[i] : '.');
	}
	snprintf(out + pos, outsz - pos, "|");
}

void hex_dump(const unsigned char *buf, size_t len, size_t base)
{
	char line[96];   /* 79 最长(满行 78+NUL)*/

	for (size_t off = 0; off < len; off += 16)
	{
		size_t n = len - off >= 16 ? 16 : len - off;

		hexdump_sn(line, sizeof line, buf + off, n, base + off);
		printf("%s\n", line);
	}
}
