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
#define ID_NDB_FREQ        4
#define ID_NDB_STDBY_FREQ  5
#define ID_NAV2_FREQ       6
#define ID_NAV2_STDBY_FREQ 7

rmp_act_t rmp_act = RMP_ADF;

uint32_t nav1_freq = 11000;
uint32_t nav1_stdby_freq = 11100;

uint32_t ndb_freq = 255;
uint32_t ndb_stdby_freq = 255;

uint32_t nav2_freq = 11000;
uint32_t nav2_stdby_freq = 11100;

void panel_rmp_cb(uint8_t id, uint32_t data);
void panel_rmp_nav1(void);
void panel_rmp_ndb(void);
void panel_rmp_nav2(void);
uint8_t panel_rmp_switch(void);
void panel_rmp_general(uint32_t *stdby, uint32_t *act, uint8_t comma_value, uint32_t high_step, uint32_t low_step, uint32_t max, uint32_t min, uint32_t id_stdby, uint32_t id_act);

rmp_act_t panel_rmp_get_active(void) {
	return rmp_act;
}

uint8_t panel_rmp_switch(void) {
	rmp_act_t new = RMP_OFF;
	rmp_act_t last = rmp_act;

	if (gpio_get_pos_event(SWITCH_VOR))
		new = RMP_VOR;
	else if (gpio_get_pos_event(SWITCH_ILS))
		new = RMP_ILS;
	else if (gpio_get_pos_event(SWITCH_VOR2))
		new = RMP_VOR2;
	else if (gpio_get_pos_event(SWITCH_ADF))
		new = RMP_ADF;
	else if (gpio_get_pos_event(SWITCH_BFO))
		new = RMP_BFO;
	else
		return 0; // No switch pressed

	rmp_act = new;
	return new != last;
}

void task_panel_rmp(void) {
	static uint32_t init_cnt = 0;
	static uint32_t cnt = 0;

	panel_rmp_switch();

	if (!usb_ready)
		return;

	if (init_cnt && (init_cnt < 50)) {
		if (init_cnt == 48) {
			panel_rmp_setup_datarefs();
		}
		init_cnt++;
		return;
	}

	if ((!init_cnt) && (systime_get() - teensy_get_last_request_time() < 500)) {
		gpio_set_led(LED6,0);
		init_cnt = 1;
	}

	if (init_cnt && systime_get() - teensy_get_last_request_time() < 500) {
		gpio_set_led(LED6, 1);
	} else {
		if (cnt++ > 5) {
			gpio_toggle_led(LED6);
			cnt = 0;
		}
	}
	
	switch (rmp_act) {
			case RMP_OFF:
			break;
			case RMP_VOR:
				//panel_rmp_nav1();
				panel_rmp_general(&nav1_stdby_freq,
				                  &nav1_freq,
				                  100,
				                  100,
				                  5,
				                  11800,
				                  10800,
				                  ID_NAV1_STDBY_FREQ,
				                  ID_NAV1_FREQ);
			break;
			case RMP_ADF:
				panel_rmp_ndb();
			break;
			case RMP_VOR2:
				//panel_rmp_nav2();
				panel_rmp_general(&nav2_stdby_freq,
				                  &nav2_freq,
				                  100,
				                  100,
				                  5,
				                  11800,
				                  10800,
				                  ID_NAV2_STDBY_FREQ,
				                  ID_NAV2_FREQ);
			break;
			default:
			break;
	}
}

void panel_rmp_general(uint32_t *stdby, uint32_t *act, uint8_t comma_value, uint32_t high_step, uint32_t low_step, uint32_t max, uint32_t min, uint32_t id_stdby, uint32_t id_act) {
	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, 0);
	int16_t hv;
	int16_t tmp;

	if (enc_low) {
		hv = *stdby / comma_value;
		tmp = *stdby % comma_value;

		tmp += enc_low * low_step;	// 5kHz

		// lap to 0-995kHz range
		if (tmp < 0) tmp += comma_value - low_step;
		if (tmp >= comma_value) tmp = 0;

		*stdby = hv * 100 + tmp;
		if (*stdby > max) nav1_stdby_freq = max;

		teensy_send_int(id_stdby, *stdby);
	}
	if (enc_high) {
		*stdby += enc_high * high_step;	// 1MHz

		// lap to 108-118MHz range
		while (*stdby < min) *stdby += high_step;
		while (*stdby >= max) *stdby -= high_step;

		teensy_send_int(id_stdby, *stdby);
	}

	if (gpio_get_pos_event(SWITCH_SW_STBY)) {
		uint32_t t = *act;
		*act = *stdby;
		*stdby = t;
		teensy_send_int(id_act, *act);
		teensy_send_int(id_stdby, *stdby);
	}
}

