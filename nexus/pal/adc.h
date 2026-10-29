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

#ifndef	__NEXUS_PAL_ADC_HEADER__
#define	__NEXUS_PAL_ADC_HEADER__

#include <stdint.h>

struct adc_dev;

struct adc_ops
{
	int (*init)(const struct adc_dev *d);
	int (*setup_channel)(const struct adc_dev *d, uint8_t ch, uint16_t acq_us);
	int (*submit)(const struct adc_dev *d, uint8_t ch);
	int (*poll)(const struct adc_dev *d, uint16_t *raw);
};

struct adc_dev
{
	const struct adc_ops	*ops;
	const void				*ctx;
	const char				*name;
	uint16_t				ref_mv;
};

int	adc_init(const struct adc_dev *d);
int	adc_setup_channel(const struct adc_dev *d, uint8_t ch, uint16_t acq_us);
int	adc_submit(const struct adc_dev *d, uint8_t ch);
int	adc_poll(const struct adc_dev *d, uint16_t *raw);

//	read = submit + spin poll
int	adc_read(const struct adc_dev *d, uint8_t ch, uint16_t *raw);

static inline int32_t adc_raw_to_mv(const struct adc_dev *d, uint16_t raw)
{
	return (int32_t)((uint32_t)raw * d->ref_mv >> 12);
}

#endif	//	__NEXUS_PAL_ADC_HEADER__
