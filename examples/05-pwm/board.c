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
#include <stddef.h>

#include "pal/pin.h"
#include "pal/pwm.h"
#include "pal/led.h"
#include "board.h"

#define	BOARD_C
#include "soc.h"

#if defined(CONFIG_PWM_TIM3)
PIN_DESC(pwm_g_pin, gpiob, 0, PIN_ALT | PIN_SPEED_HIGH);

static const struct pwm_ch pwm_g =
{
	.dev	= &tim3,
	.ch		= 3,
	.pin	= &pwm_g_pin,
	.flags	= PWM_POLARITY_INVERTED,
};

static const struct led_pwm led_g_pwm =
{
	.ch			= &pwm_g,
	.period_ns	= 1000000u,
};

static const struct led_dev led_g =
{
	.ops	= &led_pwm_ops,
	.ctx	= &led_g_pwm,
	.name	= "green",
};

static void brd_led_init(void)
{
	led_init(&led_g);
}

#define	BOARD_PWM_G	(&pwm_g)
#define	BOARD_LED_G	(&led_g)
#else
#define	BOARD_PWM_G	(NULL)
#define	BOARD_LED_G	(NULL)
#endif

const struct board board =
{
	.pwm_g	= BOARD_PWM_G,
	.led_g	= BOARD_LED_G,
};

void brd_init(void)
{
#if defined(CONFIG_PWM_TIM3)
	brd_led_init();
#endif
}
