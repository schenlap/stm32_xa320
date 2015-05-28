#ifndef GPIO_H
#define GPIO_H

#include <stdlib.h>
#include <stdint.h>
#include <libopencm3/stm32/rcc.h>

#define LED3_PIN GPIO13 // orange
#define LED3_PORT GPIOD
#define LED4_PIN GPIO12 // green
#define LED4_PORT GPIOD
#define LED5_PIN GPIO14 // red
#define LED5_PORT GPIOD
#define LED6_PIN GPIO15 // blue
#define LED6_PORT GPIOD

#define LED3 LED3_PIN
#define LED4 LED3_PIN
#define LED5 LED3_PIN
#define LED6 LED3_PIN

#define SWITCHB1_PIN GPIO0
#define SWITCH_PORT GPIOA

void gpio_setup(void);
void gpio_set_led(uint32_t pin, uint32_t state);
void gpio_toggle_led(uint32_t pin);

#endif // GPIO_H
