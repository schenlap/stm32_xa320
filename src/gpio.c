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
		uint16_t state;
		uint16_t pos_event;
		uint16_t cnt;
		uint16_t any_event;
		uint16_t last_state; // for any event
} gpio_priv_t;
#if 1
gpio_t pin_ins[] = {
	{SWITCH_SW_STBY, GPIOA, GPIO4, GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //0
	{SWITCH_NAV,  GPIOD, GPIO1,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //1
	{SWITCH_VOR,  GPIOD, GPIO2,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //2
	{SWITCH_ILS,  GPIOD, GPIO4,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //3
	{SWITCH_VOR2, GPIOD, GPIO5,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, // MLS
	{SWITCH_ADF,  GPIOD, GPIO6,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //5
	{SWITCH_BFO,  GPIOD, GPIO7,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //6
	{SWITCH_HF1,  GPIOE, GPIO3,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //7
	{SWITCH_HF2,  GPIOA, GPIO1,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //8
	{SWITCH_AM,   GPIOA, GPIO0,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //9
	{SWITCH_VHF1, GPIOE, GPIO2,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //10
	{SWITCH_VHF2, GPIOE, GPIO4,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //11
	{SWITCH_VHF3, GPIOA, GPIO2,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //12
	{SWITCH_RMP_OFF, GPIOC, GPIO12, GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, //13
	{SWITCH_CONFIG_PANEL, GPIOA, GPIO15, GPIO_PUPD_PULLUP, ACTIVE_HIGH, DEBOUNCE_NO}, // 1..PANEL_RMP, 0..PANEL_FIS
	{SWITCH_LGEN, GPIOC, GPIO1,  GPIO_PUPD_PULLUP, ACTIVE_HIGH, DEBOUNCE_LOW}, // DI8 //15
	{SWITCH_BATT, GPIOC, GPIO5,  GPIO_PUPD_PULLUP, ACTIVE_HIGH, DEBOUNCE_LOW},  // DI10 //16
	{SWITCH_RGEN, GPIOB, GPIO0,  GPIO_PUPD_PULLUP, ACTIVE_HIGH, DEBOUNCE_LOW},  // DI12 //17
	{SWITCH_LAND, GPIOB, GPIO12,  GPIO_PUPD_PULLUP, ACTIVE_HIGH, DEBOUNCE_LOW}, // DI14 //18
	{SWITCH_BCN, GPIOB, GPIO13,  GPIO_PUPD_PULLUP, ACTIVE_HIGH, DEBOUNCE_LOW},  // DI16
	{SWITCH_TAXI, GPIOD, GPIO10,  GPIO_PUPD_PULLUP, ACTIVE_HIGH, DEBOUNCE_LOW}, // DI18
	{SWITCH_IGNL_S, GPIOB, GPIO4,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_IGNL_N, GPIOB, GPIO5,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_PWR, GPIOB, GPIO11,  GPIO_PUPD_PULLUP, ACTIVE_HIGH, DEBOUNCE_LOW}, // DI13
	{SWITCH_IGNR_S, GPIOD, GPIO6,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_IGNR_N, GPIOD, GPIO7,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_LNAV, GPIOD, GPIO8,  GPIO_PUPD_PULLUP, ACTIVE_HIGH, DEBOUNCE_LOW}, // DI15
	{SWITCH_STROBE, GPIOD, GPIO9,  GPIO_PUPD_PULLUP, ACTIVE_HIGH, DEBOUNCE_LOW}, // DI17
	{SWITCH_PAX_SAFE, GPIOD, GPIO1,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, // DI3
	{SWITCH_PAX_OFF, GPIOD, GPIO3,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}, // DI4
	{SWITCH_GEAR_UP, GPIOD, GPIO0,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW},
	{SWITCH_GEAR_DOWN, GPIOD, GPIO2,  GPIO_PUPD_PULLUP, ACTIVE_LOW, DEBOUNCE_LOW}
};

gpio_t pin_outs[] = {
	{LED_GEAR_DEPLOYED, GPIOC, GPIO11, GPIO_PUPD_NONE, ACTIVE_HIGH, DEBOUNCE_LOW}, // DI8
	{LED_NAV1_GLIDESLOP_OFF, GPIOE, GPIO13, GPIO_PUPD_NONE, ACTIVE_LOW, DEBOUNCE_LOW},
	{LED_NAV1_TO, GPIOE, GPIO11, GPIO_PUPD_NONE, ACTIVE_LOW, DEBOUNCE_LOW},
	{LED_NAV1_FROM, GPIOC, GPIO13, GPIO_PUPD_NONE, ACTIVE_LOW, DEBOUNCE_LOW}
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

void gpio_set_led_nr(uint32_t nr, uint32_t state) {
	if (nr >= sizeof(pin_outs) / sizeof(gpio_t))
		return;

	if (state == pin_outs[nr].lowactive)
		gpio_set(pin_outs[nr].port, pin_outs[nr].pin);
	else
		gpio_clear(pin_outs[nr].port, pin_outs[nr].pin);
}

void gpio_set_led(uint32_t pin, uint32_t state) {
	uint32_t port;
	if (pin == LED5 || pin == LED6)
		port = LED5_PORT;
	else
		port = LED_GREEN_PORT;

	if (!state)
		gpio_set(port, pin);
	else
		gpio_clear(port, pin);
}

void gpio_toggle_led(uint32_t pin) {
	uint32_t port;
	if (pin == LED5 || pin == LED6)
		port = LED5_PORT;
	else
		port = LED_GREEN_PORT;
	gpio_toggle(port, pin);
}

int gpio_get_switch() {
	return gpio_get(SWITCHB1_PORT, SWITCHB1_PIN);
}

uint8_t gpio_get_state(uint16_t nr) {
	if (nr >= PIN_IN_CNT)
		return 0;

	return pin_ins_priv[nr].state;
}

uint8_t gpio_get_state_direct(uint16_t nr) {
	if (nr >= PIN_IN_CNT)
		return 0;

	gpio_t *d = &pin_ins[nr];
	return d->lowactive == ACTIVE_LOW ? !gpio_get(d->port, d->pin) : gpio_get(d->port, d->pin);
}

uint8_t gpio_get_pos_event(uint16_t nr) {
	if (nr >= PIN_IN_CNT)
		return 0;

	uint8_t ev = pin_ins_priv[nr].pos_event;
	pin_ins_priv[nr].pos_event = 0;

	return ev;
}

uint8_t gpio_get_any_event(uint16_t nr) {
	if (nr >= PIN_IN_CNT)
		return 0;

	uint8_t ev = pin_ins_priv[nr].any_event;
	pin_ins_priv[nr].any_event = 0;

	return ev;
}

void gpio_task(void) {
	uint16_t i;
	uint16_t cnt = PIN_IN_CNT;

	gpio_t *d = pin_ins;
	gpio_priv_t *p = pin_ins_priv;
	for (i = 0; i < cnt; i++, d++, p++) {
		uint16_t data;

		data = gpio_get(d->port, d->pin);
		data = gpio_port_read(d->port);
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

		if (data != p->last_state) {
			p->last_state = data;
			p->any_event = 1;
		}
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

	for (i = 0; i < sizeof(pin_outs) / sizeof(gpio_t); i++) {
		gpio_mode_setup(pin_outs[i].port, GPIO_MODE_OUTPUT, pin_outs[i].pupd, pin_outs[i].pin);
	}

	rcc_periph_clock_enable(RCC_GPIOC); // Leds
	rcc_periph_clock_enable(RCC_GPIOD); // Leds

	gpio_mode_setup(LED6_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			LED5_PIN | LED6_PIN);
	/* stm32eval */
	gpio_mode_setup(LED_GREEN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			LED_GREEN_PIN | LED_ORANGE_PIN | LED_BLUE_PIN | LED_RED_PIN);


	//gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
	//		SWITCHB1_PIN);
}
