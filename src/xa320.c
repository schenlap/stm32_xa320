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
#include "panel_rmp.h"

//void send_testdata(void);
//void send_buttons(void);
//void task_usb(void);
void task_encoder(void);
void task_display(void);




void task_encoder(void) {
	encoder_task();
}

void task_display(void) {
	max7219_ClearAll();
	char str[] = "1234.567890ABC";
	max7219_display_string(1, str);
}

int main(void)
{
	rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOG);
	rcc_periph_clock_enable(RCC_OTGFS);

	systime_setup();

	gpio_setup();

	max7219_setup(2);

	encoder_setup();

	usb_setup();

	task_create(task_encoder, 2);
	task_create(task_panel_rmp, 10);
	task_create(task_display, 100);

	while (1) {
			// Simple Taskswitcher
			task_start();
			task_time_increment();
	}
}

