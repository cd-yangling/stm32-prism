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

#ifndef	__NEXUS_PAL_ENC_HEADER__
#define	__NEXUS_PAL_ENC_HEADER__

#include <stdint.h>

struct pin_desc;
struct enc_dev;

enum enc_mode
{
	ENC_MODE_X2_CH2	= 1,		//	SMS=001:计 CH2 边沿,看 CH1 电平
	ENC_MODE_X2_CH1	= 2,		//	SMS=010:计 CH1 边沿,看 CH2 电平
	ENC_MODE_X4		= 3,		//	SMS=011:双边沿,x4(常用缺省)
};

//	CKD:fDTS = fCK_INT / div
#define	ENC_CKD_DIV1	((uint8_t)0)
#define	ENC_CKD_DIV2	((uint8_t)1)
#define	ENC_CKD_DIV4	((uint8_t)2)

struct enc_ch
{
	const struct enc_dev	*dev;
	const struct pin_desc	*pin_a;		//	CH1(A 相)
	const struct pin_desc	*pin_b;		//	CH2(B 相)
	enum enc_mode			mode;
	uint8_t					filter;		//	IC1F/IC2F 4bit 原值,0=无滤波
	uint8_t					ckd;		//	ENC_CKD_*
};

struct enc_ops
{
	int	(*init)(const struct enc_dev *e, const struct enc_ch *c);
	int	(*get_count)(const struct enc_dev *e, int32_t *cnt);
	uint16_t	(*get_raw)(const struct enc_dev *e);
};

struct enc_dev
{
	const struct enc_ops	*ops;
	const void				*ctx;
	const char				*name;
};

int	enc_init(const struct enc_ch *c);
int	enc_get_count(const struct enc_dev *e, int32_t *cnt);
uint16_t	enc_get_raw(const struct enc_dev *e);

#endif	//	__NEXUS_PAL_ENC_HEADER__
