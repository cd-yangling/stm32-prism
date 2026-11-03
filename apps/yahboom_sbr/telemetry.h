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

#ifndef	__APPS_YAHBOOM_SBR_TELEMETRY_HEADER__
#define	__APPS_YAHBOOM_SBR_TELEMETRY_HEADER__

#include <stdint.h>

struct tty;

enum tele_ch
{
	TELE_CH_PITCH,	//	2ms 0.01deg attitude pitch
	TELE_CH_GY,		//	2ms raw gyro y lsb
	TELE_CH_BOUT,	//	2ms balance loop out pwm
	TELE_CH_PERR,	//	2ms 0.01deg pitch-(mid+vel_off)
	TELE_CH_VSUM,	//	10ms cnt/s v_l+v_r
	TELE_CH_VT,		//	10ms cnt/s velocity target
	TELE_CH_VOFF,	//	10ms 0.01deg velocity loop out
	TELE_CH_VL,		//	10ms cnt/s left wheel
	TELE_CH_VR,		//	10ms cnt/s right wheel
	TELE_CH_VDIF,	//	10ms cnt/s v_l-v_r
	TELE_CH_TT,		//	10ms cnt/s turn target
	TELE_CH_TOUT,	//	10ms turn loop out pwm
	TELE_CH_LPWR,	//	2ms pwm left out-tout
	TELE_CH_RPWR,	//	2ms pwm right out+tout
	TELE_CH_N
};

void telemetry_init(struct tty *t);
void telemetry_feed(uint8_t ch, int16_t val);
int telemetry_set_mask(uint32_t mask);
uint32_t telemetry_get_mask(void);
void telemetry_set_up(uint8_t on);
uint8_t telemetry_get_up(void);
void telemetry_reset(void);
void telemetry_poll(void);

#endif	//	__APPS_YAHBOOM_SBR_TELEMETRY_HEADER__
