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

#include <stdio.h>
#include <errno.h>

#include "board.h"
#include "die.h"
#include "subsys/console.h"
#include "subsys/tty.h"
#include "pal/adc.h"
#include "pal/sensor.h"
#include "lib/gadget/fixtoa.h"

int main(void)
{
	uint16_t raw;
	int r;

	if ((NULL == board.console) || (NULL == board.adc) || (NULL == board.vref))
	{
		die();
	}

	tty_init(board.console);
	console_attach(board.console);

	adc_init(board.adc);
	adc_setup_channel(board.adc, 17u, 0u);
	adc_read(board.adc, 17u, &raw);
	printf("raw=%u vdda=%dmV\n", raw, 4096 * 1200 / (int)raw);

	adc_submit(board.adc, 17u);
	r = adc_poll(board.adc, &raw);
	printf("immediate poll r=%d\n", r);
	while (-EAGAIN == (r = adc_poll(board.adc, &raw)))
	{
	}
	printf("poll raw=%u r=%d\n", raw, r);

	sensor_init(board.vref);
	sensor_sample_fetch(board.vref);
	{
		struct sensor_value sv;
		char b[16];

		sensor_channel_get(board.vref, SENSOR_CHAN_VOLTAGE, &sv);
		fixtoa(b, sizeof(b), (int32_t)sensor_value_to_micro(&sv), 6);
		printf("vref=%s V\n", b);
	}

	for (;;)
	{
		__asm volatile("wfi");
	}
}
