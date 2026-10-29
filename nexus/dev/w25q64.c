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
#include "pal/spi.h"
#include "pal/flash.h"
#include "tick.h"
#include "dev/w25q64.h"

#define	TMO_BUSY_MS			2000

#define	CMD_JEDEC_ID		0x9Fu
#define	CMD_READ_STATUS		0x05u
#define	CMD_WRITE_ENABLE	0x06u
#define	CMD_READ_DATA		0x03u
#define	CMD_PAGE_PROGRAM	0x02u
#define	CMD_SECTOR_ERASE	0x20u

static int wait_busy(const struct flash *f)
{
	const struct spi_dev *d = f->ctx;
	uint32_t t0 = tick_get();
	uint8_t cmd = CMD_READ_STATUS;
	uint8_t st;
	int ret;

	while (1)
	{
		ret = spi_write_then_read(d, &cmd, 1, &st, 1);
		if (0 > ret)
		{
			return ret;
		}

		if (0 == (st & 0x01u))	//	bit0=BUSY
		{
			return 0;
		}

		if (tick_get() - t0 >= TMO_BUSY_MS)
		{
			return -ETIMEDOUT;
		}

		delay_ms(1);
	}
}

static int w25q64_read_jedec_id(const struct flash *f, uint8_t *id)
{
	const struct spi_dev *d = f->ctx;
	uint8_t cmd = CMD_JEDEC_ID;

	return spi_write_then_read(d, &cmd, 1, id, 3);
}

static int w25q64_read(const struct flash *f, uint32_t addr, uint8_t *buf, uint32_t len)
{
	const struct spi_dev *d = f->ctx;

	if (len > f->caps->size - addr)
	{
		return -EINVAL;
	}

	while (0 < len)
	{
		uint8_t head[4] =
		{
			CMD_READ_DATA,
			(uint8_t)(addr >> 16),
			(uint8_t)(addr >> 8),
			(uint8_t)addr,
		};
		uint32_t n = (0xFFFF < len) ? 0xFFFF : len;
		int ret = spi_write_then_read(d, head, 4, buf, (uint16_t)n);

		if (0 > ret)
		{
			return ret;
		}

		addr += n;
		buf += n;
		len -= n;
	}

	return 0;
}

static int w25q64_write(const struct flash *f, uint32_t addr, const uint8_t *buf, uint32_t len)
{
	const struct spi_dev *d = f->ctx;

	if (len > f->caps->size - addr)
	{
		return -EINVAL;
	}

	while (0 < len)
	{
		uint8_t head[4];
		uint8_t cmd = CMD_WRITE_ENABLE;
		uint32_t n = f->caps->page - addr % f->caps->page;
		int ret;

		if (len < n)
		{
			n = len;
		}

		ret = spi_write(d, &cmd, 1);
		if (0 > ret)
		{
			return ret;
		}

		head[0] = CMD_PAGE_PROGRAM;
		head[1] = (uint8_t)(addr >> 16);
		head[2] = (uint8_t)(addr >> 8);
		head[3] = (uint8_t)addr;
		ret = spi_write_then_write(d, head, 4, buf, (uint16_t)n);
		if (0 > ret)
		{
			return ret;
		}

		ret = wait_busy(f);
		if (0 > ret)
		{
			return ret;
		}

		addr += n;
		buf += n;
		len -= n;
	}

	return 0;
}

static int w25q64_erase(const struct flash *f, uint32_t addr, uint32_t len)
{
	const struct spi_dev *d = f->ctx;

	if (0 != (addr % f->caps->sector) || 0 != (len % f->caps->sector))
	{
		return -EINVAL;
	}

	if (len > f->caps->size - addr)
	{
		return -EINVAL;
	}

	while (0 < len)
	{
		uint8_t cmd = CMD_WRITE_ENABLE;
		uint8_t buf[4];
		int ret;

		ret = spi_write(d, &cmd, 1);
		if (0 > ret)
		{
			return ret;
		}

		buf[0] = CMD_SECTOR_ERASE;
		buf[1] = (uint8_t)(addr >> 16);
		buf[2] = (uint8_t)(addr >> 8);
		buf[3] = (uint8_t)addr;
		ret = spi_write(d, buf, 4);
		if (0 > ret)
		{
			return ret;
		}

		ret = wait_busy(f);
		if (0 > ret)
		{
			return ret;
		}

		addr += f->caps->sector;
		len -= f->caps->sector;
	}

	return 0;
}

const struct flash_ops w25q64_ops =
{
	.read		= w25q64_read,
	.write		= w25q64_write,
	.erase		= w25q64_erase,
	.read_jedec_id	= w25q64_read_jedec_id,
};

const struct flash_caps w25q64_caps =
{
	.size	= 8u << 20,
	.id		= 0xEF4017u,
	.page	= 256,
	.sector	= 4096,
};
