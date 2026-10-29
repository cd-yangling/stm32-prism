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

#include <stddef.h>

#include "pal/led.h"
#include "pal/pin.h"

static int led_gpio_init(const struct led_dev *l)
{
	pin_configure((const struct pin_desc *)l->ctx);
	return 0;
}

static int led_gpio_on(const struct led_dev *l)
{
	pin_set((const struct pin_desc *)l->ctx, 1);
	return 0;
}

static int led_gpio_off(const struct led_dev *l)
{
	pin_set((const struct pin_desc *)l->ctx, 0);
	return 0;
}

static int led_gpio_toggle(const struct led_dev *l)
{
	pin_toggle((const struct pin_desc *)l->ctx);
	return 0;
}

const struct led_ops led_gpio_ops =
{
	.init			= led_gpio_init,
	.on				= led_gpio_on,
	.off			= led_gpio_off,
	.toggle			= led_gpio_toggle,
	.set_brightness	= NULL,
};
