#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>

#include "gpio.h"
#include "usb.h"

void systick_setup(void);

uint32_t system_millis;

void systick_setup(void)
{
	/* clock rate / 1000 to get 1mS interrupt rate */
	systick_set_reload(168000);
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_counter_enable();
	/* this done last */
	systick_interrupt_enable();
}

int main(void)
{
	rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOG);
	rcc_periph_clock_enable(RCC_OTGFS);

	//systick_setup();

	gpio_setup();

	usb_setup();

	gpio_set_led(LED3, 1);

	int cnt = 0;
	while (1) {
		if (cnt++ > 100000) {
				cnt = 0;
				sys_tick_handler();
		}
	}
}

void sys_tick_handler(void)
{
	system_millis++;

	uint8_t buf[4] = {0, 0, 0, 0};
	static uint32_t cnt = 0;

	buf[1] = 'S';
	buf[2] = 't'; //cnt++;
	if (cnt > 100) {
		//gpio_toggle_led(LED4);
		cnt = 0;
	}
	usb_send_packet(buf, 4);
}

