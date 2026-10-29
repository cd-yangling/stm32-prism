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

#ifndef	__NEXUS_PAL_LED_HEADER__
#define	__NEXUS_PAL_LED_HEADER__

#include <stdint.h>

struct pwm_ch;
struct led_dev;

struct led_ops
{
	int		(*init)(const struct led_dev *l);
	int		(*on)(const struct led_dev *l);
	int		(*off)(const struct led_dev *l);
	int		(*toggle)(const struct led_dev *l);
	int		(*set_brightness)(const struct led_dev *l, uint8_t v);
};

struct led_dev
{
	const struct led_ops	*ops;
	const void				*ctx;
	const char				*name;
};

struct led_pwm
{
	const struct pwm_ch		*ch;
	uint32_t				period_ns;
};

extern const struct led_ops led_gpio_ops;
extern const struct led_ops led_pwm_ops;

int	led_init(const struct led_dev *l);
int	led_on(const struct led_dev *l);
int	led_off(const struct led_dev *l);
int	led_toggle(const struct led_dev *l);
int	led_set_brightness(const struct led_dev *l, uint8_t v);

#endif	//	__NEXUS_PAL_LED_HEADER__
