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

#ifndef	__NEXUS_DEV_AT8236_HEADER__
#define	__NEXUS_DEV_AT8236_HEADER__

#include <stdint.h>

struct pwm_ch;

#define	AT8236_DECAY_FAST	(0u << 0)
#define	AT8236_DECAY_SLOW	(1u << 0)

#define	AT8236_PERIOD_NS_MAX	(4294967u)

struct at8236_dev
{
	const struct pwm_ch	*in1;
	const struct pwm_ch	*in2;
	uint32_t			period_ns;
	uint32_t			flags;
};

int		at8236_init(const struct at8236_dev *m);
int		at8236_set(const struct at8236_dev *m, int16_t duty);
int		at8236_brake(const struct at8236_dev *m);
int		at8236_coast(const struct at8236_dev *m);

#endif	//	__NEXUS_DEV_AT8236_HEADER__
