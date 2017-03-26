#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>

#include "task.h"
#include "systime.h"


uint32_t system_millis;

void systime_setup(void)
{
	/* clock rate / 1000 to get 1mS interrupt rate */
	systick_set_reload(168000);
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_counter_enable();
	/* this done last */
	nvic_set_priority(NVIC_SYSTICK_IRQ, IRQ_PRI_HIGH);
	systick_interrupt_enable();
}

uint32_t systime_get(void) {
		return system_millis;
}

void sys_tick_handler(void)
{
	system_millis++;
}

