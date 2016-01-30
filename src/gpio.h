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
#define LED4 LED4_PIN
#define LED5 LED5_PIN
#define LED6 LED6_PIN

#define SWITCHB1_PIN GPIO0
#define SWITCHB1_PORT GPIOA
#define SWITCHB1 SWITCHB1_PIN

#define SWITCH_COM1 SWITCH_HF1
#define SWITCH_COM2 SWITCH_HF2

#define SWITCH1         0
#define SWITCH_SW_STBY  0
#define SWITCH_NAV      1
#define SWITCH_VOR      2
#define SWITCH_ILS      3
#define SWITCH_VOR2     4
#define SWITCH_ADF      5
#define SWITCH_BFO      6
#define SWITCH_HF1      7
#define SWITCH_HF2      8
#define SWITCH_AM       9
#define SWITCH_VHF1     10
#define SWITCH_VHF2     11
#define SWITCH_VHF3     12
#define SWITCH_RMP_OFF  13


void gpio_setup(void);
void gpio_set_led(uint32_t pin, uint32_t state);
void gpio_toggle_led(uint32_t pin);
int gpio_get_switch(void);

void gpio_task(void);
uint8_t gpio_get_state(uint16_t nr);
uint8_t gpio_get_pos_event(uint16_t nr);

#endif // GPIO_H
