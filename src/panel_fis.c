// Flight Instrument System Panel
#include <stdlib.h>
#include <stdio.h>

#include "task.h"
#include "systime.h"
#include "gpio.h"
#include "led.h"
#include "encoder.h"
#include "teensy.h"
#include "usb.h"
#include "servo.h"
#include "servo_display.h"
#include "xa320.h"
#include "max7219.h" // for SERVO_DEBUG
#include "panel_fis.h"

//#define SERVO_DEBUG 1
#undef SERVO_DEBUG

extern uint8_t config_panel;
static int pwm_test = 0; /* testmode, triggers by transponder mode */

void panel_fis_switch_cb(uint8_t id, uint32_t data);
void panel_fis_comp_cb(uint8_t id, uint32_t data);
void panel_fis_cb_servotest(uint8_t id, uint32_t data);


/* nonlinear functions */
servo_display_nonlinear_t fis_nl_altimeter[] = {
	{4, 4}, // array length
	{2850, 0},
	{2350, 100},
	{1330, 500},
	{800, 1800},
	{750, 5000} // 5000 will not be reached
};

servo_display_nonlinear_t fis_nl_linear[] = { // for testing only
	{4, 4}, // array length
	{0, 0},
	{100, 100},
	{500, 500},
	{5000, 5000} // 5000 will not be reached
};

#define ID_BATTERY_POWER 4
servo_display_defs servo_defs[] = {
	{SERVO_SPEED, SERVO_MAX, SERVO_MIN, 0, SERVO_LINEAR,      0, 250,       1, TEENSY_FLOAT, ID_AIRCRAFT_AIRSPEED, "*2/gauges/indicators/airspeed_kts_pilot", &panel_fis_cb},
	{SERVO_ALT,   SERVO_MIN, SERVO_MAX, -1, fis_nl_altimeter, 0, 5000,      1, TEENSY_INT, ID_AUTOP_ALT, "*2/gauges/indicators/altitude_ft_pilot", &panel_fis_cb},
	{SERVO_NAV_H, 1442, 2052,            0, SERVO_LINEAR,   -25, 25,       10, TEENSY_FLOAT, ID_NAV1_HDEF_DOTS10, "*2/radios/indicators/nav1_hdef_dots_pilot", &panel_fis_cb},
	{SERVO_NAV_V, 1442, 2070,            0, SERVO_LINEAR,   -25, 25,       10, TEENSY_FLOAT, ID_NAV1_VDEF_DOTS10, "*2/radios/indicators/nav1_vdef_dots_pilot", &panel_fis_cb},
	{SERVO_COMP,  2500, 800,             0, SERVO_LINEAR,   120, 430,       1, TEENSY_FLOAT, ID_AIRCRAFT_COURSE, "*2/gauges/indicators/compass_heading_deg_mag", &panel_fis_comp_cb},
	{SERVO_VARIO, 2200, 800,             0, SERVO_LINEAR, -1000, 1500,      1, TEENSY_FLOAT, ID_AIRCRAFT_VARIO, "*2/gauges/indicators/vvi_fpm_pilot", &panel_fis_cb},
	{SERVO_NAV2,  1200, 1800,            0, SERVO_LINEAR,   -50, 50,       10, TEENSY_FLOAT, ID_NAV2_HDEF_DOTS10, "*2/radios/indicators/nav2_hdef_dots_pilot", &panel_fis_cb},
	{0,           0,    0,               0, SERVO_LINEAR,     0, 1,         1, TEENSY_INT, ID_AVIONICS_POWER, "sim/cockpit2/switches/avionics_power_on", &panel_fis_switch_cb},
	{0,           0,    0,               0, SERVO_LINEAR,     0, 1,         1, TEENSY_INT, ID_BATTERY_POWER, "sim/cockpit2/electrical/battery_on", &panel_fis_switch_cb},
	{0,           SERVO_MIN, SERVO_MAX,  0, SERVO_LINEAR,     0, 10,        1, TEENSY_INT, ID_TRANSPONDER_MODE, "sim/cockpit/radios/transponder_mode", &panel_fis_cb_servotest},
	{0,           SERVO_MIN, SERVO_MAX,  0, SERVO_LINEAR,     0, 8000,      1, TEENSY_INT, ID_TRANSPONDER_CODE, "sim/cockpit/radios/transponder_code", &panel_fis_cb_servotest}
};


