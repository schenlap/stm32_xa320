#include <libopencm3/stm32/gpio.h>

#include "gpio.h"


void gpio_set_led(uint32_t pin, uint32_t state) {
	if (state)
		gpio_set(LED3_PORT, pin);
	else
		gpio_clear(LED3_PORT, pin);
}

void gpio_toggle_led(uint32_t pin) {
	gpio_toggle(LED3_PORT, pin);
}

int gpio_get_switch() {
	return gpio_get(SWITCHB1_PORT, SWITCHB1_PIN);
}

void gpio_setup(void)
{

	rcc_periph_clock_enable(RCC_GPIOD); // Leds
	rcc_periph_clock_enable(RCC_GPIOA); // Taster

	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			LED3_PIN | LED4_PIN | LED5_PIN | LED6_PIN);


	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
			SWITCHB1_PIN);
}
