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

#ifndef __APPS_WHEELTEC_F570_LIB_DEFRAMER_HEADER__
#define __APPS_WHEELTEC_F570_LIB_DEFRAMER_HEADER__

#define	DEFRAMER_FRAME_MAX	200u
#define	DEFRAMER_READY_LEN	4u

enum deframer_ck_kind
{
	DEFRAMER_CK_XOR,
	DEFRAMER_CK_SUM8,
};

struct deframer_cfg
{
	const unsigned char	*	head;
	unsigned short			head_len;
	unsigned short			frame_len;
	unsigned char			ck_kind;
	unsigned int			ck_init;
	unsigned short			ck_off;
	unsigned short			ck_pos;
	unsigned char			has_tail;
	unsigned char			tail_val;
};

struct deframer_rec
{
	unsigned char			buf[DEFRAMER_FRAME_MAX];
	unsigned short			len;
	unsigned int			ts;
};

struct deframer
{
	const struct deframer_cfg	*	cfg;
	unsigned short			pos;
	unsigned int			acc;
	unsigned int			fail;
	unsigned int			cur_ts;
	unsigned char			asm_buf[DEFRAMER_FRAME_MAX];
	struct deframer_rec		ready[DEFRAMER_READY_LEN];
	volatile unsigned int	rin;
	volatile unsigned int	rout;
};

int	deframer_init(struct deframer *f, const struct deframer_cfg *cfg);
int	deframer_feed(struct deframer *f, const unsigned char *data,
		unsigned short len, unsigned int ts);
const struct deframer_rec *deframer_peek(const struct deframer *f);
void deframer_advance(struct deframer *f);
unsigned int deframer_fail_get(const struct deframer *f);

#endif /* __APPS_WHEELTEC_F570_LIB_DEFRAMER_HEADER__ */
