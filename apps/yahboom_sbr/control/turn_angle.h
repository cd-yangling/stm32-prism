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

#ifndef	__APPS_YAHBOOM_SBR_CONTROL_TURN_ANGLE_HEADER__
#define	__APPS_YAHBOOM_SBR_CONTROL_TURN_ANGLE_HEADER__

#include <stdint.h>

struct turn_angle_config
{
	int16_t	kp_num;
	uint8_t	kp_shr;
	int16_t	out_max;
	uint16_t timeout_ms;
};

enum turn_angle_st
{
	TURN_ANGLE_IDLE,
	TURN_ANGLE_RUN,
	TURN_ANGLE_DONE,
	TURN_ANGLE_ETIMEOUT,
	TURN_ANGLE_ABORTED,
};

int turn_angle_measuring_start(int32_t cd);
void turn_angle_measuring_abort(void);
void turn_angle_update_yaw(int16_t gz, uint16_t dt_10us,
		int32_t num, int shr, int16_t gz_lsb_per_dps);
int turn_angle_state_get(void);
int32_t turn_angle_yaw_get(void);
uint32_t turn_angle_measuring_elapsed_ms(void);
void turn_angle_poll(const struct turn_angle_config *cfg);

#endif	//	__APPS_YAHBOOM_SBR_CONTROL_TURN_ANGLE_HEADER__
