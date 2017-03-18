#include <libopencm3/stm32/gpio.h>

#include "max6956.h"
#include "led.h"

#define PIN_IN_CNT (sizeof(led_ins) / sizeof(led_t))

typedef struct {
		uint16_t ledname;
		uint8_t i2c_dev_adr;
		uint8_t adr;
} led_t;

typedef struct {
		led_t *dat;
		uint8_t state;
} led_priv_t;


#if 1
led_t led_ins[] = {
	{LED_NAV,  0x40, 19},
	{LED_VOR,  0x40, 18},
	{LED_ILS,  0x40, 17},
	{LED_MLS,  0x40, 16}, // VOR2
	{LED_ADF,  0x40, 29},
	{LED_BFO,  0x40, 28},
	{LED_HF1,  0x40, 15},
	{LED_HF2,  0x40, 23},
	{LED_AM,   0x40, 24},
	{LED_VHF1, 0x40, 21},
	{LED_VHF2, 0x40, 20},
	{LED_VHF3, 0x40, 22},
	{LED_SEL,  0x40, 27},
};

#else
led_t led_ins[] = {
};
#endif

led_priv_t led_ins_priv[PIN_IN_CNT];

void led_set(uint32_t pin, uint32_t state) {
	if (pin >= PIN_IN_CNT)
		return;

	max6956_set_led(I2C2, led_ins[pin].i2c_dev_adr, led_ins[pin].adr, state);
}


void led_clear(uint32_t pin) {
	led_set(pin, 0);
}


void led_standby(uint8_t state) {
	max6956_standby(I2C2, 0x40, state);
}