void panel_rmp_nav1(void) {
	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, 0);
	int16_t hv;
	int16_t tmp;

	if (enc_low) {
		hv = nav1_stdby_freq / 100;
		tmp = nav1_stdby_freq % 100;

		tmp += enc_low * 5;	// 5kHz

		// lap to 0-995kHz range
		if (tmp < 0) tmp += 95;
		if (tmp > 95) tmp = 0;

		nav1_stdby_freq = hv * 100 + tmp;
		if (nav1_stdby_freq > 11800) nav1_stdby_freq = 11800;

		teensy_send_int(ID_NAV1_STDBY_FREQ, nav1_stdby_freq);
	}
	if (enc_high) {
		nav1_stdby_freq += enc_high * 100;	// 1MHz

		// lap to 108-118MHz range
		while (nav1_stdby_freq < 10800) nav1_stdby_freq += 1000;
		while (nav1_stdby_freq >= 11800) nav1_stdby_freq -= 1000;

		teensy_send_int(ID_NAV1_STDBY_FREQ, nav1_stdby_freq);
	}

	if (gpio_get_pos_event(SWITCH_SW_STBY)) {
		uint32_t t = nav1_freq;
		nav1_freq = nav1_stdby_freq;
		nav1_stdby_freq = t;
		teensy_send_int(ID_NAV1_FREQ, nav1_freq);
		teensy_send_int(ID_NAV1_STDBY_FREQ, nav1_stdby_freq);
	}
}

void panel_rmp_ndb(void) {
	uint8_t coarse = 0;
	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, &coarse);
	if (enc_low || enc_high) {
		if (enc_low) {
			if (coarse)
				ndb_stdby_freq += enc_low > 0 ? 10 : -10; // 10kHz
			else
				ndb_stdby_freq += enc_low;	// 1kHz

			if (ndb_stdby_freq > 525) ndb_stdby_freq = 525;
			if (ndb_stdby_freq < 200) ndb_stdby_freq = 200;
		}
		if (enc_high)
			ndb_stdby_freq += enc_high * 100;

		while (ndb_stdby_freq < 200) ndb_stdby_freq += 100;
		while (ndb_stdby_freq > 525) ndb_stdby_freq -= 100;

		teensy_send_int(ID_NDB_STDBY_FREQ, ndb_stdby_freq);
	}

	if (gpio_get_pos_event(SWITCH_SW_STBY)) {
		uint32_t t = ndb_freq;
		ndb_freq = ndb_stdby_freq;
		ndb_stdby_freq = t;
		teensy_send_int(ID_NDB_STDBY_FREQ, ndb_stdby_freq);
		teensy_send_int(ID_NDB_FREQ, ndb_freq);
	}
}

void panel_rmp_nav2(void) {
	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, 0);
	int16_t hv;
	int16_t tmp;

	if (enc_low) {
		hv = nav2_stdby_freq / 100;
		tmp = nav2_stdby_freq % 100;

		tmp += enc_low * 5;	// 5kHz

		// lap to 0-995kHz range
		if (tmp < 0) tmp += 95;
		if (tmp > 95) tmp = 0;

		nav2_stdby_freq = hv * 100 + tmp;
		if (nav2_stdby_freq > 11800) nav2_stdby_freq = 11800;

		teensy_send_int(ID_NAV2_STDBY_FREQ, nav2_stdby_freq);
	}
	if (enc_high) {
		nav2_stdby_freq += enc_high * 100;	// 1MHz

		// lap to 108-118MHz range
		while (nav2_stdby_freq < 10800) nav2_stdby_freq += 1000;
		while (nav2_stdby_freq >= 11800) nav2_stdby_freq -= 1000;

		teensy_send_int(ID_NAV2_STDBY_FREQ, nav2_stdby_freq);
	}

	if (gpio_get_pos_event(SWITCH_SW_STBY)) {
		uint32_t t = nav2_freq;
		nav2_freq = nav2_stdby_freq;
		nav2_stdby_freq = t;
		teensy_send_int(ID_NAV2_FREQ, nav2_freq);
		teensy_send_int(ID_NAV2_STDBY_FREQ, nav2_stdby_freq);
	}
}

uint32_t panel_rmp_get_nav1_freq(void) {
	return nav1_freq;
}

uint32_t panel_rmp_get_nav1_stdby_freq(void) {
	return nav1_stdby_freq;
}

uint32_t panel_rmp_get_ndb_freq(void) {
	return ndb_freq;
}

uint32_t panel_rmp_get_ndb_stdby_freq(void) {
	return ndb_stdby_freq;
}

uint32_t panel_rmp_get_nav2_freq(void) {
	return nav2_freq;
}

uint32_t panel_rmp_get_nav2_stdby_freq(void) {
	return nav2_stdby_freq;
}

void panel_rmp_setup_datarefs(void) {
		teensy_register_dataref(ID_STROBE_LIGHT, "sim/cockpit/electrical/strobe_lights_on", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV_LIGHT, "sim/cockpit/electrical/nav_lights_on", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_FREQ, "sim/cockpit2/radios/actuators/nav1_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_STDBY_FREQ, "sim/cockpit2/radios/actuators/nav1_standby_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NDB_FREQ, "sim/cockpit2/radios/actuators/adf1_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NDB_STDBY_FREQ, "sim/cockpit2/radios/actuators/adf1_standby_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV2_FREQ, "sim/cockpit2/radios/actuators/nav2_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV2_STDBY_FREQ, "sim/cockpit2/radios/actuators/nav2_standby_frequency_hz", 1, &panel_rmp_cb);
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
		case ID_NDB_FREQ:
				ndb_freq = data;
				break;
		case ID_NDB_STDBY_FREQ:
				ndb_stdby_freq = data;
				break;
		case ID_NAV2_FREQ:
				nav2_freq = data;
				break;
		case ID_NAV2_STDBY_FREQ:
				nav2_stdby_freq = data;
				break;
	}
}
