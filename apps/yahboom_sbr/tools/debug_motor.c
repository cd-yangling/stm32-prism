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

#include "tick.h"
#include <stdio.h>
#include "die.h"
#include "board.h"
#include "app_die.h"
#include "motor.h"
#include "dev/at8236.h"
#include "pal/enc.h"
#include "pal/pin.h"
#include "control/mt_speed.h"
#include "start_gate.h"
#include "tools/debug_motor.h"

//	电机基本测试
void debug_motor_basic_test(void)
{
	int ret;

	//	以 40% 的duty 正向/方向 分别旋转 2s
	ret = at8236_set(board.at8236_l, 400);
	printf("L+400 ret=%d\n", ret);
	delay_ms(2000);
	at8236_set(board.at8236_l, -400);
	printf("L-400\n");
	delay_ms(2000);
	at8236_coast(board.at8236_l);

	//	以 40% 的duty 正向/方向 分别旋转 2s
	at8236_set(board.at8236_r, 400);
	printf("R+400\n");
	delay_ms(2000);
	at8236_set(board.at8236_r, -400);
	printf("R-400\n");
	delay_ms(2000);
	at8236_coast(board.at8236_r);

	//	错误参数
	ret = at8236_set(board.at8236_l, 1001);
	printf("L+1001 ret=%d\n", ret);

	//	刹车测试
	at8236_brake(board.at8236_l);
	at8236_brake(board.at8236_r);
	printf("brake: hand-test\n");
	delay_ms(5000);

	//	滑行休眠 测试
	at8236_coast(board.at8236_l);
	at8236_coast(board.at8236_r);
	printf("coast: hand-test\n");
	delay_ms(2000);

	//	唤醒使用 40% 的duty 运行
	at8236_set(board.at8236_r, 300);
	printf("wake from sleep\n");
	delay_ms(2000);
	at8236_coast(board.at8236_r);
}

// 1/2: 方向标定测试(标定完毕,写board.c的配置)
void debug_motor_calibrate_dir(void)
{
	motor_set(board.motor_l, 300);
	motor_set(board.motor_r, 300);
	delay_ms(2000);

	motor_set(board.motor_l, 0);
	motor_set(board.motor_r, 0);
	delay_ms(1000);

	motor_set(board.motor_l, -300);
	motor_set(board.motor_r, -300);
	delay_ms(2000);

	motor_set(board.motor_l, 0);
	motor_set(board.motor_r, 0);
}

static int deadband_moved(const struct enc_dev *enc)
{
	int32_t a;
	int32_t b;
	int32_t moved;

	if (enc_get_count(enc, &a))
	{
		return -1;
	}

	delay_ms(800);

	if (enc_get_count(enc, &b))
	{
		return -1;
	}

	moved = b - a;

	if (0 > moved)
	{
		moved = -moved;
	}

	return (int)moved;
}

static int16_t deadband_sweep(const struct at8236_dev *m,
		const struct enc_dev *enc, int8_t sgn)
{
	int16_t hit;
	int16_t d;
	int16_t start;
	int mv;

	hit = 0;

	for (d = 10; d <= 1000; d += 10)
	{
		at8236_set(m, (int16_t)(sgn * d));
		mv = deadband_moved(enc);

		if (0 > mv)
		{
			at8236_set(m, 0);
			return -1;
		}

		if (5 <= mv)
		{
			hit = d;
			break;
		}
	}

	at8236_set(m, 0);

	if (0 == hit)
	{
		return 0;
	}

	start = (int16_t)(hit - 20);

	if (0 > start)
	{
		start = 0;
	}

	for (d = start; d < hit; d += 2)
	{
		at8236_set(m, (int16_t)(sgn * d));
		mv = deadband_moved(enc);

		if (0 > mv)
		{
			at8236_set(m, 0);
			return -1;
		}

		if (5 <= mv)
		{
			at8236_set(m, 0);
			return d;
		}
	}

	at8236_set(m, 0);

	return hit;
}

// 3:死区标定(地面受载,enc 持续旋转判据,标定完毕,写board.c的配置)
void debug_motor_calibrate_deadband(void)
{
	int16_t lp;
	int16_t ln;
	int16_t rp;
	int16_t rn;
	int16_t l_max;
	int16_t r_max;

	lp = deadband_sweep(board.at8236_l, board.enc_l->dev, 1);
	ln = deadband_sweep(board.at8236_l, board.enc_l->dev, -1);
	rp = deadband_sweep(board.at8236_r, board.enc_r->dev, 1);
	rn = deadband_sweep(board.at8236_r, board.enc_r->dev, -1);

	if (0 >= lp || 0 >= ln || 0 >= rp || 0 >= rn)
	{
		printf("FAIL no move or enc error: L+=%d L-=%d R+=%d R-=%d\n",
				lp, ln, rp, rn);
		return;
	}

	l_max = (lp > ln) ? lp : ln;
	r_max = (rp > rn) ? rp : rn;

	printf("L+=%d L-=%d R+=%d R-=%d\n", lp, ln, rp, rn);
	printf("fill board.c: motor_l.deadband=%d motor_r.deadband=%d (max+10)\n",
			(int16_t)(l_max + 10), (int16_t)(r_max + 10));
}


//	4:满占空极速测试(危险: key1 按下-松开放行后双轮满转 3s, 50ms 一轮)
void debug_motor_full_speed(void)
{
	int32_t	l0;
	int32_t	r0;
	int32_t	l1;
	int32_t	r1;
	int32_t	sum_l = 0;
	int32_t	sum_r = 0;
	int avg_l;
	int avg_r;
	int i;

	if (!board.enc_l || !board.enc_r)
	{
		app_die();
	}

	printf("full speed test: DANGER, press & release key1 to run\n");

	start_gate_wait();

	if (mt_speed_init(board.enc_l, board.enc_r))
	{
		app_die();
	}

	enc_get_count(board.enc_l->dev, &l0);
	enc_get_count(board.enc_r->dev, &r0);

	at8236_set(board.at8236_l, 1000);
	at8236_set(board.at8236_r, 1000);

	for (i = 0; i < 60; i++)
	{
		int32_t	dl;
		int32_t	dr;
		int16_t	mt_l;
		int16_t	mt_r;

		delay_ms(50);
		enc_get_count(board.enc_l->dev, &l1);
		enc_get_count(board.enc_r->dev, &r1);
		dl = l1 - l0;
		dr = r1 - r0;
		l0 = l1;
		r0 = r1;

		mt_speed_v2(&mt_l, &mt_r);

		printf("t=%d dL=%d dR=%d mtL=%dcps mtR=%dcps\n",
			(i + 1) * 50, (int)dl, (int)dr, mt_l, mt_r);

		if (50 <= i)
		{
			sum_l += dl;
			sum_r += dr;
		}
	}

	at8236_coast(board.at8236_l);
	at8236_coast(board.at8236_r);

	avg_l = (int)(sum_l / 10);
	avg_r = (int)(sum_r / 10);

	printf("steady: dL=%d dR=%d cnt/50ms\n", avg_l, avg_r);
	printf("L: %d cnt/s motor=%d RPM wheel=%d RPM\n",
		avg_l * 20, avg_l * 300 / 11, avg_l * 10 / 11);
	printf("R: %d cnt/s motor=%d RPM wheel=%d RPM\n",
		avg_r * 20, avg_r * 300 / 11, avg_r * 10 / 11);
}
