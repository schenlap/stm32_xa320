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

#define SERVO_DEBUG 1
//#undef SERVO_DEBUG

extern uint8_t config_panel;

void panel_fis_switch_cb(uint8_t id, uint32_t data);

/* nonlinear functions */
servo_display_nonlinear_t fis_nl_altimeter[] = {
	{4, 4}, // array length
	{0, 0},
	{100, 100},
	{500, 500},
	{5000, 5000} // 5000 will not be reached
};
#define ID_BATTERY_POWER 4
servo_display_defs servo_defs[] = {
	{SERVO_ALT, SERVO_MIN, SERVO_MAX, -1, fis_nl_altimeter, 0, 5000, 1, TEENSY_INT, ID_AUTOP_ALT, "sim/cockpit/autopilot/altitude", &panel_fis_cb},
	{0,                 0,         0,  0,     SERVO_LINEAR, 0, 1   , 1, TEENSY_INT, ID_AVIONICS_POWER, "sim/cockpit2/switches/avionics_power_on", &panel_fis_switch_cb},
	{0,                 0,         0,  0,     SERVO_LINEAR, 0, 1   , 1, TEENSY_INT, ID_BATTERY_POWER, "sim/cockpit2/electrical/battery_on", &panel_fis_switch_cb}
	//{SERVO_ALT, SERVO_MIN, SERVO_MAX, -1, SERVO_LINEAR, 0, 5000, 1, TEENSY_FLOAT, ID_AUTOP_ALT, "sim/cockpit/autopilot/altitude", &panel_fis_cb}
};


void task_panel_fis(void) {
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
void panel_fis_cb(uint8_t id, uint32_t data) {
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
