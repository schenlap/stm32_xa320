#ifndef _TASK_H_
#define _TASK_H_

#define TASKS_MAX 10

typedef struct {
	int cyctime_ms;
	int cyctime_counter;
	void (*task)(void);
} task_t;

void task_init();

int task_create(void (*task)(void), int cyctime_ms);

void task_start();

void task_time_increment();

void lock_irq();

void unlock_irq();

#endif
