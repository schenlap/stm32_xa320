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

#include "pwm.h"
#include "servo.h"

void servo_setup(void)
{
     /* init timer4 with a period of 20ms */
     pwm_init_timer(&RCC_APB1ENR, RCC_APB1ENR_TIM4EN, TIM4, PWM_PRESCALE, PWM_PERIOD);

     pwm_init_output_channel(TIM4, TIM_OC1, &RCC_AHB1ENR, RCC_AHB1ENR_IOPBEN, GPIOB, GPIO6);
     pwm_init_output_channel(TIM4, TIM_OC2, &RCC_AHB1ENR, RCC_AHB1ENR_IOPBEN, GPIOB, GPIO7);

     pwm_set_pulse_width(TIM4, TIM_OC1, SERVO_NULL);
     pwm_set_pulse_width(TIM4, TIM_OC2, SERVO_NULL);

     /* start timer1 */
     pwm_start_timer(TIM4);
}

void servo_set_position(uint8_t id, uint32_t pos_us)
{
	switch(id) {
		case SERVO_ALT:
			pwm_set_pulse_width(TIM4, TIM_OC1, pos_us);
			break;
		case SERVO_VARIO:
			pwm_set_pulse_width(TIM4, TIM_OC2, pos_us);
			break;
	}
}

