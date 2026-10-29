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

#ifndef	__NEXUS_PAL_FLASH_HEADER__
#define	__NEXUS_PAL_FLASH_HEADER__

#include <stdint.h>

struct flash;
struct flash_ops;

struct flash_ops
{
	int (*read)(const struct flash *f, uint32_t addr, uint8_t *buf, uint32_t len);
	int (*write)(const struct flash *f, uint32_t addr, const uint8_t *buf, uint32_t len);
	int (*erase)(const struct flash *f, uint32_t addr, uint32_t len);
	int (*read_jedec_id)(const struct flash *f, uint8_t *id);
};

struct flash_caps
{
	uint32_t	size;		//	容量字节
	uint32_t	id;			//	期望器件身份
	uint16_t	page;		//	编程页字节
	uint16_t	sector;		//	最小擦除单位
};

struct flash
{
	const struct flash_ops	*ops;
	const void			*ctx;
	const struct flash_caps	*caps;
	const char			*name;
};

int flash_read(const struct flash *f, uint32_t addr, uint8_t *buf, uint32_t len);
int flash_write(const struct flash *f, uint32_t addr, const uint8_t *buf, uint32_t len);
int flash_erase(const struct flash *f, uint32_t addr, uint32_t len);
int flash_read_jedec_id(const struct flash *f, uint8_t *id);

#endif	//	__NEXUS_PAL_FLASH_HEADER__
