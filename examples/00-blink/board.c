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
#include "pal/led.h"
#include "board.h"

#define	BOARD_C
#include "soc.h"

#if defined(CONFIG_LED)
PIN_DESC(led_green, gpiob, 0, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);
PIN_DESC(led_blue,  gpiob, 1, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);
PIN_DESC(led_red,   gpiob, 5, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);

static const struct led_dev led_r = { .ops = &led_gpio_ops, .ctx = &led_red,   .name = "red" };
static const struct led_dev led_g = { .ops = &led_gpio_ops, .ctx = &led_green, .name = "green" };
static const struct led_dev led_b = { .ops = &led_gpio_ops, .ctx = &led_blue,  .name = "blue" };

static void brd_leds_init(void)
{
	led_init(&led_g);
	led_init(&led_b);
	led_init(&led_r);
}
#define	BOARD_LED_R	(&led_r)
#define	BOARD_LED_G	(&led_g)
#define	BOARD_LED_B	(&led_b)
#else
#define	BOARD_LED_R	(NULL)
#define	BOARD_LED_G	(NULL)
#define	BOARD_LED_B	(NULL)
#endif

const struct board board =
{
	.led_r	= BOARD_LED_R,
	.led_g	= BOARD_LED_G,
	.led_b	= BOARD_LED_B,
};

void brd_init(void)
{
#if defined(CONFIG_LED)
	brd_leds_init();
#endif
}
