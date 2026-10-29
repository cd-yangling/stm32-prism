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

#ifndef	__NEXUS_PAL_EEPROM_HEADER__
#define	__NEXUS_PAL_EEPROM_HEADER__

#include <stdint.h>

struct eeprom;
struct eeprom_ops;

struct eeprom_ops
{
	int (*read)(const struct eeprom *e, uint16_t off, uint8_t *buf, uint16_t len);
	int (*write)(const struct eeprom *e, uint16_t off, const uint8_t *buf, uint16_t len);
};

struct eeprom_caps
{
	uint32_t	size;		//	容量字节
	uint16_t	page;		//	页写字节(单次写跨页回卷,驱动内切块避开)
	uint8_t		addr_width;	//	内部地址宽 bit(8/16)
	uint8_t		timeout_ms;	//	写周期上限 tWC
};

struct eeprom
{
	const struct eeprom_ops	*ops;
	const void			*ctx;
	const struct eeprom_caps	*caps;
	const char			*name;
};

int eeprom_read(const struct eeprom *e, uint16_t off, uint8_t *buf, uint16_t len);
int eeprom_write(const struct eeprom *e, uint16_t off, const uint8_t *buf, uint16_t len);

#endif	//	__NEXUS_PAL_EEPROM_HEADER__
