#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>

#include "systime.h"
#include "gpio.h"
#include "usb.h"

void send_testdata(void);

void send_testdata(void)
{
	uint8_t buf[4] = {0, 0, 0, 0};
	static uint32_t cnt = 0;

	buf[1] = 'S';
	buf[2] = 't'; //cnt++;
	if (cnt++ > 100000) {
		cnt = 0;
		usb_send_packet(buf, 4);
	}
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

	long long cnt = 0;

	while (1) {
		if (cnt++ > 1680000) {
				cnt = 0;
				gpio_toggle_led(LED6);
		}
		if (usb_ready) {
			send_testdata();
			gpio_set_led(LED4, 1);
		}
		gpio_set_led(LED5, gpio_get_switch());
	}

}

