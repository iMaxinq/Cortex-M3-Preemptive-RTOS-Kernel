#include "task.h"
#include "scheduler.h"
#include "systick.h"
#include <stddef.h>

/* Static TCB pool (no heap) */
static tcb_t    s_tasks[MAX_TASKS];
static uint32_t s_num_tasks = 0U;
static volatile bool s_scheduler_running = false;
static volatile uint32_t s_idle_ticks = 0U;
static tcb_t   *s_idle_task = NULL;

/* Shared with PendSV_Handler in kernel/port/port.c */
volatile tcb_t *g_current_task = NULL;
volatile tcb_t *g_next_task    = NULL;

/* Return address of every task; tasks must never return */
static void task_exit_trap(void)
{
    while (1) {
    }
}

/* Runs only when no other task is READY */
static void idle_task(void)
{
    while (1) {
        /* Sleep the core until the next interrupt */
        __asm volatile ("wfi");
    }
}

int task_create(task_func_t entry, const char *name, uint8_t priority)
{
    if ((entry == NULL) || (s_num_tasks >= MAX_TASKS)) {
        return -1;
    }

    tcb_t *tcb = &s_tasks[s_num_tasks];
    tcb->pid   = s_num_tasks;
    tcb->name  = (name != NULL) ? name : "unnamed";
    tcb->priority  = priority;
    tcb->state     = TASK_READY;
    tcb->wake_tick = 0U;

    /* Top of the stack, 8-byte aligned as AAPCS requires */
    uint32_t *sp = (uint32_t *)((uintptr_t)(&tcb->stack[TASK_STACK_WORDS]) & ~0x7UL);

    /* Fake exception frame, popped by hardware; R1-R12 hold debug markers */
    *(--sp) = 0x01000000UL;                  /* xPSR: Thumb bit */
    *(--sp) = (uint32_t)(uintptr_t)entry & ~0x1UL;    /* PC */
    *(--sp) = (uint32_t)(uintptr_t)task_exit_trap; /* LR */
    *(--sp) = 0x12121212UL;                  /* R12 */
    *(--sp) = 0x03030303UL;                  /* R3 */
    *(--sp) = 0x02020202UL;                  /* R2 */
    *(--sp) = 0x01010101UL;                  /* R1 */
    *(--sp) = 0x00000000UL;                  /* R0: entry argument */

    /* Registers popped by PendSV_Handler */
    *(--sp) = 0x11111111UL;                  /* R11 */
    *(--sp) = 0x10101010UL;                  /* R10 */
    *(--sp) = 0x09090909UL;                  /* R9 */
    *(--sp) = 0x08080808UL;                  /* R8 */
    *(--sp) = 0x07070707UL;                  /* R7 */
    *(--sp) = 0x06060606UL;                  /* R6 */
    *(--sp) = 0x05050505UL;                  /* R5 */
    *(--sp) = 0x04040404UL;                  /* R4 */

    /* PendSV_Handler starts restoring from R4 */
    tcb->sp = sp;

    s_num_tasks++;
    return (int)tcb->pid;
}

bool task_scheduler_is_running(void)
{
    return s_scheduler_running;
}

uint32_t task_get_idle_ticks(void)
{
    return s_idle_ticks;
}

/* Picks the next task; true if PendSV must switch to it. Interrupts must be off. */
static bool task_reschedule(void)
{
    uint32_t next = sched_pick_next(s_tasks, s_num_tasks, g_current_task->pid);

    g_next_task = &s_tasks[next];
    return (g_next_task != g_current_task);
}

bool task_wake_and_reschedule(uint32_t now)
{
    if (!s_scheduler_running) {
        return false;
    }

    if (g_current_task == s_idle_task) {
        s_idle_ticks++;
    }

    /* A task woken here preempts a lower-priority one on this same tick */
    sched_wake_sleepers(s_tasks, s_num_tasks, now);
    return task_reschedule();
}

void task_sleep(uint32_t ms)
{
    __asm volatile ("cpsid i" ::: "memory");

    tcb_t *self = (tcb_t *)g_current_task;
    if (ms > 0U) {
        self->wake_tick = systick_get_ticks() + ms;
        self->state     = TASK_BLOCKED;
    }

    if (task_reschedule()) {
        port_trigger_pendsv();
    }

    /* The pended switch happens here; we resume once woken and picked again */
    __asm volatile ("cpsie i \n isb" ::: "memory");
}

void rtos_start(void)
{
    /* The idle task takes the last free slot */
    if ((s_num_tasks == 0U) ||
        (task_create(idle_task, "idle", TASK_PRIORITY_IDLE) < 0)) {
        while (1) {
        }
    }
    s_idle_task = &s_tasks[s_num_tasks - 1U];

    /* Start with the highest-priority task, not the first one created */
    uint32_t first = sched_pick_next(s_tasks, s_num_tasks, s_num_tasks - 1U);

    g_current_task     = &s_tasks[first];
    g_next_task        = &s_tasks[first];
    s_scheduler_running = true;

    port_start_first_task();
}
