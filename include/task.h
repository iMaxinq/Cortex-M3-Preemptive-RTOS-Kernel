#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "minirtos.h"

#define MAX_TASKS           8U     /* Includes the idle task */
#define TASK_STACK_WORDS    256U   /* 1 KB */

/* The running task is READY too; g_current_task tells which one runs */
typedef enum {
    TASK_READY   = 0,
    TASK_BLOCKED = 1,              /* Sleeping until wake_tick */
} task_state_t;

/* Task Control Block */
typedef struct tcb {
    uint32_t    *sp;                       /* Saved PSP; must stay first for PendSV */
    uint32_t     pid;                      /* Index in the task table */
    const char  *name;
    uint8_t      priority;                 /* Higher = more important */
    task_state_t state;
    uint32_t     wake_tick;                /* When a BLOCKED task wakes */
    uint32_t     stack[TASK_STACK_WORDS];
} tcb_t;

/* Used by PendSV_Handler in kernel/port/port.c */
extern volatile tcb_t *g_current_task;
extern volatile tcb_t *g_next_task;

/* Called by SysTick every 1 ms: wakes due sleepers and picks the next task.
 * Returns true if a context switch is needed. */
bool task_wake_and_reschedule(uint32_t now);

/* True once rtos_start() has run */
bool task_scheduler_is_running(void);

/* Implemented in kernel/port/port.c */
void port_start_first_task(void);
void port_trigger_pendsv(void);

#endif /* TASK_H */
