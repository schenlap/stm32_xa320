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

#ifndef __SERVO_H_
#define __SERVO_H_

#include <libopencm3/stm32/timer.h>

/**
 * Prescale 72000000 Hz system clock by 72 = 1000000 Hz.
 */
#define PWM_PRESCALE	(72)

/**
 * We need a 50 Hz period (1000 / 20ms = 50), thus devide 100000 by 50 = 20000 (us).
 */
#define PWM_PERIOD		(20000)

/**
 * Max. pos. at 2050 us (2.00ms).
 */
#define SERVO_MAX		(2900)

/**
 * Min. pos. at 950  us (0.95ms).
 */
#define SERVO_MIN		(750)

/**
 * Middle pos. at 1580 us (1.58ms).
 */
#define SERVO_NULL		(1580)

/**
 * TIM2 channel for servo 1.
 *
 * Changing this also requires to change settings in {@link servo_setup}!
 */
#define SERVO_SPEED     0 // PWM01
#define SERVO_ALT       1 // PWM02
#define SERVO_NAV_H     2 // PWM03
#define SERVO_NAV_V     3 // PWM04
#define SERVO_COMP      4 // PWM05
#define SERVO_VARIO     5 // PWM06
#define SERVO_NAV2      6 // PWM07
#define SERVO_PWM08     7
#define SERVO_PWM09     8
#define SERVO_PWM10     9
#define SERVO_PWM11     10
#define SERVO_PWM12     11
#define SERVO_CNT       12

/**
 * Initialize and start the PWM used for the servos, drive servos to middle position.
 */
void servo_setup(void);

/**
 * Drive the servo connected to the given channel to the given position in us.
 *
 * @param[in]	id		The channel of the servo. E.g. SERVO_ALT, ...
 * @param[in]	pos_us	The position in us to which to drive the servo to.
 */
void servo_set_position(uint8_t id, uint32_t pos_us);

#endif
