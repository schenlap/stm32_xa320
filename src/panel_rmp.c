#include <string.h>

#include "systime.h"
#include "gpio.h"
#include "encoder.h"
#include "teensy.h"
#include "usb.h"
#include "led.h"
#include "panel_rmp.h"

#define ID_AIRCRAFT_TYPE   0
#define ID_STROBE_LIGHT    1
#define ID_NAV_LIGHT       2
#define ID_NAV1_FREQ       3
#define ID_NAV1_STDBY_FREQ 4
#define ID_NAV1_CRS        5
#define ID_NDB_FREQ        6
#define ID_NDB_STDBY_FREQ  7
#define ID_NAV2_FREQ       8
#define ID_NAV2_STDBY_FREQ 9
#define ID_NAV2_CRS        10
#define ID_COM1_FREQ       11
#define ID_COM1_STDBY_FREQ 12
#define ID_COM2_FREQ       13
#define ID_COM2_STDBY_FREQ 14
#define ID_AVIONICS_POWER  15
#define ID_AUTOP_HEADING   16
#define ID_AUTOP_ALT       17
#define ID_COM1_LARGE_UP   18
#define ID_COM1_LARGE_DOWN 19
#define ID_COM1_SMALL_UP   20
#define ID_COM1_SMALL_DOWN 21
#define ID_AIRCRAFT_AIRSPEED 22
#define ID_AIRCRAFT_VARIO 23
#define ID_AIRCRAFT_COURSE 24

static uint8_t is_init = 0;
rmp_act_t rmp_act = RMP_VOR;

uint32_t nav1_freq = 11000;
uint32_t nav1_stdby_freq = 11100;
int32_t nav1_crs = 90;

uint32_t ndb_freq = 255;
uint32_t ndb_stdby_freq = 255;

uint32_t nav2_freq = 11000;
uint32_t nav2_stdby_freq = 11100;
int32_t nav2_crs = 90;

uint32_t com1_freq10 = 121700; // * 10 um 0.02, 0.05, 0.07, 0.10 darzustellen
uint32_t com1_stdby_freq10 = 121700;

uint32_t com2_freq10 = 121700; // * 10
uint32_t com2_stdby_freq10 = 121700;

uint32_t avionics_power = 99; // must be != 0|1 to be sent on startup

int32_t autop_heading = 90;
int32_t autop_alt = 2000;

int32_t airspeed = 999;
int32_t variometer = -999;
uint32_t course = 360;

void panel_rmp_cb(uint8_t id, uint32_t data);
void panel_rmp_connect_cb(uint8_t id, uint32_t data);
void panel_rmp_ndb(void);
uint8_t panel_rmp_switch(void);
void panel_rmp_general(uint32_t *stdby, uint32_t *act, uint16_t comma_value, uint32_t high_step, uint32_t low_step, uint32_t max, uint32_t min, uint32_t id_stdby, uint32_t id_act);
void panel_rmp_general_single_float(int32_t *act, int32_t high_step, int32_t low_step, int32_t max, int32_t min, uint32_t id_act);
void panel_send_dial_commands(uint32_t large_up, uint32_t large_down, uint32_t small_up, uint32_t small_down);
void panel_set_led(void);

uint8_t panel_get_associated_led(uint8_t page);
int panel_rmp_setup_datarefs_connect(void);

rmp_act_t panel_rmp_get_active(void) {
	return rmp_act;
}


void panel_set_led(void) {
		switch(rmp_act) {
		case RMP_VOR:
		case RMP_VOR_CRS:
				led_set(LED_VOR, 1);
				break;
		case RMP_VOR2:
		case RMP_VOR2_CRS:
		case RMP_ILS:
				led_set(LED_ILS, 1);
				break;
		case RMP_ADF:
				led_set(LED_ADF, 1);
				break;
		case RMP_BFO:
		case RMP_BFO_ALT:
				led_set(LED_BFO, 1);
				break;
		case RMP_COM1:
				led_set(LED_HF1, 1);
				break;
		case RMP_COM2:
				led_set(LED_HF2, 1);
				break;
		case RMP_VHF1:
				led_set(LED_VHF1, 1);
				break;
		default:
				;
		}
}


