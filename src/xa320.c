#include <stdlib.h>
#include <stdio.h>
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

void task_encoder(void);
void task_display(void);


uint8_t disp_in_newdata(uint32_t freq, uint32_t freq_stdby, rmp_act_t act);


void task_encoder(void) {
	encoder_task();
}

uint8_t disp_in_newdata(uint32_t freq, uint32_t freq_stdby, rmp_act_t act) {
	static uint32_t freq_last;
	static uint32_t freq_stdby_last;
	static rmp_act_t act_last;
	uint8_t new = 0;

	if (freq != freq_last)
		new = 1;
	else if (freq_stdby != freq_stdby_last)
		new = 1;
	else if (act != act_last)
		new = 1;

	freq_last = freq;
	freq_stdby_last = freq_stdby;
	act_last = act;

	return new;
}

void task_display(void) {
	char str[8];
	rmp_act_t act =  panel_rmp_get_active();
	uint32_t freq, freq_stdby;

	switch(act) {
		case RMP_VOR:
			freq = panel_rmp_get_nav1_freq();
			freq_stdby = panel_rmp_get_nav1_stdby_freq();
			if (!disp_in_newdata(freq, freq_stdby, act))
				break;
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 3);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 3);
			max7219_display_string(0, "V1");
		break;
		case RMP_VOR_CRS:
			freq = panel_rmp_get_nav1_crs();
			if (!disp_in_newdata(freq, 0, act))
				break;
			max7219_display_string_fixpoint(3, "     ", 99);
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(8, str, 99);
			max7219_display_string(0, "V1");
		break;
		case RMP_ADF:
			freq = panel_rmp_get_ndb_freq();
			freq_stdby = panel_rmp_get_ndb_stdby_freq();
			if (!disp_in_newdata(freq, freq_stdby, act))
				break;
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 99);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 99);
			max7219_display_string(0, "AD");
		break;
		case RMP_VOR2:
			freq = panel_rmp_get_nav2_freq();
			freq_stdby = panel_rmp_get_nav2_stdby_freq();
			if (!disp_in_newdata(freq, freq_stdby, act))
				break;
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 3);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 3);
			max7219_display_string(0, "V2");
		break;
		case RMP_VOR2_CRS:
			freq = panel_rmp_get_nav2_crs();
			if (!disp_in_newdata(freq, 0, act))
				break;
			max7219_display_string_fixpoint(3, "     ", 99);
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(8, str, 99);
			max7219_display_string(0, "V2");
		break;
		case RMP_COM1:
			freq = panel_rmp_get_com1_freq();
			freq_stdby = panel_rmp_get_com1_stdby_freq();
			if (!disp_in_newdata(freq, freq_stdby, act))
				break;
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 3);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 3);
			max7219_display_string(0, "C1");
		break;
		case RMP_COM2:
			freq = panel_rmp_get_com2_freq();
			freq_stdby = panel_rmp_get_com2_stdby_freq();
			if (!disp_in_newdata(freq, freq_stdby, act))
				break;
			snprintf(str, 8, "%5d", (int)freq);
			max7219_display_string_fixpoint(3, str, 3);
			snprintf(str, 8, "%5d", (int)freq_stdby);
			max7219_display_string_fixpoint(8, str, 3);
			max7219_display_string(0, "C2");
		break;
		default:
			if (!disp_in_newdata(0, 0, act))
				break;
			snprintf(str, 8, "%5d", (int)0);
			max7219_display_string_fixpoint(3, str, 3);
			snprintf(str, 8, "%5d", (int)0);
			max7219_display_string_fixpoint(8, str, 3);
			max7219_display_string(0, "--");
		break;
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

	max7219_setup(2);

	encoder_setup();

	usb_setup();

	gpio_set_led(LED6, 0);

	//task_create(task_encoder, 2);
	task_create(task_panel_rmp, 10);
	task_create(task_display, 100);
	task_create(gpio_task, 50);

	while (1) {
		// Simple Taskswitcher
		task_start();
		task_time_increment();
	}
}

