#ifndef MAX6956_H
#define MAX6956_H

#include <libopencm3/stm32/i2c.h>

void max6956_setup(void);
void max6956_write_register(uint32_t i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t value);
void max6956_set_led_brightness(uint32_t i2c, uint8_t dev_adr, uint8_t nr, uint8_t brightness);
void max6956_set_led(uint32_t i2c, uint8_t dev_adr, uint8_t nr, uint8_t on);
int32_t max6956_read_register(uint32_t i2c, uint8_t dev_addr, uint8_t reg_addr);
void max6956_clear_led(uint32_t i2c, uint8_t dev_adr, uint8_t nr);
void max6956_clear_all_leds(uint32_t i2c, uint8_t dev_adr);
void max6956_standby(uint32_t i2c, uint8_t dev_adr, uint8_t stdby);

#endif // MAX6956_H
