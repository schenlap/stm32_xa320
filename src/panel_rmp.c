#include <string.h>

#include "systime.h"
#include "gpio.h"
#include "encoder.h"
#include "teensy.h"
#include "usb.h"
#include "panel_rmp.h"

#define ID_STROBE_LIGHT    0
#define ID_NAV_LIGHT       1
#define ID_NAV1_FREQ       2
#define ID_NAV1_STDBY_FREQ 3

uint32_t nav1_freq = 11000;
uint32_t nav1_stdby_freq = 11100;

void panel_rmp_cb(uint8_t id, uint32_t data);
void panel_rmp_nav1(void);

void task_panel_rmp(void) {
	static uint8_t init = 0;
	static uint8_t cnt = 0;

	panel_rmp_nav1();

	if (!usb_ready)
		return;

	if (!init && systime_get() - teensy_get_last_request_time() < 500) {
		init = 1;
		panel_rmp_setup_datarefs();
	}

	if (init && systime_get() - teensy_get_last_request_time() < 500) {
		gpio_set_led(LED6, 1);
	} else {
		if (cnt++ > 5) {
			gpio_toggle_led(LED6);
			cnt = 0;
		}
	}

	
}

void panel_rmp_nav1(void) {
	uint8_t coarse = 0;
	int16_t enc = encoder_read(ENC_A, &coarse);
	if (enc) {
		if (coarse)
			nav1_stdby_freq += enc > 0 ? 100 : -100; // 1MHz
		else
			nav1_stdby_freq += enc * 5;	// 5kHz

		// lap to 108-118MHz range
		while (nav1_stdby_freq < 10800) nav1_stdby_freq += 1000;
		while (nav1_stdby_freq >= 11800) nav1_stdby_freq -= 1000;

		teensy_send_int(3, nav1_stdby_freq);
	}

	if (gpio_get_pos_event(SWITCH1)) {
		uint32_t t = nav1_freq;
		nav1_freq = nav1_stdby_freq;
		nav1_stdby_freq = t;
	}
}

uint32_t panel_rmp_get_nav1_freq(void) {
	return nav1_freq;
}

uint32_t panel_rmp_get_nav1_stdby_freq(void) {
	return nav1_stdby_freq;
}

void panel_rmp_setup_datarefs(void) {
		teensy_register_dataref(ID_STROBE_LIGHT, "sim/cockpit/electrical/strobe_lights_on", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV_LIGHT, "sim/cockpit/electrical/nav_lights_on", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_FREQ, "sim/cockpit2/radios/actuators/nav1_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_STDBY_FREQ, "sim/cockpit2/radios/actuators/nav1_standby_frequency_hz", 1, &panel_rmp_cb);
}

void panel_rmp_cb(uint8_t id, uint32_t data) {
	switch(id) {
		case ID_STROBE_LIGHT:
				gpio_set_led(LED5, !!data);
				break;
		case ID_NAV_LIGHT:
				gpio_set_led(LED4, !!data);
				break;
		case ID_NAV1_FREQ:
				nav1_freq = data;
				break;
		case ID_NAV1_STDBY_FREQ:
				nav1_stdby_freq = data;
				break;
	}
}
