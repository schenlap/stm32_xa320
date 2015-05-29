#ifndef SYSTIME_H
#define SYSTIME_H

#include <stdlib.h>
#include <stdint.h>
#include <libopencm3/stm32/rcc.h>

void systime_setup(void);
uint32_t systimems_get(void);

#endif // SYSTIME_H