uint8_t panel_get_associated_led(uint8_t page) {
	switch(page) {
			case RMP_COM1:
					return LED_HF1;
			case RMP_COM2:
					return LED_HF2;
			case RMP_VOR:
			case RMP_VOR_CRS:
					return LED_VOR;
			case RMP_ILS:
			case RMP_VOR2: /* MLS */
			case RMP_VOR2_CRS: /* MLS */
					return LED_ILS;
			case RMP_ADF:
					return LED_ADF;
			case RMP_BFO: /* Autopilot heading */
			case RMP_BFO_ALT: /* Autopilot height */
					return LED_BFO;
			case RMP_VHF1: /* Airplane speed, course and varia */
					return LED_VHF1;
			default:
					return -1;
	}
}


uint8_t panel_rmp_switch(void) {
	rmp_act_t new = RMP_OFF;
	rmp_act_t last = rmp_act;
	uint8_t lednr;

	if (gpio_get_pos_event(SWITCH_VOR)) {
		if (last == RMP_VOR)
			new = RMP_VOR_CRS;
		else
			new = RMP_VOR;
	/*} else if (gpio_get_pos_event(SWITCH_ILS)) {
		new = RMP_ILS; */
	} else if (/*gpio_get_pos_event(SWITCH_VOR2) ||*/ gpio_get_pos_event(SWITCH_ILS)) { // VOR2 = MLS is short circuit to ADF
		if (last == RMP_VOR2)
			new = RMP_VOR2_CRS;
		else
			new = RMP_VOR2;
	} else if (gpio_get_pos_event(SWITCH_ADF)) {
		new = RMP_ADF;
	} else if (gpio_get_pos_event(SWITCH_BFO) ||
			  ((last == RMP_BFO || last == RMP_BFO_ALT) && gpio_get_pos_event(SWITCH_SW_STBY))) {
		if (last == RMP_BFO)
			new = RMP_BFO_ALT;
		else
			new = RMP_BFO;
	} else if (gpio_get_pos_event(SWITCH_COM1)) {
		new = RMP_COM1;
	} else if (gpio_get_pos_event(SWITCH_COM2)) {
		new = RMP_COM2;
	} else if (gpio_get_pos_event(SWITCH_NAV)) {
			led_set(LED_SEL, 1);
			return 0; // nothing to do here
	} else if (gpio_get_pos_event(SWITCH_VHF1)) { 
		new = RMP_VHF1;
	} else {
		return 0; // No switch pressed
	}

	rmp_act = new;
	lednr = panel_get_associated_led(last);

	if (new != last) {
		if (lednr > 0)
				led_clear(lednr);
		panel_set_led();
	}

	return new != last;
}

