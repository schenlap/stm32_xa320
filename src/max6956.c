// https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f3/stm32f3-discovery/i2c/i2c.c
// https://searchcode.com/file/59129219/code/tmp.c
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/gpio.h>

#include "max6956.h"


void max6956_write_register(uint32_t i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t value)
{
	uint32_t reg32 __attribute__((unused));

	/* Send START condition. */
	i2c_send_start(i2c);

	/* Waiting for START is send and switched to master mode. */
	while (!((I2C_SR1(i2c) & I2C_SR1_SB)
		& (I2C_SR2(i2c) & (I2C_SR2_MSL | I2C_SR2_BUSY))));

	/* Send destination address. */
	i2c_send_7bit_address(i2c, dev_addr, I2C_WRITE);

	/* Waiting for address is transferred. */
	while (!(I2C_SR1(i2c) & I2C_SR1_ADDR));

	/* Cleaning ADDR condition sequence. */
	reg32 = I2C_SR2(i2c);

	/* Sending the data. */
	i2c_send_data(i2c, reg_addr);
	while (!(I2C_SR1(i2c) & I2C_SR1_BTF)); /* Await ByteTransferedFlag. */

	i2c_send_data(i2c, value);
	while (!(I2C_SR1(i2c) & (I2C_SR1_BTF | I2C_SR1_TxE)));

	/* Send STOP condition. */
	i2c_send_stop(i2c);
}


int32_t max6956_read_register(uint32_t i2c, uint8_t dev_addr, uint8_t reg_addr)
{
	uint8_t data;

	/* Send START condition and wait for completion */
	i2c_send_start(i2c);
	while (!(I2C_SR2(i2c) &  I2C_SR1_SB));

	/* Send destination address. */
	i2c_send_7bit_address(i2c, dev_addr, I2C_WRITE);
	while (!(I2C_SR1(i2c) & I2C_SR1_ADDR));

	/* Sending the data. */
	i2c_send_data(i2c, reg_addr);
	while (!(I2C_SR1(i2c) & I2C_SR1_BTF)); /* Await ByteTransferedFlag. */

	/* Send START condition and wait for completion */
	i2c_send_start(i2c);
	while (!(I2C_SR2(i2c) &  I2C_SR1_SB));

	/* Send destination address. */
	i2c_send_7bit_address(i2c, dev_addr, I2C_READ);
	while (!(I2C_SR1(i2c) & I2C_SR1_ADDR));

	if (I2C_SR1(i2c) & I2C_SR2_BUSY) {
		while ((I2C_SR1(i2c) & I2C_SR1_RxNE) == 0);
		data = i2c_get_data(i2c);
	} else {
		return -1;
	}
	/* Send STOP condition. */
	i2c_send_stop(i2c);

	return data;
}


void max6956_set_led_brightness(uint32_t i2c, uint8_t dev_adr, uint8_t nr, uint8_t brightness)
{
	int reg_adr;
	int high = 0;
	int32_t bright_tmp;

	if (nr < 4)
		return;
	if (nr > 31)
		return;

	nr = nr - 4;
	reg_adr = 0x12 + (nr >> 1);
	if (nr & 0x01)
		high = 1;

	brightness = brightness & 0x0f;

	bright_tmp = max6956_read_register(i2c, dev_adr, reg_adr);
	if (bright_tmp == -1)
		return;


	if (high) {
		brightness = brightness << 4;
		bright_tmp &= 0x0f;
	} else {
		bright_tmp &= 0xf0;
	}

	bright_tmp |= brightness;
	max6956_write_register(i2c, dev_adr, reg_adr, bright_tmp); 
}


void max6956_set_led(uint32_t i2c, uint8_t dev_adr, uint8_t nr, uint8_t on)
{
	if (nr < 4)
		return;
	if (nr > 31)
		return;

	max6956_write_register(i2c, dev_adr, 0x20 + nr, !!on);
}


void max6956_clear_led(uint32_t i2c, uint8_t dev_adr, uint8_t nr)
{
	max6956_set_led(i2c, dev_adr, nr, 0);
}


void max6956_standby(uint32_t i2c, uint8_t dev_adr, uint8_t stdby)
{
	max6956_write_register(i2c, dev_adr, 0x04, !stdby);
}


void max6956_clear_all_leds(uint32_t i2c, uint8_t dev_adr)
{
	// reset all outputs
	max6956_write_register(i2c, dev_adr, 0x44, 0x00); // p4-11 = off
	max6956_write_register(i2c, dev_adr, 0x4c, 0x00); // p12-19 = off
	max6956_write_register(i2c, dev_adr, 0x54, 0x00); // p20-27 = off
	max6956_write_register(i2c, dev_adr, 0x5c, 0x00); // p28-31 = off
}


void max6956_setup(void)
{
	int i;

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

	/* set brightness for all ports */
	for (i = 0x12; i <= 0x1f; i++)
		max6956_write_register(I2C2, 0x40, i, 0x88);

	max6956_write_register(I2C2, 0x40, 0x04, 0x41); // normal op, individual current (brightness)

	for (i = 0x09; i <= 0x0f; i++)
		max6956_write_register(I2C2, 0x40, i, 0x00); // led output

	
	max6956_clear_all_leds(I2C2, 0x40);

	//max6956_set_led_brightness(I2C2, 0x40, 4, 2); // Test Led4, low brighness
	//max6956_set_led(I2C2, 0x40, 21, 1); // Test Led21 on vhf1 low
	//max6956_set_led(I2C2, 0x40, 22, 1); // Test Led21 on vhf3 high bfo
	//max6956_set_led(I2C2, 0x40, 23, 1); // Test Led21 on hf2 low
	//max6956_set_led(I2C2, 0x40, 24, 1); // Test Led21 on am high
}
