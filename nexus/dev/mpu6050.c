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

#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include "tick.h"
#include "pal/i2c.h"
#include "pal/pin.h"
#include "pal/sensor.h"
#include "dev/mpu6050.h"

#define	MPU_SMPLRT_DIV			0x19u
#define	MPU_CONFIG				0x1Au
#define	MPU_GYRO_CONFIG			0x1Bu
#define	MPU_ACCEL_CONFIG		0x1Cu
#define	MPU_ACCEL_XOUT_H		0x3Bu
#define	MPU_INT_PIN_CFG			0x37u
#define	MPU_INT_ENABLE			0x38u
#define	MPU_PWR_MGMT_1			0x6Bu
#define	MPU_WHO_AM_I			0x75u

#define	MPU_ID					0x68u

struct mpu6050_dat
{
	int16_t						raw[7];
	struct pin_callback			cb;
	sensor_trigger_handler_t	h;
	const struct sensor		*	s;
	int16_t						offset[6];	//	0..2=ax ay az, 3..5=gx gy gz (LSB)
};

_Static_assert(sizeof(struct mpu6050_dat) == MPU6050_DAT_SIZE, "MPU6050_DAT_SIZE out of sync with struct mpu6050_dat");

//	写一个寄存器 = [START][W][reg][val][STOP]
static int wr_reg(const struct i2c_dev *d, uint8_t reg, uint8_t val)
{
	uint8_t b[2] = {reg, val};

	return i2c_write(d, b, 2u);
}

//	从一个寄存器 起连读 n 字节(dummy write 定位 + repeated START,组合器)
static int rd_regs(const struct i2c_dev *d, uint8_t reg, uint8_t *buf, uint16_t n)
{
	return i2c_mem_read(d, &reg, 1u, buf, n);
}

static int mpu6050_init(const struct sensor *s)
{
	const struct mpu6050_dev *m = s->ctx;
	uint8_t id;
	int r;

	//	读取 MPU_ID
	r = rd_regs(m->i2c, MPU_WHO_AM_I, &id, 1u);
	if (0 != r)
	{
		return r;
	}

	if (MPU_ID != id)
	{
		return -ENODEV;
	}

	/* 唤醒 + CLKSEL=001 手册推荐 PLL(X 轴陀螺参考) */
	r = wr_reg(m->i2c, MPU_PWR_MGMT_1, 0x01u);
	if (0 != r)
	{
		return r;
	}

	//	采样率 设置
	r = wr_reg(m->i2c, MPU_SMPLRT_DIV, m->smplrt_div);
	if (0 != r)
	{
		return r;
	}

	//	DLPF 设置
	r = wr_reg(m->i2c, MPU_CONFIG, m->dlpf_cfg);
	if (0 != r)
	{
		return r;
	}

	//	陀螺仪 FS_SEL 设置
	r = wr_reg(m->i2c, MPU_GYRO_CONFIG, (uint8_t)(m->gfs_sel << 3));
	if (0 != r)
	{
		return r;
	}

	//	加速度计 AFS_SEL 设置
	r = wr_reg(m->i2c, MPU_ACCEL_CONFIG, (uint8_t)(m->afs_sel << 3));
	if (0 != r)
	{
		return r;
	}

	delay_ms(30u);
	return 0;
}

//	14B 原子连读进快照:ax..gz 同一采样时刻
static int mpu6050_sample_fetch(const struct sensor *s)
{
	const struct mpu6050_dev *m = s->ctx;
	struct mpu6050_dat *dat = m->dat;
	uint8_t b[14];
	int r = rd_regs(m->i2c, MPU_ACCEL_XOUT_H, b, sizeof(b));
	int i;

	if (0 != r)
	{
		return r;
	}

	for (i = 0; i < 7; i++)
	{
		dat->raw[i] = (int16_t)((b[2 * i] << 8) | b[2 * i + 1]);
	}

	return 0;
}