void task_panel_fis(void) {
	static uint32_t del;
	static uint32_t pwm = SERVO_MIN;
	if (del++ > 100) {
		del = 0;
		gpio_toggle_led(LED_BLUE);
		pwm += 100;
		if (pwm > SERVO_MAX)
			pwm = SERVO_MIN;

//	for (int i = 0; i < SERVO_CNT; i++) {
//		servo_set_position(i, pwm);
//	}

	}
#if defined SERVO_DEBUG
	static uint32_t servo_debug_data_us = SERVO_MIN;
	char str[8 * 2 + 1];

	int16_t enc_high = encoder_read(ENC_B, 0);
	int16_t enc_low = encoder_read(ENC_A, 0);

	if (enc_low)
		servo_debug_data_us += enc_low;
	if (enc_high)
		servo_debug_data_us += 50 * enc_high;

	if (servo_debug_data_us > SERVO_MAX + 500)
		servo_debug_data_us = SERVO_MAX + 500;
	if (servo_debug_data_us < SERVO_MIN - 500)
		servo_debug_data_us = SERVO_MIN - 500;

	for (int i = 0; i < SERVO_CNT; i++) {
		servo_set_position(i, servo_debug_data_us);
	}

	snprintf(str, 9, "%5d US", (int)servo_debug_data_us);
	max7219_display_string(8, str);
#endif
}


void panel_fis_switch_cb(uint8_t id, uint32_t data) {
	switch(id) {
	case ID_AVIONICS_POWER:
		gpio_set_led(LED_RED, !data);
		gpio_set_led(LED_GREEN, !data);
		break;
	}
	
}


/* test servo with transponder_id. */
/* tronsp_id % 10 = servo_id, trans_id = pwm pulse in us */
/* if trans_id > 3000, 300 is added to pwm value */
void panel_fis_cb_servotest(uint8_t id, uint32_t data) {
	static int servo_id = 99;
	static int pwm_value = SERVO_NULL;

	if (id == ID_TRANSPONDER_MODE) {
		if (data == 3 /* TEST_MODE */)
			pwm_test = 1; /* servo 0 .. 7 */
		else if (data == 4)
			pwm_test = 2; /* servo 8 ... */
		else
			pwm_test = 0;
	} else if (id == ID_TRANSPONDER_CODE) {
		servo_id = data % 10;
		if (data > 3000) {/* transponder values are 0 .. 7 */
			data += 300;
			data -= 3000;
		}
		pwm_value = data;
	}
	if (pwm_value > SERVO_MAX)
		pwm_value = SERVO_MAX;
	if (pwm_value < SERVO_MIN)
		pwm_value = SERVO_MIN;

	if (pwm_test == 1)
		servo_set_position(servo_id, pwm_value);
	else if (pwm_test == 2)
		servo_set_position(servo_id + 8, pwm_value);
}


/* map 120 .. 360 degree and 0 .. 70 degree to servo_min .. servo max. */
/* 70 .. 120 can not be displayed */
void panel_fis_comp_cb(uint8_t id, uint32_t data) {
	if (pwm_test != 0)
		return;

	for (uint32_t i = 0; i < sizeof(servo_defs)/sizeof(servo_display_defs); i++) {
		if (id == servo_defs[i].ref_id) {
			if (servo_defs[i].simval_type == TEENSY_FLOAT) {
				data = teensy_from_float(data);
			}
			if (data > 70 && data < 120) // can not be dispalyed on my servo gauge
				data = 0;
			else if (data > 0 && data <= 70)
				data += 360; // we actually display 120 .. 430 degree
			servo_display_value(servo_defs[i].servo_id, servo_defs[i].sim_multi * data, &servo_defs[i]);
			break;
		}
	}
}


void panel_fis_cb(uint8_t id, uint32_t data) {
	if (pwm_test != 0)
		return;

	for (uint32_t i = 0; i < sizeof(servo_defs)/sizeof(servo_display_defs); i++) {
		if (id == servo_defs[i].ref_id) {
			if (servo_defs[i].simval_type == TEENSY_INT)
				servo_display_value(servo_defs[i].servo_id, servo_defs[i].sim_multi * data, &servo_defs[i]);
			else if (servo_defs[i].simval_type == TEENSY_FLOAT)
				servo_display_value(servo_defs[i].servo_id, servo_defs[i].sim_multi * teensy_from_float(data), &servo_defs[i]);
			break;
		}
	}
}


void panel_fis_setup_datarefs(void) {
	for (uint32_t i = 0; i < sizeof(servo_defs)/sizeof(servo_display_defs); i++) {
		teensy_register_dataref(servo_defs[i].ref_id, servo_defs[i].ref, servo_defs[i].simval_type, servo_defs[i].cb);
	}
}


void panel_fis_setup(void) {

#if defined SERVO_DEBUG
if (config_panel) {
	led_set(LED_SEL, 1);
	max7219_ClearAll();
}
#endif
}
