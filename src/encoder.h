#ifndef ENCODER_H
#define ENCODER_H

#include <stdlib.h>
#include <stdint.h>
#include <libopencm3/stm32/rcc.h>

#define ENC_A     0
#define ENC_CNT   1



char *encoder_read(uint16_t nr);
void encoder_task(void);
void encoder_setup(void);

#endif // ENCODER_H
