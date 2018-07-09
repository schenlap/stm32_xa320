/*
 * This file is part of the PWM-Servo example.
 *
 * Copyright (C) 2011 Stefan Wendler <sw@kaltpost.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "servo.h"
#include "xa320.h"
#include "servo_display.h"

uint32_t servo_display_get_nonlinear(servo_display_nonlinear_t *nl, int32_t sim_value);

uint32_t servo_display_get_nonlinear(servo_display_nonlinear_t *nl, int32_t sim_value) {
	uint32_t output;

	/* check bounds */
	if (sim_value > nl[nl[0].sim].sim)
		return 99;
	if (sim_value < nl[1].sim)
		return 98;

	for (int i = 0; i <= (nl[0].sim) - 1; i++) { // last row not used
		if ((sim_value >= nl[i].sim) && (sim_value < nl[i+1].sim)) { // entry found
			output = (sim_value - nl[i].sim) * (nl[i + 1].pwm - nl[i].pwm) / (nl[i + 1].sim - nl[i].sim) + nl[i].sim; // we don't know offset here
			return output;
		}
	}

	// value out of range
	return 97;
}


/* display value with servo, data is already multiplied with factor */
void servo_display_value(uint16_t id, int32_t data, servo_display_defs *s) {
	int32_t pos_us;

	if (data > s->simval_max)
		data = s->simval_max;
	if (data < s->simval_min)
		data = s->simval_min;

	if (s->nl == SERVO_LINEAR)
		pos_us = (data - s->simval_min) * (s->servo_max - s->servo_min) / (s->simval_max - s->simval_min) + s->servo_min + s->servo_offset;
	else
		pos_us = servo_display_get_nonlinear(s->nl, data) + s->servo_offset;

	if (pos_us > SERVO_MAX)
		pos_us = SERVO_MAX;
	if (pos_us < SERVO_MIN)
		pos_us = SERVO_MIN;

	servo_set_position(id, pos_us);
}


void servo_display_setup(void)
{
	servo_set_position(SERVO_ALT, SERVO_MIN);
	servo_set_position(SERVO_VARIO, SERVO_NULL);

}
