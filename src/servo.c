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
	pwm_init_timer(&RCC_APB1ENR, RCC_APB1ENR_TIM3EN, TIM3, PWM_PRESCALE, PWM_PERIOD);
	pwm_init_timer(&RCC_APB1ENR, RCC_APB1ENR_TIM4EN, TIM4, PWM_PRESCALE, PWM_PERIOD);
	pwm_init_timer(&RCC_APB1ENR, RCC_APB1ENR_TIM5EN, TIM5, PWM_PRESCALE, PWM_PERIOD);
	pwm_init_timer(&RCC_APB2ENR, RCC_APB2ENR_TIM9EN, TIM9, PWM_PRESCALE, PWM_PERIOD);
	
	pwm_init_output_channel(TIM3, TIM_OC1, &RCC_AHB1ENR, RCC_AHB1ENR_IOPCEN, GPIOC, GPIO6); // PWM01 OK
	pwm_init_output_channel(TIM3, TIM_OC3, &RCC_AHB1ENR, RCC_AHB1ENR_IOPCEN, GPIOC, GPIO8); // PWM02 OK
	pwm_init_output_channel(TIM3, TIM_OC4, &RCC_AHB1ENR, RCC_AHB1ENR_IOPCEN, GPIOC, GPIO9); // PWM03 OK
	pwm_init_output_channel(TIM5, TIM_OC3, &RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN, GPIOA, GPIO2); // PWM04 OK
	pwm_init_output_channel(TIM4, TIM_OC2, &RCC_AHB1ENR, RCC_AHB1ENR_IOPBEN, GPIOB, GPIO7); // PWM05 OK
	pwm_init_output_channel(TIM4, TIM_OC3, &RCC_AHB1ENR, RCC_AHB1ENR_IOPBEN, GPIOB, GPIO8); // PWM06 OK
	pwm_init_output_channel(TIM9, TIM_OC1, &RCC_AHB1ENR, RCC_AHB1ENR_IOPEEN, GPIOE, GPIO5); // PWM07
	pwm_init_output_channel(TIM9, TIM_OC2, &RCC_AHB1ENR, RCC_AHB1ENR_IOPEEN, GPIOE, GPIO6); // PWM08
	pwm_init_output_channel(TIM4, TIM_OC4, &RCC_AHB1ENR, RCC_AHB1ENR_IOPBEN, GPIOB, GPIO9); // PWM09 OK
	pwm_init_output_channel(TIM5, TIM_OC1, &RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN, GPIOA, GPIO0); // PWM10 OK
	pwm_init_output_channel(TIM5, TIM_OC2, &RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN, GPIOA, GPIO1); // PWM11 OK
	pwm_init_output_channel(TIM5, TIM_OC4, &RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN, GPIOA, GPIO3); // PWM12 OK

	pwm_set_pulse_width(TIM3, TIM_OC1, SERVO_NULL);
	pwm_set_pulse_width(TIM3, TIM_OC2, SERVO_NULL);
	pwm_set_pulse_width(TIM3, TIM_OC3, SERVO_NULL);
	pwm_set_pulse_width(TIM3, TIM_OC4, SERVO_NULL);
	pwm_set_pulse_width(TIM5, TIM_OC1, SERVO_NULL);
	pwm_set_pulse_width(TIM5, TIM_OC2, SERVO_NULL);
	pwm_set_pulse_width(TIM5, TIM_OC3, SERVO_NULL);
	pwm_set_pulse_width(TIM4, TIM_OC1, SERVO_NULL);
	pwm_set_pulse_width(TIM4, TIM_OC2, SERVO_NULL);
	pwm_set_pulse_width(TIM4, TIM_OC3, SERVO_NULL);
	pwm_set_pulse_width(TIM4, TIM_OC4, SERVO_NULL);
	pwm_set_pulse_width(TIM9, TIM_OC1, SERVO_NULL);

	/* start timer */
	pwm_start_timer(TIM3);
	pwm_start_timer(TIM4);
	pwm_start_timer(TIM5);
	pwm_start_timer(TIM9);
}

uint32_t debug_tim_cnt = 0;
uint32_t debug_pwm_out = 0;
void servo_set_position(uint8_t id, uint32_t pos_us)
{
	debug_tim_cnt = timer_get_counter(TIM9);
	debug_pwm_out = gpio_get(GPIOE, GPIO6);
	switch(id) {
		case SERVO_SPEED:
			pwm_set_pulse_width(TIM3, TIM_OC1, pos_us);
			break;
		case SERVO_ALT:
			pwm_set_pulse_width(TIM3, TIM_OC3, pos_us);
			break;
		case SERVO_NAV_H:
			pwm_set_pulse_width(TIM3, TIM_OC4, pos_us);
			break;
		case SERVO_NAV_V:
			pwm_set_pulse_width(TIM5, TIM_OC3, pos_us);
			break;
		case SERVO_COMP:
			pwm_set_pulse_width(TIM4, TIM_OC2, pos_us);
			break;
		case SERVO_VARIO:
			pwm_set_pulse_width(TIM4, TIM_OC3, pos_us);
			break;
		case SERVO_NAV2:
			pwm_set_pulse_width(TIM9, TIM_OC1, 1000);
			break;
		case SERVO_PWM08:
			pwm_set_pulse_width(TIM9, TIM_OC2, 10000);
			break;
		case SERVO_PWM09:
			pwm_set_pulse_width(TIM4, TIM_OC4, pos_us);
			break;
		case SERVO_PWM10:
			pwm_set_pulse_width(TIM5, TIM_OC1, pos_us);
			break;
		case SERVO_PWM11:
			pwm_set_pulse_width(TIM5, TIM_OC2, pos_us);
			break;
		case SERVO_PWM12:
			pwm_set_pulse_width(TIM5, TIM_OC4, pos_us);
			break;
	}
}

