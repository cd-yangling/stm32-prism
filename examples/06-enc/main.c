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

#include "tick.h"
#include "board.h"
#include "die.h"
#include "subsys/console.h"
#include "subsys/tty.h"
#include "pal/enc.h"

int main(void)
{
	int32_t cnt;
	int32_t prev;
	int32_t d;

	if ((NULL == board.console) || (NULL == board.enc))
	{
		die();
	}

	tty_init(board.console);
	console_attach(board.console);
	printf("=== 06-enc: EC11 -> TIM3 x4 (PC6/PC7) ===\n");

	enc_init(board.enc);
	enc_get_count(board.enc->dev, &prev);

	while (1)
	{
		enc_get_count(board.enc->dev, &cnt);
		if (cnt != prev)
		{
			d = cnt - prev;
			printf("cnt=%d delta=%d dir=%s\n",
				(int)cnt, (int)d, (0 < d) ? "CW" : "CCW");
			prev = cnt;
		}
		delay_ms(100u);
	}
}
