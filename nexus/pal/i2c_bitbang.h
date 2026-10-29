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

#ifndef	__NEXUS_PAL_I2C_BITBANG_HEADER__
#define	__NEXUS_PAL_I2C_BITBANG_HEADER__

#if defined(CONFIG_I2C_BITBANG)

#include "pal/dev.h"

struct i2c_bus;
struct i2c_pins;
struct i2c_ops;
struct i2c_bitbang_dat;

extern const struct i2c_ops i2c_bitbang_ops;

#if defined(CONFIG_I2C_BITBANG_FAST)
#define	BITBANG_DAT_SIZE	24
#else
#define	BITBANG_DAT_SIZE	8
#endif

struct i2c_bitbang_ctx
{
	struct i2c_bitbang_dat	*dat;
	const struct i2c_pins	*pins;
};

#define	I2C_BITBANG_BUS(name, pins_v) \
	OPAQUE_INST(struct i2c_bitbang_dat, name##_dat, BITBANG_DAT_SIZE); \
	static const struct i2c_bitbang_ctx name##_ctx = { name##_dat, pins_v }; \
	static const struct i2c_bus name = { &i2c_bitbang_ops, &name##_ctx, #name }

#endif	//	CONFIG_I2C_BITBANG

#endif	//	__NEXUS_PAL_I2C_BITBANG_HEADER__
