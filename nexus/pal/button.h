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

#ifndef	__NEXUS_PAL_BUTTON_HEADER__
#define	__NEXUS_PAL_BUTTON_HEADER__

#include <stdint.h>
#include "pal/dev.h"

struct pin_desc;
struct button_dat;

#define	BUTTON_DAT_SIZE		16
#define	BUTTON_DAT(name)	OPAQUE_INST(struct button_dat, name, BUTTON_DAT_SIZE)

struct button_dev
{
	const struct pin_desc	*pin;
	uint16_t		debounce_ms;
	uint16_t		code;
	struct button_dat	*dat;
	const char		*name;
};

void button_init(const struct button_dev *b);
void button_poll(const struct button_dev *b);
int button_pressed(const struct button_dev *b);

#endif	//	__NEXUS_PAL_BUTTON_HEADER__
