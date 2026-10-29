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

#include <errno.h>
#include "pal/adc.h"

int adc_init(const struct adc_dev *d)
{
	return d->ops->init(d);
}

int adc_setup_channel(const struct adc_dev *d, uint8_t ch, uint16_t acq_us)
{
	return d->ops->setup_channel(d, ch, acq_us);
}

int adc_submit(const struct adc_dev *d, uint8_t ch)
{
	return d->ops->submit(d, ch);
}

int adc_poll(const struct adc_dev *d, uint16_t *raw)
{
	return d->ops->poll(d, raw);
}

int adc_read(const struct adc_dev *d, uint8_t ch, uint16_t *raw)
{
	int r = adc_submit(d, ch);

	if (0 != r)
	{
		return r;
	}

	do {
		r = adc_poll(d, raw);
	} while (-EAGAIN == r);

	return r;
}
