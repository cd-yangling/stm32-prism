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

#ifndef	__NEXUS_PAL_I2C_BUS_HEADER__
#define	__NEXUS_PAL_I2C_BUS_HEADER__

#include <stdint.h>

struct i2c_bus;
struct i2c_pins;

struct i2c_ops
{
	int (*init)(const struct i2c_bus *b, const struct i2c_pins *pins);
	void (*start)(const struct i2c_bus *b);
	int (*xfer)(const struct i2c_bus *b, uint8_t addr7, int rd, uint8_t *buf, uint16_t len);
	void (*stop)(const struct i2c_bus *b);
	void (*config)(const struct i2c_bus *b, const uint32_t spd_hz);
};

struct i2c_bus
{
	const struct i2c_ops *ops;
	const void *ctx;
	const char * name;
};

struct i2c_dev
{
	const struct i2c_bus *bus;
	uint16_t addr7;
	uint32_t spd_hz;
	const char *name;
};

int i2c_bus_init(const struct i2c_bus *b, const struct i2c_pins *pins);

int i2c_write(const struct i2c_dev *d, const uint8_t *buf, uint16_t len);

int i2c_read(const struct i2c_dev *d, uint8_t *buf, uint16_t len);

int i2c_mem_read(const struct i2c_dev *d, uint8_t *mb, uint16_t ml, uint8_t *rb, uint16_t rl);

int i2c_probe(const struct i2c_dev *d);

struct pin_desc;

struct i2c_pins
{
	const struct pin_desc	*scl;
	const struct pin_desc	*sda;
};

#endif	//	__NEXUS_PAL_I2C_BUS_HEADER__

