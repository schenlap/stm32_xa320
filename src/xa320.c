#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>

#include "task.h"
#include "systime.h"
#include "gpio.h"
#include "max7219.h"
#include "encoder.h"
#include "usb.h"
#include "teensy.h"

void send_testdata(void);
void send_buttons(void);
void task_usb(void);
void task_encoder(void);
void task_xplane(void);

extern uint32_t nav1_freq;

void task_xplane(void) {
	static uint8_t init = 0;
	static uint8_t cnt = 0;

	if (!usb_ready)
		return;

	if (!init && systime_get() - usb_get_last_request_time() < 500) {
		init = 1;
		teensy_register_dataref("sim/cockpit/electrical/strobe_lights_on", 1);
		teensy_register_dataref("sim/cockpit/electrical/nav_lights_on", 1);
		teensy_register_dataref("sim/cockpit2/radios/actuators/nav1_frequency_hz", 1);
	}

	if (init && systime_get() - usb_get_last_request_time() < 500) {
		gpio_set_led(LED6, 1);
	} else {
		if (cnt++ > 5) {
			gpio_toggle_led(LED6);
			cnt = 0;
		}
	}

	uint8_t coarse = 0;
	int16_t enc = encoder_read(ENC_A, &coarse);
	if (enc) {
		if (coarse)
			nav1_freq += enc > 0 ? 100 : -100; // 1MHz
		else
			nav1_freq += enc * 5;	// 5kHz

		// lap to 108-118MHz range
		while (nav1_freq < 10800) nav1_freq += 1000;
		while (nav1_freq >= 11800) nav1_freq -= 1000;

		teensy_send_int(3, nav1_freq);
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

	max7219_setup();
//	max7219_DisplayChar (0, 'x');
/*	max7219_DisplayChar (7, '7');
	max7219_DisplayChar (6, '6');
	max7219_DisplayChar (5, '5');
	max7219_DisplayChar (4, '4');
	max7219_DisplayChar (3, '3');
	max7219_DisplayChar (2, '2');
	max7219_DisplayChar (1, '1');
	max7219_DisplayChar (0, '0');*/
	char str[] = "1234.56";
	max7219_display_string(1, str);

	encoder_setup();

	usb_setup();

	task_create(task_encoder, 2);
	task_create(task_xplane, 10);

	while (1) {
			// Simple Taskswitcher
			task_start();
			task_time_increment();
	}
}

