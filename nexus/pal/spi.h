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

#ifndef	__NEXUS_PAL_SPI_HEADER__
#define	__NEXUS_PAL_SPI_HEADER__

#include <stdint.h>

struct spi_bus;
struct spi_pins;
struct spi_dev;
struct pin_desc;

struct spi_ops
{
	int (*init)(const struct spi_bus *b, const struct spi_pins *pins);
	void (*config)(const struct spi_bus *b, const struct spi_dev *d);
	int (*xfer)(const struct spi_bus *b, const uint8_t *tx, uint8_t *rx, uint16_t len);
};

struct spi_bus
{
	const struct spi_ops *ops;
	const void *ctx;
	const char *name;
};

struct spi_dev
{
	const struct spi_bus *bus;	//	挂载的SPI总线
	const struct pin_desc *cs;	//	片选
	uint32_t max_hz;			//	设备速度上限
	uint8_t mode;				//	0-3(CPOL<<1|CPHA)
	uint8_t lsb_first;			//	0=MSB / 1=LSB
	const char *name;			//	设备名字
};

int spi_bus_init(const struct spi_bus *b, const struct spi_pins *pins);

int spi_write(const struct spi_dev *d, const uint8_t *buf, uint16_t len);

int spi_read(const struct spi_dev *d, uint8_t *buf, uint16_t len);

int spi_write_then_read(const struct spi_dev *d, const uint8_t *wb, uint16_t wl, uint8_t *rb, uint16_t rl);

int spi_write_then_write(const struct spi_dev *d, const uint8_t *wb, uint16_t wl, const uint8_t *data, uint16_t dl);

struct spi_pins
{
	const struct pin_desc	*sck;
	const struct pin_desc	*miso;
	const struct pin_desc	*mosi;
};

#endif	//	__NEXUS_PAL_SPI_HEADER__
