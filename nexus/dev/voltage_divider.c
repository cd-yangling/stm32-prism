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
#include "pal/sensor.h"
#include "pal/adc.h"
#include "dev/voltage_divider.h"

struct voltage_divider_dat
{
	uint16_t		raw;
};

_Static_assert(sizeof(struct voltage_divider_dat) == VOLTAGE_DIVIDER_DAT_SIZE, "VOLTAGE_DIVIDER_DAT_SIZE out of sync with struct voltage_divider_dat");

static int voltage_divider_init(const struct sensor *s)
{
	const struct voltage_divider_dev *v = s->ctx;
	int r = adc_init(v->adc);

	if (0 != r)
	{
		return r;
	}

	return adc_setup_channel(v->adc, v->ch, 0u);
}

static int voltage_divider_sample_fetch(const struct sensor *s)
{
	const struct voltage_divider_dev *v = s->ctx;

	return adc_read(v->adc, v->ch, &v->dat->raw);
}

static int voltage_divider_channel_get(const struct sensor *s, enum sensor_chan sc,
	struct sensor_value *sv)
{
	const struct voltage_divider_dev *v = s->ctx;
	int32_t mv;
	int64_t uv;

	if (SENSOR_CHAN_VOLTAGE != sc)
	{
		return -EOPNOTSUPP;
	}

	mv = adc_raw_to_mv(v->adc, v->dat->raw);
	uv = (int64_t)mv * 1000 * v->full_ohms / v->output_ohms;
	micro_to_sensor_value(uv, sv);

	return 0;
}

static int voltage_divider_channel_get_raw(const struct sensor *s, enum sensor_chan sc,
	struct sensor_raw_value *srv)
{
	const struct voltage_divider_dev *v = s->ctx;

	if (SENSOR_CHAN_VOLTAGE != sc)
	{
		return -EOPNOTSUPP;
	}

	srv->type		= SENSOR_RAW_UINT16;
	srv->data.u16	= v->dat->raw;

	return 0;
}

static int voltage_divider_submit(const struct sensor *s)
{
	const struct voltage_divider_dev *v = s->ctx;

	return adc_submit(v->adc, v->ch);
}

static int voltage_divider_poll(const struct sensor *s)
{
	const struct voltage_divider_dev *v = s->ctx;

	return adc_poll(v->adc, &v->dat->raw);
}

const struct sensor_ops voltage_divider_sensor_ops =
{
	.init				= voltage_divider_init,
	.sample_fetch		= voltage_divider_sample_fetch,
	.channel_get		= voltage_divider_channel_get,
	.channel_get_raw	= voltage_divider_channel_get_raw,
	.submit				= voltage_divider_submit,
	.poll				= voltage_divider_poll,
};
