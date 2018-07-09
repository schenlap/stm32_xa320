#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>

#include "task.h"
#include "systime.h"
#include "gpio.h"
#include "max7219.h"
#include "encoder.h"
#include "led.h"
#include "max6956.h" // max6956_setup()
#include "usb.h"
#include "servo.h"
#include "teensy.h"
#include "panel_rmp.h"
#include "panel_fis.h"
#include "xa320.h"

void task_encoder(void);

void task_switches(void);
void task_xplane_detect(void);

int xa320_setup_datarefs_connect(void);

void servo_display_setup(void);

static uint32_t cnt = 0;
uint8_t config_panel = 0;
static uint32_t xplane_ready = 0;
static uint8_t is_init = 0;

void task_encoder(void) {
	encoder_task();
}


/* try to register dataref to see ix X-Plane is started and ready
 * return: 0 .. ok, <0 .. error (X-PLane not ready)
 */
int xa320_setup_datarefs_connect(void) {
		int ret;

		ret = teensy_register_dataref(ID_AIRCRAFT_TYPE, "sim/aircraft/view/acf_ICAO", 1, &panel_rmp_connect_cb);
		return ret;
}


void task_switches(void) {
	uint32_t on;
	static uint32_t on_last = 99;
	on = !gpio_get_state(SWITCH_RMP_OFF);
	panel_rmp_set_avionics_power(on);
	if (on != on_last) {
		on_last = on;
		led_standby(!on);
	}
}


void task_xplane_detect(void) {
	if (!usb_ready)
		return;
	gpio_set_led(LED6, 0);
	gpio_set_led(LED_ORANGE, 1);

	if (!xplane_ready) {
		xa320_setup_datarefs_connect(); // fill buffer
		if (!xplane_ready && xa320_setup_datarefs_connect() == 0) {
				xplane_ready = 1;
		}
	}

	if (!xplane_ready) {
		gpio_set_led(LED5, 0);
			return;
	}
	gpio_set_led(LED5, 1);
	gpio_set_led(LED_RED, 1);

	if (!is_init) {
		uint32_t timeout;
		/* wait 100ms until teensy plugin is ready */
		timeout = systime_get() + 100;
		while(systime_get() < timeout)
			;
		if (config_panel) {
			panel_rmp_setup_datarefs();
			gpio_set_led(LED_GREEN, 1);
		} else {
			panel_fis_setup_datarefs();
			gpio_set_led(LED_BLUE, 1);
		}
		is_init = 1;
	}

	if (systime_get() - teensy_get_last_request_time() < 1500) {
		//gpio_set_led(LED6, 1);
	} else {
		if (cnt++ > 5) {
			/* connection lost, try to reconnect */
			xplane_ready = 0;
			is_init = 0;
			cnt = 0;
		}
	}
}


uint8_t xa320_datarefs_ready() {
	return is_init;
}


uint8_t xa320_xplane_ready() {
	return xplane_ready;
}

int main(void)
{
	#if 0
	int debug = 0;
	while (!debug)
			;
	#endif
	rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOG);
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_OTGFS);

	systime_setup();

	gpio_setup();

	config_panel = gpio_get_state_direct(SWITCH_CONFIG_PANEL);

	if (config_panel) {
		max7219_setup(2);
	}

	encoder_setup();

	usb_setup();

	gpio_set_led(LED6, 0);


	if (config_panel) {
		max6956_setup();
		panel_rmp_setup();
	} else {
		gpio_set_led(LED_GREEN, 0);
		gpio_set_led(LED_ORANGE, 0);
		panel_fis_setup();
		servo_setup();
		gpio_set_led(LED_RED, 0);
		servo_display_setup();
		gpio_set_led(LED_BLUE, 0);
	}

	task_create(task_xplane_detect, 50);
	if (config_panel) {
		task_create(task_panel_rmp, 10);
		task_create(task_display, 100);
		task_create(task_switches, 100);
	} else {
		task_create(task_panel_fis, 10);
	}
	task_create(gpio_task, 50);

	while (1) {
		// Simple Taskswitcher
		task_start();
		task_time_increment();
	}
}

