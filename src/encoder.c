#include <stdio.h>
#include <libopencm3/stm32/gpio.h>

#include "systime.h"
#include "encoder.h"

typedef struct {
		uint8_t nr;
		uint32_t port;
		uint32_t pina;
		uint32_t pinb;
} enc_defs;

typedef struct {
		uint16_t pindata;
		int16_t value;
		char valstr[4];
		uint32_t last;
		uint8_t coarse;
} enc_priv_t;

enc_defs enc_desc[] = {
		{ENC_A, GPIOD, GPIO8, GPIO10},
		{ENC_B, GPIOB, GPIO13, GPIO15}
};

enc_priv_t enc_priv[ENC_CNT];

int16_t encoder_process(uint16_t d);

int16_t encoder_process(uint16_t d) {
	int16_t up = 0;

	switch(d) {
			case 0b0000: up = 0; break; 
			case 0b0001: up = -1; break; 
			case 0b0010: up = 1; break; 
			case 0b0011: up = 0; break; 
			case 0b0100: up = 1; break; 
			case 0b0101: up = 0; break; 
			case 0b0110: up = 0; break; 
			case 0b0111: up = -1; break; 
			
			case 0b1000: up = -1; break; 
			case 0b1001: up = 0; break; 
			case 0b1010: up = 0; break; 
			case 0b1011: up = 1; break; 
			case 0b1100: up = 0; break; 
			case 0b1101: up = 1; break; 
			case 0b1110: up = -1; break; 
			case 0b1111: up = 0; break; 
	}

	return up;
}

void encoder_task(void) {
	int i;
	enc_defs *p = enc_desc;
	enc_priv_t *priv = enc_priv;
	uint16_t up;
	uint32_t pdata;           // Port data
	uint32_t now = systime_get();

	for (i = 0; i < (int)(sizeof(enc_desc) / sizeof(enc_defs)); i++, p++, priv++) {
			pdata = gpio_port_read(p->port);
			priv->pindata = (priv->pindata << 2) & 0x0F;
			if (pdata & p->pina)
				priv->pindata |= 0x01;
			if (pdata & p->pinb)
				priv->pindata |= 0x02;

			up = encoder_process(priv->pindata);
			if (up) {
				priv->value += up;
				if (priv->value / 4) {
					if (now - priv->last < 15)
						priv->coarse = 1;
					priv->last = systime_get();
				}
			}
	}
}

char *encoder_read_str(uint16_t nr) {
		if (nr >= ENC_CNT)
			return 0;
		snprintf(enc_priv[nr].valstr, 5, "%03d", enc_priv[nr].value);
		enc_priv[nr].value = 0;
		return enc_priv[nr].valstr;
}

int16_t encoder_read(uint16_t nr, uint8_t *coarse) {
		int16_t val;

		if (nr >= ENC_CNT)
			return 0;
		val = enc_priv[nr].value;
		enc_priv[nr].value = enc_priv[nr].value % 4; // Rest erhalten

		if(coarse)
			*coarse = enc_priv[nr].coarse;
		enc_priv[nr].coarse = 0;

		return val / 4;
}

void encoder_setup(void)
{
	int i;
	enc_defs *p = enc_desc;
	rcc_periph_clock_enable(RCC_GPIOD); // Leds, Encoder

	for (i = 0; i < (int)(sizeof(enc_desc) / sizeof(enc_defs)); i++, p++) {
		gpio_mode_setup(p->port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP,
		     p->pina | p->pinb);
	}
}
