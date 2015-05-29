#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>

#include "task.h"
#include "systime.h"
#include "gpio.h"
#include "usb.h"

void send_testdata(void);
void task_usb(void);

void send_testdata(void)
{
	uint8_t buf[4] = {0, 0, 0, 0};
	buf[0] = 'S';
	buf[1] = 't';
	buf[2] = 'e';
	usb_send_packet(buf, 4);
}

void task_usb(void) {
	static uint8_t cnt = 0;

	if (usb_ready) {
		send_testdata();
		gpio_set_led(LED6, 1);
	} else {
		if (cnt++ > 5) {
			gpio_toggle_led(LED6);
			cnt = 0;
		}
	}

	gpio_set_led(LED5, gpio_get_switch());
}

int main(void)
{
	rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOG);
	rcc_periph_clock_enable(RCC_OTGFS);

	systime_setup();

	gpio_setup();

	usb_setup();

	gpio_set_led(LED5, 1);

	task_create(task_usb, 100);

	while (1) {
			// Simple Taskswitcher
			task_start();
			task_time_increment();
			usb_poll();
	}
}