//	按 芯片轴向 -> 逻辑轴向 映射表 将三元组翻译成逻辑轴向 x/y/z
static void mpu6050_map_axes(int16_t v[3], const struct mpu6050_axis_map *mp)
{
	int16_t s[3] = { v[0], v[1], v[2] };
	int16_t temp;

	//	x
	temp = s[mp->x.src];
	v[0] = (mp->x.neg) ? (int16_t)-temp : temp;

	//	y
	temp = s[mp->y.src];
	v[1] = (mp->y.neg) ? (int16_t)-temp : temp;

	//	z
	temp = s[mp->z.src];
	v[2] = (mp->z.neg) ? (int16_t)-temp : temp;
}

//	逆映射: 逻辑轴向三元组换回芯片通道
static void mpu6050_unmap_axes(int16_t v[3], const struct mpu6050_axis_map *mp)
{
	int16_t s[3] = { v[0], v[1], v[2] };

	//	x
	v[mp->x.src] = (mp->x.neg) ? (int16_t)-s[0] : s[0];

	//	y
	v[mp->y.src] = (mp->y.neg) ? (int16_t)-s[1] : s[1];

	//	z
	v[mp->z.src] = (mp->z.neg) ? (int16_t)-s[2] : s[2];
}

/**
 *	SMULL 方法 + 移位
 *
 *	陀螺仪 (rad/s)
 *
 *	rad/s = raw / 131 * (PI / 180)
 *
 *	FS_SEL	        量程   灵敏度  Q31系数
 *	    0   +/-  250 dps    131     286112
 *	    1   +/-  500 dps   65.5     572224
 *	    2   +/- 1000 dps   32.8    1142703
 *	    3   +/- 2000 dps   16.4    2285406
 *
 * -------------------------------------------------
 *
 *	加速度计 (m/s^2)
 *
 *	m/s^2 = raw / 16384 * 9.80665
 *
 *	AFS_SEL	   量程   灵敏度  Q31系数
 *	   0   +/-  2g    16384   1285377
 *	   1   +/-  4g    8192    2570754
 *	   2   +/-  8g    4096    5141509
 *	   3   +/- 16g    2048    10283018
 *
 *	换算全式:raw 乘系数,乘 1000000,再 >>31,得微单位
 *	(urad/s,um/s^2) sensor_value_from_micro(按 10^-6 拆 val1/val2)
 */
#define GYRO_250_SCALE_Q31		(286112)
#define GYRO_500_SCALE_Q31		(572224)
#define GYRO_1000_SCALE_Q31		(1142703)
#define GYRO_2000_SCALE_Q31		(2285406)

#define ACCEL_2G_SCALE_Q31		(1285377)
#define ACCEL_4G_SCALE_Q31		(2570754)
#define ACCEL_8G_SCALE_Q31		(5141509)
#define ACCEL_16G_SCALE_Q31		(10283018)

static inline int raw_to_gyro_rad_s(int16_t raw, uint8_t gfs_sel, struct sensor_value *sv)
{
	static const int32_t scale[4] =
	{
		GYRO_250_SCALE_Q31, GYRO_500_SCALE_Q31,
		GYRO_1000_SCALE_Q31, GYRO_2000_SCALE_Q31,
	};

	if (3u < gfs_sel)
	{
		return -EINVAL;
	}

	micro_to_sensor_value((((int64_t)raw * scale[gfs_sel] * 1000000) >> 31), sv);

	return 0;
}

static inline int raw_to_accel_ms2(int16_t raw, uint8_t afs_sel, struct sensor_value *sv)
{
	static const int32_t scale[4] =
	{
		ACCEL_2G_SCALE_Q31, ACCEL_4G_SCALE_Q31,
		ACCEL_8G_SCALE_Q31, ACCEL_16G_SCALE_Q31,
	};

	if (3u < afs_sel)
	{
		return -EINVAL;
	}

	micro_to_sensor_value((((int64_t)raw * scale[afs_sel] * 1000000) >> 31), sv);

	return 0;
}

