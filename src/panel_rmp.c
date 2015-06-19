#include <string.h>

#include "systime.h"
#include "gpio.h"
#include "encoder.h"
#include "teensy.h"
#include "usb.h"
#include "panel_rmp.h"

uint32_t nav1_freq;

void task_panel_rmp(void) {
	static uint8_t init = 0;
	static uint8_t cnt = 0;

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

void panel_rmp_setup_datarefs(void) {
		teensy_register_dataref("sim/cockpit/electrical/strobe_lights_on", 1);
		teensy_register_dataref("sim/cockpit/electrical/nav_lights_on", 1);
		teensy_register_dataref("sim/cockpit2/radios/actuators/nav1_frequency_hz", 1);
}
