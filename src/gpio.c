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
#if 1
gpio_t pin_ins[] = {
	{SWITCH_SW_STBY, GPIOA, GPIO4, GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_NAV,  GPIOD, GPIO1,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_VOR,  GPIOD, GPIO2,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_ILS,  GPIOD, GPIO4,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_VOR2, GPIOD, GPIO5,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, // MLS
	{SWITCH_ADF,  GPIOD, GPIO6,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_BFO,  GPIOD, GPIO7,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_HF1,  GPIOE, GPIO3,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_HF2,  GPIOA, GPIO1,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_AM,   GPIOA, GPIO0,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_VHF1, GPIOE, GPIO2,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_VHF2, GPIOE, GPIO4,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_VHF3, GPIOA, GPIO2,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_RMP_OFF, GPIOC, GPIO12, GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}
};

#else
gpio_t pin_ins[] = {
	{SWITCH_SW_STBY, GPIOE, GPIO7, GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_NAV,  GPIOA, GPIO15,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_VOR,  GPIOD, GPIO2,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_ILS,  GPIOD, GPIO0,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_VOR2, GPIOE, GPIO6,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, // MLS
	{SWITCH_ADF,  GPIOE, GPIO2,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_BFO,  GPIOB, GPIO8,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_HF1,  GPIOC, GPIO9,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_HF2,  GPIOE, GPIO15, GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_AM,   GPIOE, GPIO13, GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_VHF1, GPIOC, GPIO8,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_VHF2, GPIOE, GPIO11, GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_VHF3, GPIOE, GPIO9,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_RMP_OFF, GPIOD, GPIO6, GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}
};
#endif

gpio_priv_t pin_ins_priv[PIN_IN_CNT];

void gpio_set_led(uint32_t pin, uint32_t state) {
	if (!state)
		gpio_set(LED5_PORT, pin);
	else
		gpio_clear(LED5_PORT, pin);
}

void gpio_toggle_led(uint32_t pin) {
	gpio_toggle(LED5_PORT, pin);
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

	gpio_mode_setup(LED6_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			LED5_PIN | LED6_PIN);


	//gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
	//		SWITCHB1_PIN);
}
