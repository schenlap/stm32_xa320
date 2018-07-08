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

void servo_display_value(uint16_t id, int32_t data, servo_display_defs *s);

servo_display_defs servo_defs[] = {
	{SERVO_ALT, SERVO_MIN, SERVO_MAX, -1, 0, 5000, 1, TEENSY_INT, ID_AUTOP_ALT, "sim/cockpit/autopilot/altitude"}
	//{SERVO_VOR1, SERVO_MIN, SERVO_MAX, -10, -2500, 2500, 1000, TEENSY_FLOAT, ID_AUTOP_ALT, "sim/cockpit/nav1"}
};


void task_panel_fis(void) {
	//panel_fis_cb(ID_AUTOP_ALT, 0);
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

/* display value with servo, data is already multiplied with factor */
void servo_display_value(uint16_t id, int32_t data, servo_display_defs *s) {
	int32_t pos_us;

	if (data > s->simval_max)
		data = s->simval_max;
	if (data < s->simval_min)
		data = s->simval_min;

	pos_us = (data - s->simval_min) * (s->servo_max - s->servo_min) / (s->simval_max - s->simval_min) + s->servo_min + s->servo_offset;

	if (pos_us > SERVO_MAX)
		pos_us = SERVO_MAX;
	if (pos_us < SERVO_MIN)
		pos_us = SERVO_MIN;

	servo_set_position(id, pos_us);
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


void panel_fis_setup(void) {
	/* TODO REGSITER DATAREFS AFTER CONNECT */
//	for (uint32_t i = 0; i < sizeof(servo_defs)/sizeof(servo_display_defs); i++) {
//		teensy_register_dataref(servo_defs[i].ref_id, servo_defs[i].ref, servo_defs[i].simval_type, &panel_fis_cb);
//	}

#if defined SERVO_DEBUG
led_set(LED_SEL, 1);
max7219_ClearAll();
#endif
}
