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
#include "pal/sensor.h"
#include "dev/icm20948.h"

#define	ICM_WHO_AM_I				0x00u
#define	ICM_USER_CTRL				0x03u
#define	ICM_PWR_MGMT_1				0x06u
#define	ICM_ACCEL_XOUT_H			0x2Du
#define	ICM_REG_BANK_SEL			0x7Fu

#define	ICM_BANK_0					0x00u
#define	ICM_BANK_2					0x20u

#define	ICM_BANK2_GYRO_SMPLRT_DIV	0x00u
#define	ICM_BANK2_GYRO_CONFIG_1		0x01u
#define	ICM_BANK2_ACCEL_SMPLRT_DIV_2	0x11u
#define	ICM_BANK2_ACCEL_CONFIG		0x14u

#define	ICM_ID						0xEAu

struct icm20948_dat
{
	int16_t		raw[7];
	int16_t		offset[6];
};

_Static_assert(sizeof(struct icm20948_dat) == ICM20948_DAT_SIZE, "ICM20948_DAT_SIZE out of sync with struct icm20948_dat");

static int wr_reg(const struct i2c_dev *d, uint8_t reg, uint8_t val)
{
	uint8_t b[2] = {reg, val};

	return i2c_write(d, b, 2u);
}

static int rd_regs(const struct i2c_dev *d, uint8_t reg, uint8_t *buf, uint16_t n)
{
	return i2c_mem_read(d, &reg, 1u, buf, n);
}

static int icm20948_init(const struct sensor *s)
{
	const struct icm20948_dev *m = s->ctx;
	uint8_t id;
	int r;

	r = wr_reg(m->i2c, ICM_REG_BANK_SEL, ICM_BANK_0);
	if (0 != r)
	{
		return r;
	}

	r = rd_regs(m->i2c, ICM_WHO_AM_I, &id, 1u);
	if (0 != r)
	{
		return r;
	}

	if (ICM_ID != id)
	{
		return -ENODEV;
	}

	r = wr_reg(m->i2c, ICM_PWR_MGMT_1, 0x80u);
	if (0 != r)
	{
		return r;
	}

	delay_ms(100u);

	r = wr_reg(m->i2c, ICM_USER_CTRL, 0x00u);
	if (0 != r)
	{
		return r;
	}

	r = wr_reg(m->i2c, ICM_PWR_MGMT_1, 0x01u);
	if (0 != r)
	{
		return r;
	}

	r = wr_reg(m->i2c, ICM_REG_BANK_SEL, ICM_BANK_2);
	if (0 != r)
	{
		return r;
	}

	r = wr_reg(m->i2c, ICM_BANK2_GYRO_SMPLRT_DIV, m->gyro_smplrt_div);
	if (0 != r)
	{
		return r;
	}

	r = wr_reg(m->i2c, ICM_BANK2_GYRO_CONFIG_1,
		(uint8_t)((m->gfs_sel << 1) | 0x01u | (m->gyro_dlpf_cfg << 3)));
	if (0 != r)
	{
		return r;
	}

	r = wr_reg(m->i2c, ICM_BANK2_ACCEL_SMPLRT_DIV_2, m->accel_smplrt_div);
	if (0 != r)
	{
		return r;
	}

	r = wr_reg(m->i2c, ICM_BANK2_ACCEL_CONFIG,
		(uint8_t)((m->afs_sel << 1) | 0x01u | (m->accel_dlpf_cfg << 3)));
	if (0 != r)
	{
		return r;
	}

	return wr_reg(m->i2c, ICM_REG_BANK_SEL, ICM_BANK_0);
}

static int icm20948_sample_fetch(const struct sensor *s)
{
	const struct icm20948_dev *m = s->ctx;
	struct icm20948_dat *dat = m->dat;
	uint8_t b[14];
	int r = rd_regs(m->i2c, ICM_ACCEL_XOUT_H, b, sizeof(b));
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

#define	GYRO_250_SCALE_Q31		(286112)
#define	GYRO_500_SCALE_Q31		(572224)
#define	GYRO_1000_SCALE_Q31		(1142703)
#define	GYRO_2000_SCALE_Q31		(2285406)

#define	ACCEL_2G_SCALE_Q31		(1285377)
#define	ACCEL_4G_SCALE_Q31		(2570754)
#define	ACCEL_8G_SCALE_Q31		(5141509)
#define	ACCEL_16G_SCALE_Q31		(10283018)

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

static int icm20948_channel_get(const struct sensor *s, enum sensor_chan sc, struct sensor_value *sv)
{
	const struct icm20948_dev *m = s->ctx;
	const struct icm20948_dat *dat = m->dat;
	int16_t a[3];
	int16_t g[3];
	int i, ret;

	for (i = 0; i < 3; i++)
	{
		a[i] = (int16_t)(dat->raw[i] + dat->offset[i]);
		g[i] = (int16_t)(dat->raw[3 + i] + dat->offset[3 + i]);
	}

	if (NULL != m->axis_map)
	{
		sensor_imu_map_axes(a, m->axis_map);
		sensor_imu_map_axes(g, m->axis_map);
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
		micro_to_sensor_value(((int64_t)dat->raw[6] * 100000000 / 33387 + 21000000), sv);
		return 0;

	default:
		return -EOPNOTSUPP;
	}
}

static int icm20948_channel_get_raw(const struct sensor *s, enum sensor_chan sc, struct sensor_raw_value *srv)
{
	const struct icm20948_dev *m = s->ctx;
	const struct icm20948_dat *dat = m->dat;
	int16_t a[3];
	int16_t g[3];

	for (int i = 0; i < 3; i++)
	{
		a[i] = (int16_t)(dat->raw[i] + dat->offset[i]);
		g[i] = (int16_t)(dat->raw[3 + i] + dat->offset[3 + i]);
	}

	if (NULL != m->axis_map)
	{
		sensor_imu_map_axes(a, m->axis_map);
		sensor_imu_map_axes(g, m->axis_map);
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
		srv->data.i16		= dat->raw[6];
		return 0;

	default:
		return -EOPNOTSUPP;
	}
}

static int icm20948_attr_set_raw(const struct sensor *s, enum sensor_chan sc,
	enum sensor_attr attr, const struct sensor_raw_value *srv)
{
	const struct icm20948_dev *m = s->ctx;
	struct icm20948_dat *dat = m->dat;

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
			sensor_imu_unmap_axes(v, m->axis_map);
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

const struct sensor_ops icm20948_sensor_ops =
{
	.init				= icm20948_init,
	.sample_fetch		= icm20948_sample_fetch,
	.channel_get		= icm20948_channel_get,
	.channel_get_raw	= icm20948_channel_get_raw,
	.attr_set_raw		= icm20948_attr_set_raw,
};
