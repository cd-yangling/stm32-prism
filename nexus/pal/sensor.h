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

#ifndef	__NEXUS_PAL_SENSOR_HEADER__
#define	__NEXUS_PAL_SENSOR_HEADER__

#include <stdint.h>

//	原始数据类型
enum sensor_raw_type
{
	SENSOR_RAW_INT16,
	SENSOR_RAW_UINT16,
	SENSOR_RAW_INT32,
	SENSOR_RAW_UINT32,
	SENSOR_RAW_Q15,
	SENSOR_RAW_Q31,
};

struct sensor_raw_value
{
	enum sensor_raw_type type;
	union
	{
		int16_t		i16;
		int32_t		i32;
		uint16_t	u16;
		uint32_t	u32;
	} data;
};

//	泛型传感器(参考 zephyr 裁剪)

//	 0.5: val1 =  0, val2 =  500000
//	-0.5: val1 =  0, val2 = -500000
//	-1.0: val1 = -1, val2 =  0
//	-1.5: val1 = -1, val2 = -500000
struct sensor_value
{
	int32_t val1;					//	整数部分
	int32_t val2;					//	小数部分(单位: 10^-6)(微)
};

static inline int64_t sensor_value_to_micro(const struct sensor_value *sv)
{
	return ((int64_t)sv->val1 * 1000000) + sv->val2;
}

static inline void micro_to_sensor_value(int64_t micro, struct sensor_value *sv)
{
	int32_t m = (int32_t)(uint32_t)micro;

	if ((int64_t)m == micro)	/* 直接用 32 位硬件除, 避免 64 位除法 */
	{
		sv->val1 = m / 1000000;
		sv->val2 = m % 1000000;
	}
	else
	{
		sv->val1 = (int32_t)(micro / 1000000LL);
		sv->val2 = (int32_t)(micro % 1000000LL);
	}
}

static inline double sensor_value_to_double(const struct sensor_value *sv)
{
	return (double)sv->val1 + (double)sv->val2 / 1000000;
}

static inline float sensor_value_to_float(const struct sensor_value *sv)
{
	return (float)sv->val1 + (float)sv->val2 / 1000000;
}

//	国际单位制 SI:
enum sensor_chan
{
	SENSOR_CHAN_ALL = 0,			//	全通道/不指定
	SENSOR_CHAN_ACCEL_XYZ = 1,		//	m/s^2
	SENSOR_CHAN_ACCEL_X,			//	m/s^2
	SENSOR_CHAN_ACCEL_Y,			//	m/s^2
	SENSOR_CHAN_ACCEL_Z,			//	m/s^2

	SENSOR_CHAN_GYRO_XYZ,			//	rad/s
	SENSOR_CHAN_GYRO_X,				//	rad/s
	SENSOR_CHAN_GYRO_Y,				//	rad/s
	SENSOR_CHAN_GYRO_Z,				//	rad/s

	SENSOR_CHAN_DIE_TEMP,			//	摄氏度
	SENSOR_CHAN_VOLTAGE,			//	伏
};

struct sensor;

//	attr 属性(final_value = raw + offset, 单位 LSB)
enum sensor_attr
{
	SENSOR_ATTR_OFFSET,				//	零偏修正量, 依赖量程: 须与校准时同 cfg
};

enum sensor_trigger_type
{
	SENSOR_TRIG_DATA_READY,
};

struct sensor_trigger
{
	enum sensor_trigger_type	type;
	enum sensor_chan			chan;
};

typedef void (*sensor_trigger_handler_t)(const struct sensor *s,
					const struct sensor_trigger *trig);

struct sensor_ops
{
	int (*init)(const struct sensor *s);
	int (*sample_fetch)(const struct sensor *s);
	int (*channel_get)(const struct sensor *s, enum sensor_chan sc, struct sensor_value *sv);
	int (*channel_get_raw)(const struct sensor *s, enum sensor_chan sc, struct sensor_raw_value *srv);
	int (*trigger_set)(const struct sensor *s, const struct sensor_trigger *trig,
		sensor_trigger_handler_t handler);
	int (*attr_set_raw)(const struct sensor *s, enum sensor_chan sc,
		enum sensor_attr attr, const struct sensor_raw_value *srv);

	int (*submit)(const struct sensor *s);
	int (*poll)(const struct sensor *s);
};

struct sensor
{
	const struct sensor_ops *ops;
	const void *ctx;
	const char *name;
};

int sensor_init(const struct sensor *s);
int sensor_sample_fetch(const struct sensor *s);
int sensor_channel_get(const struct sensor *s, enum sensor_chan sc, struct sensor_value *sv);
int sensor_channel_get_raw(const struct sensor *s, enum sensor_chan sc, struct sensor_raw_value *srv);
int sensor_trigger_set(const struct sensor *s, const struct sensor_trigger *trig,
	sensor_trigger_handler_t handler);
int sensor_attr_set_raw(const struct sensor *s, enum sensor_chan sc,
	enum sensor_attr attr, const struct sensor_raw_value *srv);
int sensor_submit(const struct sensor *s);
int sensor_poll(const struct sensor *s);

#endif	//	__NEXUS_PAL_SENSOR_HEADER__
