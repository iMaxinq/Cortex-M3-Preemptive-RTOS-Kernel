#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "minirtos.h"

#define MAX_TASKS           8U     /* Includes the internal idle task */
#define TASK_STACK_WORDS    256U   /* 256 * 4 bytes = 1 KB stack per task */

/* Scheduling state. The running task is also READY; g_current_task tells which one runs. */
typedef enum {
    TASK_READY   = 0,              /* Runnable: eligible to be picked     */
    TASK_BLOCKED = 1,              /* Sleeping until wake_tick            */
} task_state_t;

/* Task Control Block (TCB) */
typedef struct tcb {
    uint32_t    *sp;                       /* MUST be first (offset 0): Saved PSP */
    uint32_t     pid;                      /* Unique Task ID (0 .. MAX_TASKS-1)   */
    const char  *name;                     /* Task name string                    */
    uint8_t      priority;                 /* Higher number = more important      */
    task_state_t state;                    /* READY or BLOCKED                    */
    uint32_t     wake_tick;                /* Tick at which a BLOCKED task wakes  */
    uint32_t     stack[TASK_STACK_WORDS];  /* Private 1 KB stack in RAM           */
} tcb_t;

/* Global pointers read/updated by PendSV_Handler in kernel/port/port.c */
extern volatile tcb_t *g_current_task;
extern volatile tcb_t *g_next_task;

/**
 * @brief Called by SysTick_Handler every 1 ms.
 *        Wakes sleeping tasks whose time is up, selects the highest-priority
 *        ready task, and returns true if a context switch should be pended.
 *
 * @param now  Current tick count.
 */
bool task_wake_and_reschedule(uint32_t now);

/**
 * @brief Returns true once rtos_start() has initialized the scheduler.
 */
bool task_scheduler_is_running(void);

/* Architecture port function implemented in kernel/port/port.c */
void port_start_first_task(void);
void port_trigger_pendsv(void);

#endif /* TASK_H */
