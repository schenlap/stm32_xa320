// https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f3/stm32f3-discovery/i2c/i2c.c
// https://searchcode.com/file/59129219/code/tmp.c
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/gpio.h>

#include "max6956.h"

void max6956_setup(void)
{
	rcc_periph_clock_enable(RCC_I2C2);
	rcc_periph_clock_enable(RCC_GPIOB);
	//rcc_set_i2c_clock_hsi(I2C2);

	i2c_reset(I2C2);

	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO10 | GPIO11);
	gpio_set_af(GPIOB, GPIO_AF4, GPIO10| GPIO11);

	i2c_peripheral_disable(I2C2);
	i2c_set_clock_frequency( I2C2, I2C_CR2_FREQ_36MHZ);
	i2c_set_standard_mode(I2C2); // up to 100kHz
	i2c_set_ccr(I2C2, 1600);
	i2c_set_trise(I2C2, 0x10);
	//addressing mode
	i2c_peripheral_enable(I2C2);
}
