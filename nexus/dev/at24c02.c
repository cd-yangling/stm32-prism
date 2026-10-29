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
#include "pal/i2c.h"
#include "pal/eeprom.h"
#include "tick.h"
#include "dev/at24c02.h"

#define	TMO_MARGIN	10

//	写周期等待: EEPROM 内部编程期间对一切访问 NACK
static int at24c02_wait_ready(const struct eeprom *e)
{
	const struct i2c_dev *d = e->ctx;
	uint32_t budget = (uint32_t)e->caps->timeout_ms * TMO_MARGIN;
	uint32_t t0 = tick_get();
	int ret;

	while (-ENXIO == (ret = i2c_probe(d)))
	{
		if (tick_get() - t0 >= budget)
		{
			return -ETIMEDOUT;
		}

		delay_ms(1);
	}

	return ret;
}

static int at24c02_caps_ok(const struct eeprom *e)
{
	if (8 != e->caps->addr_width || 8 != e->caps->page)
	{
		return -EINVAL;
	}

	return 0;
}

//	随机读
static int at24c02_read(const struct eeprom *e, uint16_t off, uint8_t *buf, uint16_t len)
{
	const struct i2c_dev *d = e->ctx;
	uint8_t a = (uint8_t)off;
	int ret;

	if (0 != at24c02_caps_ok(e))
	{
		return -EINVAL;
	}

	if ((uint32_t)off + len > e->caps->size)
	{
		return -EINVAL;
	}

	if (0 == len)
	{
		return 0;
	}

	ret = at24c02_wait_ready(e);
	if (0 > ret)
	{
		return ret;
	}

	return i2c_mem_read(d, &a, 1, buf, len);
}

//	页切块写
static int at24c02_write(const struct eeprom *e, uint16_t off, const uint8_t *buf, uint16_t len)
{
	const struct i2c_dev *d = e->ctx;
	uint8_t block[1 + 8];
	uint16_t i;
	uint16_t n;
	int ret;

	if (0 != at24c02_caps_ok(e))
	{
		return -EINVAL;
	}

	if ((uint32_t)off + len > e->caps->size)
	{
		return -EINVAL;
	}

	while (len)
	{
		//	本块上限: 页内剩余与剩余总量取小
		n = e->caps->page - off % e->caps->page;

		if (len < n)
		{
			n = len;
		}

		//	上一块写周期(tWC)未完时器件 NACK, 等它就绪
		ret = at24c02_wait_ready(e);
		if (0 > ret)
		{
			return ret;
		}

		block[0] = (uint8_t)off;

		for (i = 0; i < n; i++)
		{
			block[1 + i] = buf[i];
		}

		ret = i2c_write(d, block, 1 + n);
		if (0 > ret)
		{
			return ret;
		}

		off += n;
		buf += n;
		len -= n;
	}

	return 0;
}

const struct eeprom_ops at24c02_ops =
{
	.read	= at24c02_read,
	.write	= at24c02_write,
};

const struct eeprom_caps at24c02_caps =
{
	.size		= 256,
	.page		= 8,
	.addr_width	= 8,
	.timeout_ms	= 5,
};