//	按照 sensor 泛型要求全部转换成 国际单位制 SI
static int mpu6050_channel_get(const struct sensor *s, enum sensor_chan sc, struct sensor_value *sv)
{
	const struct mpu6050_dev *m = s->ctx;
	const struct mpu6050_dat *dat = m->dat;
	int16_t a[3];
	int16_t g[3];
	int i, ret;

	for (i = 0; i < 3; i++)
	{
		a[i] = (int16_t)(dat->raw[i] + dat->offset[i]);
		g[i] = (int16_t)(dat->raw[4 + i] + dat->offset[3 + i]);
	}

	if (NULL != m->axis_map)
	{
		mpu6050_map_axes(a, m->axis_map);
		mpu6050_map_axes(g, m->axis_map);
	}

	switch (sc)
	{
	case SENSOR_CHAN_ACCEL_X:
		return raw_to_accel_ms2(a[0], m->afs_sel, sv);

	case SENSOR_CHAN_ACCEL_Y:
		return raw_to_accel_ms2(a[1], m->afs_sel, sv);

	case SENSOR_CHAN_ACCEL_Z:
		return raw_to_accel_ms2(a[2], m->afs_sel, sv);

	case SENSOR_CHAN_ACCEL_XYZ:
		for (i = 0, ret = 0; i < 3 && !ret; i++)
		{
			ret = raw_to_accel_ms2(a[i], m->afs_sel, sv + i);
		}
		return ret;

	case SENSOR_CHAN_GYRO_X:
		return raw_to_gyro_rad_s(g[0], m->gfs_sel, sv);

	case SENSOR_CHAN_GYRO_Y:
		return raw_to_gyro_rad_s(g[1], m->gfs_sel, sv);

	case SENSOR_CHAN_GYRO_Z:
		return raw_to_gyro_rad_s(g[2], m->gfs_sel, sv);

	case SENSOR_CHAN_GYRO_XYZ:
		for (i = 0, ret = 0; i < 3 && !ret; i++)
		{
			ret = raw_to_gyro_rad_s(g[i], m->gfs_sel, sv + i);
		}
		return ret;

	case SENSOR_CHAN_DIE_TEMP:
		micro_to_sensor_value(((int64_t)dat->raw[3] * 1000000 / 340 + 36530000), sv);
		return 0;

	default:
		return -EOPNOTSUPP;
	}
}

static int mpu6050_channel_get_raw(const struct sensor *s, enum sensor_chan sc, struct sensor_raw_value *srv)
{
	const struct mpu6050_dev *m = s->ctx;
	const struct mpu6050_dat *dat = m->dat;
	int16_t a[3];
	int16_t g[3];

	for (int i = 0; i < 3; i++)
	{
		a[i] = (int16_t)(dat->raw[i] + dat->offset[i]);
		g[i] = (int16_t)(dat->raw[4 + i] + dat->offset[3 + i]);
	}

	if (NULL != m->axis_map)
	{
		mpu6050_map_axes(a, m->axis_map);
		mpu6050_map_axes(g, m->axis_map);
	}

	switch (sc)
	{
	case SENSOR_CHAN_ACCEL_X:
		srv->type			= SENSOR_RAW_INT16;
		srv->data.i16		= a[0];
		return 0;

	case SENSOR_CHAN_ACCEL_Y:
		srv->type			= SENSOR_RAW_INT16;
		srv->data.i16		= a[1];
		return 0;

	case SENSOR_CHAN_ACCEL_Z:
		srv->type			= SENSOR_RAW_INT16;
		srv->data.i16		= a[2];
		return 0;

	case SENSOR_CHAN_ACCEL_XYZ:
		for (int i = 0; i < 3; i++, srv++)
		{
			srv->type		= SENSOR_RAW_INT16;
			srv->data.i16	= a[i];
		}
		return 0;

	case SENSOR_CHAN_GYRO_X:
		srv->type			= SENSOR_RAW_INT16;
		srv->data.i16		= g[0];
		return 0;

	case SENSOR_CHAN_GYRO_Y:
		srv->type			= SENSOR_RAW_INT16;
		srv->data.i16		= g[1];
		return 0;

	case SENSOR_CHAN_GYRO_Z:
		srv->type			= SENSOR_RAW_INT16;
		srv->data.i16		= g[2];
		return 0;

	case SENSOR_CHAN_GYRO_XYZ:
		for (int i = 0; i < 3; i++, srv++)
		{
			srv->type		= SENSOR_RAW_INT16;
			srv->data.i16	= g[i];
		}
		return 0;

	case SENSOR_CHAN_DIE_TEMP:
		srv->type			= SENSOR_RAW_INT16;
		srv->data.i16		= dat->raw[3];
		return 0;

	default:
		return -EOPNOTSUPP;
	}
}