void task_panel_rmp(void) {
	static uint32_t cnt = 0;
	static uint32_t xplane_ready = 0;

	panel_rmp_switch();

	switch (rmp_act) {
			case RMP_OFF:
			break;
			case RMP_VOR:
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
			case RMP_VOR_CRS:
				panel_rmp_general_single_float(&nav1_crs,
				                  30,
				                  1,
				                  359,
				                  0,
				                  ID_NAV1_CRS);
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
			case RMP_VOR2_CRS:
				panel_rmp_general_single_float(&nav2_crs,
				                  30,
				                  1,
				                  359,
				                  0,
				                  ID_NAV2_CRS);
			break;
			case RMP_COM1:
#ifdef QPAC_A320
				panel_send_dial_commands(ID_COM1_LARGE_UP, ID_COM1_LARGE_DOWN, ID_COM1_SMALL_UP, ID_COM1_SMALL_DOWN);
#else
				panel_rmp_general(&com1_stdby_freq10,
				                  &com1_freq10,
				                  1000,
				                  1000,
				                  25,
				                  136000,
				                  118000,
				                  ID_COM1_STDBY_FREQ,
				                  ID_COM1_FREQ);
#endif
			break;
			case RMP_COM2:
				panel_rmp_general(&com2_stdby_freq10,
				                  &com2_freq10,
				                  1000,
				                  1000,
				                  25,
				                  136000,
				                  118000,
				                  ID_COM2_STDBY_FREQ,
				                  ID_COM2_FREQ);
			break;
			case RMP_BFO:
				panel_rmp_general_single_float(&autop_heading,
				                  30,
				                  1,
				                  359,
				                  0,
				                  ID_AUTOP_HEADING);
			break;
			case RMP_BFO_ALT:
				panel_rmp_general_single_float(&autop_alt,
				                  500,
				                  10,
				                  90000,
				                  0,
				                  ID_AUTOP_ALT);
			break;
			case RMP_VHF1:
			break;
			default:
			break;
	}

	if (!usb_ready)
		return;
	gpio_set_led(LED6, 0);

	if (!xplane_ready) {
		panel_rmp_setup_datarefs_connect(); // fill buffer
		if (!xplane_ready && panel_rmp_setup_datarefs_connect() == 0) {
				xplane_ready = 1;
		}
	}

	if (!xplane_ready) {
		gpio_set_led(LED5, 0);
			return;
	}
	gpio_set_led(LED5, 1);

	if (!is_init) {
		uint32_t timeout;
		/* wait 100ms until teensy plugin is ready */
		timeout = systime_get() + 100;
		while(systime_get() < timeout)
			;
		panel_rmp_setup_datarefs();
		is_init = 1;
	}

	if (systime_get() - teensy_get_last_request_time() < 500) {
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

void panel_rmp_general_single_float(int32_t *act, int32_t high_step, int32_t low_step, int32_t max, int32_t min, uint32_t id_act) {
	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, 0);
	int32_t last = *act;
	int32_t tmp = *act;
	float f;

	if (enc_low) {
		tmp += enc_low * low_step;	// 5kHz
		if (tmp < min) tmp = max;
		if (tmp > max) tmp = min;
		*act = tmp;
	} else if (enc_high) {
		tmp += enc_high * high_step;	// 1MHz
		if (tmp < min) {
			while (tmp + high_step < max) tmp += high_step;
		}
		*act = tmp;
		if (*act > max) {
			tmp = *act % high_step;
			*act = min + tmp;
		}
	}

	f = *act;
	if (*act != last)
		teensy_send_float(id_act, f);
}

void panel_rmp_general(uint32_t *stdby, uint32_t *act, uint16_t comma_value, uint32_t high_step, uint32_t low_step, uint32_t max, uint32_t min, uint32_t id_stdby, uint32_t id_act) {
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

		*stdby = hv * comma_value + tmp;
		if (*stdby > max) *stdby = max;

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

void panel_send_dial_commands(uint32_t large_up, uint32_t large_down, uint32_t small_up, uint32_t small_down) {
	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, 0);

	if (enc_high)
		teensy_send_command_once(enc_high > 0 ? large_up : large_down);
	
	if (enc_low)
		teensy_send_command_once(enc_low > 0 ? small_up : small_down);
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

uint32_t panel_rmp_get_nav1_freq(void) {
	return nav1_freq;
}

uint32_t panel_rmp_get_nav1_stdby_freq(void) {
	return nav1_stdby_freq;
}

uint32_t panel_rmp_get_nav1_crs(void) {
	return nav1_crs;
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

uint32_t panel_rmp_get_nav2_crs(void) {
	return nav2_crs;
}

uint32_t panel_rmp_get_com1_freq(void) {
	return com1_freq10;
}

uint32_t panel_rmp_get_com1_stdby_freq(void) {
	return com1_stdby_freq10;
}

uint32_t panel_rmp_get_com2_freq(void) {
	return com2_freq10;
}

uint32_t panel_rmp_get_com2_stdby_freq(void) {
	return com2_stdby_freq10;
}

void panel_rmp_set_avionics_power(uint32_t on) {
	if (!is_init)
		return;

	if (on != avionics_power) {
		avionics_power = on;
		teensy_send_int(ID_AVIONICS_POWER, avionics_power);
	}
}

uint32_t panel_rmp_get_autop_heading(void) {
	return autop_heading;
}

uint32_t panel_rmp_get_autop_alt(void) {
	return autop_alt;
}

int32_t panel_rmp_get_aircraft_speed(void) {
	return airspeed;
}

uint32_t panel_rmp_get_aircraft_course(void) {
	return course;
}

int32_t panel_rmp_get_aircraft_variometer(void) {
	return variometer;
}

/* try to register dataref to see ix X-Plane is started and ready
 * return: 0 .. ok, <0 .. error (X-PLane not ready)
 */
int panel_rmp_setup_datarefs_connect(void) {
		int ret;

		ret = teensy_register_dataref(ID_AIRCRAFT_TYPE, "sim/aircraft/view/acf_ICAO", 1, &panel_rmp_connect_cb);
		return ret;
}

void panel_rmp_setup_datarefs(void) {
		teensy_register_dataref(ID_AIRCRAFT_TYPE, "sim/aircraft/view/acf_ICAO", 1, &panel_rmp_connect_cb);
		teensy_register_dataref(ID_STROBE_LIGHT, "sim/cockpit/electrical/strobe_lights_on", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV_LIGHT, "sim/cockpit/electrical/nav_lights_on", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_FREQ, "sim/cockpit2/radios/actuators/nav1_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_STDBY_FREQ, "sim/cockpit2/radios/actuators/nav1_standby_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV1_CRS, "sim/cockpit/radios/nav1_obs_degm", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_NDB_FREQ, "sim/cockpit2/radios/actuators/adf1_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NDB_STDBY_FREQ, "sim/cockpit2/radios/actuators/adf1_standby_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV2_FREQ, "sim/cockpit2/radios/actuators/nav2_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV2_STDBY_FREQ, "sim/cockpit2/radios/actuators/nav2_standby_frequency_hz", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_NAV2_CRS, "sim/cockpit/radios/nav2_obs_degm", 2, &panel_rmp_cb);
		//teensy_register_dataref(ID_COM1_FREQ, "sim/cockpit2/radios/actuators/com1_frequency_hz_833", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_FREQ, "*2/radios/actuators/com1_frequency_hz_833", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_STDBY_FREQ, "*2/radios/actuators/com1_standby_frequency_hz_833", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_COM2_FREQ, "*2/radios/actuators/com2_frequency_hz_833", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_COM2_STDBY_FREQ, "*2/radios/actuators/com2_standby_frequency_hz_833", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_AVIONICS_POWER, "sim/cockpit2/switches/avionics_power_on", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_AUTOP_HEADING, "sim/cockpit2/autopilot/heading_dial_deg_mag_pilot", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_AUTOP_ALT, "sim/cockpit/autopilot/altitude", 2, &panel_rmp_cb);
#ifdef QPAC_A320
		teensy_register_dataref(ID_COM1_STDBY_FREQ,   "AirbusFBW/RMP1StbyFreq", 1, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_LARGE_UP, "AirbusFBW/RMP1FreqUpLrg", 0, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_LARGE_DOWN, "AirbusFBW/RMP1FreqDownLrg", 0, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_SMALL_UP, "AirbusFBW/RMP1FreqUpSml", 0, &panel_rmp_cb);
		teensy_register_dataref(ID_COM1_SMALL_DOWN, "AirbusFBW/RMP1FreqDownSml", 0, &panel_rmp_cb);
#else
#endif
		teensy_register_dataref(ID_AIRCRAFT_AIRSPEED, "sim/cockpit2/gauges/indicators/airspeed_kts_pilot", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_AIRCRAFT_VARIO, "sim/cockpit2/gauges/indicators/vvi_fpm_pilot", 2, &panel_rmp_cb);
		teensy_register_dataref(ID_AIRCRAFT_COURSE, "sim/cockpit2/gauges/indicators/compass_heading_deg_mag", 2, &panel_rmp_cb);
}


void panel_rmp_connect_cb(uint8_t id, uint32_t data) {
		id = id;
		data = data;
}


void panel_rmp_cb(uint8_t id, uint32_t data) {
	switch(id) {
		case ID_STROBE_LIGHT:
				//gpio_set_led(LED5, !!data);
				break;
		case ID_NAV_LIGHT:
				//gpio_set_led(LED4, !!data);
				break;
		case ID_NAV1_FREQ:
				nav1_freq = data;
				break;
		case ID_NAV1_STDBY_FREQ:
				nav1_stdby_freq = data;
				break;
		case ID_NAV1_CRS:
				nav1_crs = teensy_from_float(data);
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
		case ID_NAV2_CRS:
				nav2_crs = data;
				break;
		case ID_COM1_FREQ:
				com1_freq10 = data;
				break;
		case ID_COM1_STDBY_FREQ:
				com1_stdby_freq10 = data;
				break;
		case ID_COM2_FREQ:
				com2_freq10 = data;
				break;
		case ID_COM2_STDBY_FREQ:
				com2_stdby_freq10 = data;
				break;
		case ID_AVIONICS_POWER:
				//avionics_power_on = data;
				break;
		case ID_AIRCRAFT_AIRSPEED:
				airspeed = teensy_from_float(data);
				break;
		case ID_AIRCRAFT_VARIO:
				variometer = teensy_from_float(data);
				break;
		case ID_AIRCRAFT_COURSE:
				course = teensy_from_float(data);
				break;
	}
}


void panel_rmp_setup(void)
{
	rmp_act = RMP_VOR;
	led_set(LED_VOR, 1);
	gpio_set_led(LED5, 0);
	gpio_set_led(LED6, 1);
}
