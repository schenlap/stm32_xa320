#ifndef _TASK_H_
#define _TASK_H_

#define TASKS_MAX 10

typedef struct {
	int cyctime_ms;
	int cyctime_counter;
	void (*task)(void);
} task_t;

void task_init(void);

int task_create(void (*task)(void), int cyctime_ms);

void task_start(void);

void task_time_increment(void);

//void lock_irq(void);
uint32_t lock_irq_low(void);

void unlock_irq(void);

#define IRQ_PRI_LOW 0x20
#define IRQ_PRI_HIGH 0x10 // never locked
#endif
