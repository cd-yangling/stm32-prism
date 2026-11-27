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
#include "pal/pin.h"
#include "pal/led.h"
#include "pal/uart.h"
#include "pal/sensor.h"
#include "pal/i2c.h"
#include "dev/voltage_divider.h"
#include "dev/icm20948.h"
#include "subsys/tty.h"
#include "dev/sensor/lc307.h"
#include "dev/sensor/stp23l.h"
#include "board.h"

#define	BOARD_C
#include "soc.h"

#if defined(CONFIG_LED)
PIN_DESC(led_1, gpiob, 14, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);
PIN_DESC(led_2, gpiob, 15, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);

static const struct led_dev led1 = { .ops = &led_gpio_ops, .ctx = &led_1, .name = "led1" };
static const struct led_dev led2 = { .ops = &led_gpio_ops, .ctx = &led_2, .name = "led2" };
#define	BOARD_LED1	(&led1)
#define	BOARD_LED2	(&led2)
#else
#define	BOARD_LED1	(NULL)
#define	BOARD_LED2	(NULL)
#endif

#if defined(CONFIG_BUZZER)
PIN_DESC(buzzer, gpioa, 4, PIN_OUT | PIN_SPEED_LOW | PIN_INIT_INACTIVE);
#define	BOARD_BUZZER	(&buzzer)
#else
#define	BOARD_BUZZER	(NULL)
#endif

#if defined(CONFIG_USART1)
PIN_DESC(usart1_tx, gpioa, 9,  PIN_ALT | PIN_MUX(7) | PIN_SPEED_VERY_HIGH);
PIN_DESC(usart1_rx, gpioa, 10, PIN_ALT | PIN_MUX(7));
static const struct uart_pins usart1_pins =
{
	.tx	= &usart1_tx,
	.rx	= &usart1_rx,
};
static const struct uart_config con_cfg =
{
	.baud = 256000,
	.data_bits = 8,
	.parity = 0,
	.stop_bits = 1,
};
static const struct uart_dev con_dev =
{
	.port	= &usart1,
	.cfg	= &con_cfg,
};

#if defined(CONFIG_USART1_IRQ) || defined(CONFIG_USART1_DMA)
static unsigned char con_rx_buf[256];
static unsigned char con_tx_buf[256];
#endif
#ifdef CONFIG_USART1_DMA
static unsigned char con_dma_tx_buf[96];
static unsigned char con_dma_rx_buf[64];
#endif

static struct tty con_tty =
{
	.dev		= &con_dev,
#if defined(CONFIG_USART1_DMA)
	.mode		= TTY_ASYNC,
#elif defined(CONFIG_USART1_IRQ)
	.mode		= TTY_IRQ,
#else
	.mode		= TTY_POLL,
#endif
#if defined(CONFIG_USART1_IRQ) || defined(CONFIG_USART1_DMA)
	.rx_buf		= con_rx_buf,
	.rx_len		= sizeof(con_rx_buf),
	.tx_buf		= con_tx_buf,
	.tx_len		= sizeof(con_tx_buf),
#endif
#ifdef CONFIG_USART1_DMA
	.dma_tx_buf	= con_dma_tx_buf,
	.dma_tx_len	= sizeof(con_dma_tx_buf),
	.dma_rx_buf	= con_dma_rx_buf,
	.dma_rx_len	= sizeof(con_dma_rx_buf),
#endif
};
#define	BOARD_CONSOLE	(&con_tty)
#else
#define	BOARD_CONSOLE	(NULL)
#endif

#if defined(CONFIG_USART3)
PIN_DESC(usart3_tx, gpiob, 10, PIN_ALT | PIN_MUX(7) | PIN_SPEED_VERY_HIGH);
PIN_DESC(usart3_rx, gpiob, 11, PIN_ALT | PIN_MUX(7));
static const struct uart_pins usart3_pins =
{
	.tx			= &usart3_tx,
	.rx			= &usart3_rx,
};
static const struct uart_config oflow_cfg =
{
	.baud		= 19200,
	.data_bits	= 8,
	.parity		= 0,
	.stop_bits	= 1,
};
static const struct uart_dev oflow_dev =
{
	.port		= &usart3,
	.cfg		= &oflow_cfg,
};

static const unsigned char oflow_head[] = {0xFE, 0x0A};
static const struct deframer_cfg oflow_dfr_cfg =
{
	.head		= oflow_head,
	.head_len	= 2u,
	.frame_len	= 14u,
	.ck_kind	= DEFRAMER_CK_XOR,
	.ck_init	= 0u,
	.ck_off		= 2u,
	.ck_pos		= 12u,
	.has_tail	= 1u,
	.tail_val	= 0x55u,
};
static struct lc307 oflow_sen =
{
	.io			=
	{
		.dev	= &oflow_dev,
		.dfr_cfg= &oflow_dfr_cfg,
#if defined(CONFIG_USART3_DMA) || defined(CONFIG_USART3_DMA_RX)
		.mode	= STRM_DEFRAMER_ASYNC,
#elif defined(CONFIG_USART3_IRQ)
		.mode	= STRM_DEFRAMER_IRQ,
#else
		.mode	= STRM_DEFRAMER_POLL,
#endif
	},
};
#define	BOARD_OFLOW	(&oflow_sen)
#else
#define	BOARD_OFLOW	(NULL)
#endif

