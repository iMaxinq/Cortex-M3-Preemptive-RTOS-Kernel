#ifndef MINIRTOS_H
#define MINIRTOS_H

#include <stdint.h>
#include <stdbool.h>

/* Function signature for every RTOS task: void task_fn(void) */
typedef void (*task_func_t)(void);

/* Priority of the internal idle task. Application tasks should use >= 1. */
#define TASK_PRIORITY_IDLE  0U

/**
 * @brief Create a new task with its own private stack and forged initial context.
 *
 * @param entry     Function pointer to the task's infinite loop.
 * @param name      Human-readable task name (for debugging/crash dumps).
 * @param priority  Higher number = more important. The scheduler always runs
 *                  the highest-priority ready task; equal priorities share
 *                  the CPU round-robin, one 1 ms tick each.
 * @return          Assigned task ID (PID >= 0) on success, or -1 if full.
 */
int task_create(task_func_t entry, const char *name, uint8_t priority);

/**
 * @brief Block the calling task for `ms` ticks (1 tick = 1 ms). It wakes on the
 *        `ms`-th tick interrupt after the call, so the first ms may be partial.
 *        A sleeping task is never scheduled, so it uses no CPU time.
 *        task_sleep(0) just yields to another ready task of equal priority.
 *        Must only be called from a task, after rtos_start().
 */
void task_sleep(uint32_t ms);

/**
 * @brief Number of ticks during which the idle task was running, i.e. the
 *        CPU had nothing to do. Compare with systick_get_ticks() for load.
 */
uint32_t task_get_idle_ticks(void);

/**
 * @brief Start the preemptive scheduler and launch the first task.
 *        Does not return.
 */
void rtos_start(void);

#endif /* MINIRTOS_H */