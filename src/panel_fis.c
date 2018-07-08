// Flight Instrument System Panel
#include <stdlib.h>
#include <stdio.h>

#include "task.h"
#include "systime.h"
#include "gpio.h"
#include "encoder.h"
#include "teensy.h"
#include "usb.h"
#include "servo.h"
#include "servo_display.h"
#include "xa320.h"
#include "panel_fis.h"

void servo_display_value(uint16_t id, int32_t data, servo_display_defs *s);

servo_display_defs servo_defs[] = {
	{SERVO_ALT, SERVO_MIN, SERVO_MAX, -1, 0, 5000, 1, TEENSY_INT, ID_AUTOP_ALT, "sim/cockpit/autopilot/altitude"}
	//{SERVO_VOR1, SERVO_MIN, SERVO_MAX, -10, -2500, 2500, 1000, TEENSY_FLOAT, ID_AUTOP_ALT, "sim/cockpit/nav1"}
};


void task_panel_fis(void) {
	panel_fis_cb(ID_AUTOP_ALT, 0);
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
	for (uint32_t i = 0; i < sizeof(servo_defs)/sizeof(servo_display_defs); i++) {
		teensy_register_dataref(servo_defs[i].ref_id, servo_defs[i].ref, servo_defs[i].simval_type, &panel_fis_cb);
	}

}
