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
#include "pal/pwm.h"

static int led_pwm_init(const struct led_dev *l)
{
	return pwm_init(((const struct led_pwm *)l->ctx)->ch);
}

static int led_pwm_on(const struct led_dev *l)
{
	const struct led_pwm *p = l->ctx;

	return pwm_set(p->ch, p->period_ns, p->period_ns);
}

static int led_pwm_off(const struct led_dev *l)
{
	const struct led_pwm *p = l->ctx;

	return pwm_set(p->ch, p->period_ns, 0u);
}

static int led_pwm_set_brightness(const struct led_dev *l, uint8_t v)
{
	const struct led_pwm *p = l->ctx;

	return pwm_set(p->ch, p->period_ns,
		(uint32_t)((uint64_t)p->period_ns * (uint64_t)v / 255u));
}

const struct led_ops led_pwm_ops =
{
	.init			= led_pwm_init,
	.on				= led_pwm_on,
	.off			= led_pwm_off,
	.toggle			= NULL,
	.set_brightness	= led_pwm_set_brightness,
};
