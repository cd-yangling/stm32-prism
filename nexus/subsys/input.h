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

#ifndef	__NEXUS_SUBSYS_INPUT_HEADER__
#define	__NEXUS_SUBSYS_INPUT_HEADER__

#include <stdint.h>

#define	INPUT_EV_KEY		(0x01u)
#define	INPUT_KEY_1			(1u)
#define	INPUT_KEY_2			(2u)

struct input_event
{
	const void	*dev;
	uint8_t		 type;
	uint16_t	 code;
	int32_t		 value;
	uint8_t		 sync;
};

typedef void (*input_cb_t)(const struct input_event *ev, void *user);

struct input_callback
{
	input_cb_t		handler;
	const void		*dev;
	void			*user;
	struct input_callback	*next;
};

void input_callback_init(struct input_callback *cb, input_cb_t handler,
	const void *dev, void *user);
int input_register_callback(struct input_callback *cb);
int input_remove_callback(struct input_callback *cb);
void input_report(const void *dev, uint8_t type, uint16_t code,
	int32_t value, uint8_t sync);

#endif	//	__NEXUS_SUBSYS_INPUT_HEADER__
