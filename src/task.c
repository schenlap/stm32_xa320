#include <stdint.h>
#include "systime.h"
#include "task.h"


task_t tasks[TASKS_MAX];
uint32_t uptime_ms;
int task_lock_irq_counter = 0;


static void lock_irq(void);


void task_init(void) {
	int i;

	for (i = 0; i < TASKS_MAX; i++) {
		tasks[i].cyctime_counter = 0;
		tasks[i].task = 0;
	}
}


int task_create(void (*task)(void), int cyctime_ms) {
	int i = 0;
	while (i < TASKS_MAX) {
		if (tasks[i].task == 0) {
			tasks[i].task = task;
			tasks[i].cyctime_ms = cyctime_ms;
			return 0;
		}
		i++;
	}

	/* no free slot */
	return -1;
}


void task_start(void) {
	int i;

	for (i = 0; i < TASKS_MAX; i++) {
		if (tasks[i].task == 0)
			break;

		if (tasks[i].cyctime_counter >= tasks[i].cyctime_ms) {
			tasks[i].cyctime_counter -= tasks[i].cyctime_ms;
			tasks[i].task();	// run task
		}
	}
}


void task_time_increment(void) {
	static long last_tick = 0;
	int i;
	long up;

	lock_irq();
	up = systime_get();
	unlock_irq();

	// wait for next ms tick
	if (up != last_tick) {
		lock_irq();
			//delay = up - last_tick;
			last_tick = up;
		unlock_irq();

		for (i = 0; i < TASKS_MAX; i++) {
			if (tasks[i].task == 0)
				break;

			//tasks[i].cyctime_counter += delay;;
			tasks[i].cyctime_counter ++;
		}
	}

}


static void lock_irq(void) {
	if (!task_lock_irq_counter)
		asm volatile ("cpsid i");
	task_lock_irq_counter++;
}


/** \brief  Get Base Priority
 *  This function returns the current value of the Base Priority register.
 *  \return               Base Priority register value
 */
static uint32_t get_BASEPRI(void)
{
	int pri;
	asm volatile("mrs %0, BASEPRI" : "=r"(pri));
	return pri;
}


/** \brief  Set Base Priority
 *  This function assigns the given value to the Base Priority register.
 *  \param [in]    basePri  Base Priority value to set
 *
 */
static void set_BASEPRI(uint32_t basePri)
{
	basePri = basePri;
	asm("msr BASEPRI, r0");
}


uint32_t lock_irq_low(void)
{
	int ret = get_BASEPRI();

	if (!task_lock_irq_counter)
		set_BASEPRI(IRQ_PRI_LOW);
	task_lock_irq_counter++;
	return ret;
}


void unlock_irq(void) {
	task_lock_irq_counter--;
	if (!task_lock_irq_counter) {
		asm volatile ("cpsie i");
		set_BASEPRI(0x00);
	}
}

