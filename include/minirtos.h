#ifndef MINIRTOS_H
#define MINIRTOS_H

#include <stdint.h>
#include <stdbool.h>

/* Entry function of every task */
typedef void (*task_func_t)(void);

/* Idle task priority; application tasks should use >= 1 */
#define TASK_PRIORITY_IDLE  0U

/* Creates a task. Higher priority = more important; equal priorities share the
 * CPU round-robin, 1 ms each. Returns the task ID, or -1 if the table is full. */
int task_create(task_func_t entry, const char *name, uint8_t priority);

/* Blocks the calling task for `ms` ticks of 1 ms (the first may be partial),
 * using no CPU. task_sleep(0) yields to a ready task of equal priority. */
void task_sleep(uint32_t ms);

/* Ticks spent in the idle task, i.e. with nothing to run */
uint32_t task_get_idle_ticks(void);

/* Starts the scheduler with the highest-priority task. Never returns. */
void rtos_start(void);

#endif /* MINIRTOS_H */
