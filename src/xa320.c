#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>

#include "task.h"
#include "systime.h"
#include "gpio.h"
#include "encoder.h"
#include "usb.h"

void send_testdata(void);
void send_buttons(void);
void task_usb(void);
void task_encoder(void);

uint8_t buf[6] = {'E', 'n', 0, 0, 0, 0};
void send_testdata(void)
{
	char *p;
	static uint16_t cnt = 0;
	if (cnt++ > 10) {
		cnt = 0;
		p = encoder_read(ENC_A);
	//	uint8_t buf[4] = {0, 0, 0, 0};
		buf[0] = 0;
		buf[1] = p[0];
		buf[2] = p[1];
		buf[3] = p[2];
		buf[4] = p[3];
		buf[5] = '\n';
	}
	usb_send_packet(buf, 6);
}

void send_buttons(void)
{
	uint8_t buff[4] = {0, 0, 0, 0};
	buff[0] = 0;
	buff[1] = (!!gpio_get_switch()) << 7;
	buff[2] = (!!gpio_get_switch()) << 7; // AP - HDG Button
	buff[3] = 0;
	buff[1] = gpio_get_switch() ? 0xff : 0;
	buff[2] = gpio_get_switch() ? 0xff : 0;
	usb_send_packet(buff, 4);
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
}


void task_encoder(void) {
	encoder_task();
}

int main(void)
{
	rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOG);
	rcc_periph_clock_enable(RCC_OTGFS);

	systime_setup();

	gpio_setup();

	encoder_setup();

	usb_setup();

	task_create(task_usb, 10);
	task_create(task_encoder, 2);

	while (1) {
			// Simple Taskswitcher
			task_start();
			task_time_increment();
	}
}

