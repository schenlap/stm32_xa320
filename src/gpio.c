#include <libopencm3/stm32/gpio.h>

#include "gpio.h"

#define ACTIVE_HIGH  0
#define ACTIVE_LOW   1

#define DEBOUNCE_NO  0
#define DEBOUNCE_LOW 3
#define DEBOUNCE_MED 10
#define DEBOUNCE_HIGH 20

#define PIN_IN_CNT (sizeof(pin_ins) / sizeof(gpio_t))

typedef struct {
		uint16_t pinname;
		uint32_t port;
		uint32_t pin;
		uint32_t pupd;
		uint8_t lowactive;
		uint8_t debounce;
} gpio_t;

typedef struct {
		gpio_t *dat;
		uint8_t state;
		uint8_t pos_event;
		uint8_t cnt;
} gpio_priv_t;

gpio_t pin_ins[] = {
	{SWITCH1, GPIOA, GPIO0, GPIO_PUPD_NONE, ACTIVE_HIGH, DEBOUNCE_LOW}
};

gpio_priv_t pin_ins_priv[PIN_IN_CNT];

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

uint8_t gpio_get_state(uint16_t nr) {
	if (nr >= PIN_IN_CNT)
		return 0;

	return pin_ins_priv[nr].state;
}

uint8_t gpio_get_pos_event(uint16_t nr) {
	if (nr >= PIN_IN_CNT)
		return 0;

	uint8_t ev = pin_ins_priv[nr].pos_event;
	pin_ins_priv[nr].pos_event = 0;

	return ev;
}

void gpio_task(void) {
	uint16_t i;
	uint16_t cnt = PIN_IN_CNT;

	gpio_t *d = pin_ins;
	gpio_priv_t *p = pin_ins_priv;
	for (i = 0; i < cnt; i++, d++, p++) {
		uint8_t data;
		data = d->lowactive == ACTIVE_LOW ? !gpio_get(d->port, d->pin) : gpio_get(d->port, d->pin);

		if (data) {
			if(p->cnt <= d->debounce)
				p->cnt++;
		} else {
			p->cnt = 0;
		}

		if (p->cnt == d->debounce)
			p->pos_event = 1;

		p->state = p->cnt >= d->debounce;
	}
}

void gpio_setup(void)
{
	uint16_t i;
	uint16_t cnt = PIN_IN_CNT;

	for (i = 0; i < cnt; i++) {
		uint32_t rcc = 0;
		gpio_t *d = pin_ins;
	
		switch(d[i].port) {
			case GPIOA:
				rcc = RCC_GPIOA;
				break;
			case GPIOB:
				rcc = RCC_GPIOB;
				break;
			case GPIOC:
				rcc = RCC_GPIOC;
				break;
			case GPIOD:
				rcc = RCC_GPIOD;
				break;
			case GPIOE:
				rcc = RCC_GPIOE;
				break;
			case GPIOF:
				rcc = RCC_GPIOF;
				break;
			case GPIOH:
				rcc = RCC_GPIOH;
				break;
		}
		rcc_periph_clock_enable(rcc);

		gpio_mode_setup(d[i].port, GPIO_MODE_INPUT, d[i].pupd, d[i].pin);

		pin_ins_priv[i].dat = &d[i];
	}

	rcc_periph_clock_enable(RCC_GPIOD); // Leds
	//rcc_periph_clock_enable(RCC_GPIOA); // Taster

	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			LED3_PIN | LED4_PIN | LED5_PIN | LED6_PIN);


	//gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
	//		SWITCHB1_PIN);
}
