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

#ifndef	__NEXUS_DEV_VOLTAGE_DIVIDER_HEADER__
#define	__NEXUS_DEV_VOLTAGE_DIVIDER_HEADER__

#include <stdint.h>
#include "pal/dev.h"

struct adc_dev;
struct sensor_ops;
struct voltage_divider_dat;

#define	VOLTAGE_DIVIDER_DAT_SIZE	(2)
#define	VOLTAGE_DIVIDER_DAT(name)	\
	OPAQUE_INST(struct voltage_divider_dat, name, VOLTAGE_DIVIDER_DAT_SIZE)

struct voltage_divider_dev
{
	const struct adc_dev		*adc;
	uint8_t						ch;
	uint16_t					full_ohms;
	uint16_t					output_ohms;
	struct voltage_divider_dat	*dat;
};

extern const struct sensor_ops voltage_divider_sensor_ops;


#endif	//	__NEXUS_DEV_VOLTAGE_DIVIDER_HEADER__
