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

#ifndef	__NEXUS_PAL_PWM_HEADER__
#define	__NEXUS_PAL_PWM_HEADER__

#include <stdint.h>

#define	PWM_POLARITY_NORMAL		(0u << 0)
#define	PWM_POLARITY_INVERTED	(1u << 0)

#define	PWM_PERIOD_NS(freq_hz)	(1000000000u / (freq_hz))

struct pin_desc;
struct pwm_dev;

struct pwm_ch
{
	const struct pwm_dev	*dev;
	uint8_t					ch;
	const struct pin_desc	*pin;
	uint32_t					flags;
};

struct pwm_ops
{
	int			(*init)(const struct pwm_dev *d, const struct pwm_ch *c);
	int			(*set_cycles)(const struct pwm_dev *d, uint8_t ch,
					uint32_t period, uint32_t pulse, uint32_t flags);
	uint32_t	(*get_cycles_per_sec)(const struct pwm_dev *d);
};

struct pwm_dev
{
	const struct pwm_ops	*ops;
	const void				*ctx;
	const char				*name;
};

int		pwm_init(const struct pwm_ch *c);
int		pwm_set(const struct pwm_ch *c, uint32_t period_ns, uint32_t pulse_ns);
uint32_t	pwm_get_cycles_per_sec(const struct pwm_dev *d);

#endif	//	__NEXUS_PAL_PWM_HEADER__
