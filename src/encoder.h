#ifndef ENCODER_H
#define ENCODER_H

#include <stdlib.h>
#include <stdint.h>
#include <libopencm3/stm32/rcc.h>

#define ENC_A     0
#define ENC_B     1
#define ENC_CNT   2



char *encoder_read_str(uint16_t nr);
int16_t encoder_read(uint16_t nr, uint8_t *coarse);
void encoder_task(void);
void encoder_setup(void);

#endif // ENCODER_H