#define	dat_of_cb(cb_p) \
	((struct mpu6050_dat *)(void *)((uint8_t *)(cb_p) - offsetof(struct mpu6050_dat, cb)))

static const struct sensor_trigger mpu_trig_drdy =
{
	.type	= SENSOR_TRIG_DATA_READY,
	.chan	= SENSOR_CHAN_ALL,
};

static void mpu_bounce(const struct pin_port *pt, struct pin_callback *cb)
{
	struct mpu6050_dat *dat = dat_of_cb(cb);

	(void)pt;
	dat->h(dat->s, &mpu_trig_drdy);
}

static int mpu6050_trigger_set(const struct sensor *s,
	const struct sensor_trigger *trig, sensor_trigger_handler_t handler)
{
	const struct mpu6050_dev *m = s->ctx;
	struct mpu6050_dat *dat = m->dat;
	int r;

	if ((NULL == m->int_pin) || (NULL == trig)
		|| (SENSOR_TRIG_DATA_READY != trig->type))
	{
		return -EOPNOTSUPP;
	}

	if (NULL == handler)
	{
		r = wr_reg(m->i2c, MPU_INT_ENABLE, 0x00u);
		if (0 != r)
		{
			return r;
		}

		pin_irq_configure(m->int_pin, PIN_INT_DISABLE);
		pin_remove_callback(m->int_pin->port, &dat->cb);
		dat->h = NULL;
		return 0;
	}

	pin_configure(m->int_pin);
	pin_remove_callback(m->int_pin->port, &dat->cb);
	dat->h = handler;
	dat->s = s;
	pin_callback_init(&dat->cb, mpu_bounce, m->int_pin->pin);
	pin_add_callback(m->int_pin->port, &dat->cb);

	r = pin_irq_configure(m->int_pin, PIN_INT_RISING);
	if (0 != r)
	{
		pin_remove_callback(m->int_pin->port, &dat->cb);
		dat->h = NULL;
		return r;
	}

	r = wr_reg(m->i2c, MPU_INT_PIN_CFG, 0x00u);
	if (0 != r)
	{
		pin_irq_configure(m->int_pin, PIN_INT_DISABLE);
		pin_remove_callback(m->int_pin->port, &dat->cb);
		dat->h = NULL;
		return r;
	}

	return wr_reg(m->i2c, MPU_INT_ENABLE, 0x01u);
}

static int mpu6050_attr_set_raw(const struct sensor *s, enum sensor_chan sc,
	enum sensor_attr attr, const struct sensor_raw_value *srv)
{
	const struct mpu6050_dev *m = s->ctx;
	struct mpu6050_dat *dat = m->dat;

	if (SENSOR_ATTR_OFFSET != attr)
	{
		return -EOPNOTSUPP;
	}

	switch (sc)
	{
	case SENSOR_CHAN_ACCEL_XYZ:
	case SENSOR_CHAN_GYRO_XYZ:
	{
		int16_t v[3];

		for (int i = 0; i < 3; i++)
		{
			if (SENSOR_RAW_INT16 != srv[i].type)
			{
				return -EINVAL;
			}

			v[i] = srv[i].data.i16;
		}

		if (NULL != m->axis_map)
		{
			mpu6050_unmap_axes(v, m->axis_map);
		}

		if (SENSOR_CHAN_GYRO_XYZ == sc)
		{
			for (int i = 0; i < 3; i++)
			{
				dat->offset[3 + i] = v[i];
			}
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				dat->offset[i] = v[i];
			}
		}

		return 0;
	}

	default:
		return -EOPNOTSUPP;
	}
}

const struct sensor_ops mpu6050_sensor_ops =
{
	.init				= mpu6050_init,
	.sample_fetch		= mpu6050_sample_fetch,
	.channel_get		= mpu6050_channel_get,
	.channel_get_raw	= mpu6050_channel_get_raw,
	.trigger_set		= mpu6050_trigger_set,
	.attr_set_raw		= mpu6050_attr_set_raw,
};