#if defined(CONFIG_UART5)
PIN_DESC(uart5_tx, gpioc, 12, PIN_ALT | PIN_MUX(8) | PIN_SPEED_VERY_HIGH);
PIN_DESC(uart5_rx, gpiod, 2, PIN_ALT | PIN_MUX(8));
static const struct uart_pins uart5_pins =
{
	.tx			= &uart5_tx,
	.rx			= &uart5_rx,
};
static const struct uart_config laser_cfg =
{
	.baud		= 230400,
	.data_bits	= 8,
	.parity		= 0,
	.stop_bits	= 1,
};
static const struct uart_dev laser_dev =
{
	.port		= &uart5,
	.cfg		= &laser_cfg,
};

static const unsigned char laser_head[] =
{
	0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x02, 0x00, 0x00, 0xB8, 0x00,
};
static const struct deframer_cfg laser_dfr_cfg =
{
	.head		= laser_head,
	.head_len	= 10u,
	.frame_len	= 195u,
	.ck_kind	= DEFRAMER_CK_SUM8,
	.ck_init	= 0u,
	.ck_off		= 4u,
	.ck_pos		= 194u,
	.has_tail	= 0u,
	.tail_val	= 0u,
};
static struct stp23l laser_sen =
{
	.io			=
	{
		.dev	= &laser_dev,
		.dfr_cfg= &laser_dfr_cfg,
#if defined(CONFIG_UART5_DMA) || defined(CONFIG_UART5_DMA_RX)
		.mode	= STRM_DEFRAMER_ASYNC,
#elif defined(CONFIG_UART5_IRQ)
		.mode	= STRM_DEFRAMER_IRQ,
#else
		.mode	= STRM_DEFRAMER_POLL,
#endif
	},
};
#define	BOARD_LASER	(&laser_sen)
#else
#define	BOARD_LASER	(NULL)
#endif

static void brd_uart_init(void)
{
#if defined(CONFIG_USART1)
	uart_dev_init(&con_dev, &usart1_pins);
#endif
#if defined(CONFIG_USART3)
	uart_dev_init(&oflow_dev, &usart3_pins);
#endif
#if defined(CONFIG_UART5)
	uart_dev_init(&laser_dev, &uart5_pins);
#endif
}

#if defined(CONFIG_I2C1)
PIN_DESC(i2c1_scl, gpiob, 6, PIN_ALT | PIN_MUX(4) | PIN_OD | PIN_SPEED_VERY_HIGH);
PIN_DESC(i2c1_sda, gpiob, 7, PIN_ALT | PIN_MUX(4) | PIN_OD | PIN_SPEED_VERY_HIGH);
static const struct i2c_pins i2c1_pins =
{
	.scl	= &i2c1_scl,
	.sda	= &i2c1_sda,
};
#endif

static void brd_i2c_init(void)
{
#if defined(CONFIG_I2C1)
	i2c_bus_init(&i2c1, &i2c1_pins);
#endif
}

#if defined(CONFIG_VOLTAGE_DIVIDER)
VOLTAGE_DIVIDER_DAT(vdiv_dat);
static const struct voltage_divider_dev vdiv_dev =
{
	.adc			= &adc1,
	.ch				= 7u,
	.full_ohms		= 11000u,
	.output_ohms	= 1000u,
	.dat			= vdiv_dat,
};
static const struct sensor vbat_sen =
{
	.ops	= &voltage_divider_sensor_ops,
	.ctx	= &vdiv_dev,
	.name	= "vbat_sen",
};
#define	BOARD_VBAT_SEN	(&vbat_sen)
#else
#define	BOARD_VBAT_SEN	(NULL)
#endif

#if defined(CONFIG_ICM20948)
ICM20948_DAT(imu1_dat);
static const struct i2c_dev imu1 =
{
	.bus	= &i2c1,
	.addr7	= 0x68u,
	.spd_hz	= 400000u,
};
static const struct sensor_imu_axis_map imu1_axis_map = SENSOR_IMU_AXIS_MAP_XRIGHT_YFRONT_ZUP_INIT;
#ifdef CONFIG_ICM20948_MAGN
static const struct sensor_imu_axis_map imu1_mag_axis_map =
{
	.x	= { .src = SENSOR_IMU_AXIS_Y, .neg = 0 },
	.y	= { .src = SENSOR_IMU_AXIS_X, .neg = 0 },
	.z	= { .src = SENSOR_IMU_AXIS_Z, .neg = 0 },
};
#endif
static const struct icm20948_dev imu1_dev =
{
	.i2c				= &imu1,
	.gyro_smplrt_div	= 4u,
	.accel_smplrt_div	= 4u,
	.gyro_dlpf_cfg		= 3u,
	.accel_dlpf_cfg		= 5u,
	.gfs_sel			= 3u,
	.afs_sel			= 0u,
	.dat				= imu1_dat,
	.axis_map			= &imu1_axis_map,
#ifdef CONFIG_ICM20948_MAGN
	.mag_mode			= 0x08u,
	.mag_axis_map		= &imu1_mag_axis_map,
#endif
};
static const struct sensor imu_sen =
{
	.ops	= &icm20948_sensor_ops,
	.ctx	= &imu1_dev,
	.name	= "imu_sen",
};
#define	BOARD_IMU	(&imu_sen)
#else
#define	BOARD_IMU	(NULL)
#endif

static void brd_led_init(void)
{
#if defined(CONFIG_LED)
	led_init(&led1);
	led_init(&led2);
#endif
}

const struct board board =
{
	.led1		= BOARD_LED1,
	.led2		= BOARD_LED2,
	.buzzer		= BOARD_BUZZER,
	.vbat_sen	= BOARD_VBAT_SEN,
	.imu		= BOARD_IMU,
	.console	= BOARD_CONSOLE,
	.oflow		= BOARD_OFLOW,
	.laser		= BOARD_LASER,
};

void brd_init(void)
{
	brd_led_init();
	brd_uart_init();
	brd_i2c_init();
}
