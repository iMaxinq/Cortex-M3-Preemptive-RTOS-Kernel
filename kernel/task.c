#include "task.h"
#include "scheduler.h"
#include "systick.h"
#include <stddef.h>

/* Static pool of Task Control Blocks in .bss RAM */
static tcb_t    s_tasks[MAX_TASKS];
static uint32_t s_num_tasks = 0U;
static volatile bool s_scheduler_running = false;
static volatile uint32_t s_idle_ticks = 0U;
static tcb_t   *s_idle_task = NULL;

/* Pointers shared with PendSV_Handler assembly in kernel/port/port.c */
volatile tcb_t *g_current_task = NULL;
volatile tcb_t *g_next_task    = NULL;

/* =========================================================================
 * Safety Trap if a Task Function Ever Returns
 * ========================================================================= */
static void task_exit_trap(void)
{
    while (1) {
    }
}

/* =========================================================================
 * Idle Task: runs only when every other task is sleeping
 * ========================================================================= */
static void idle_task(void)
{
    while (1) {
        /* Halt the core until the next interrupt instead of burning cycles */
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

    /* Start at the top (highest memory address) of this task's stack array.
     * Mask with ~0x7UL to guarantee 8-byte stack alignment per ARM AAPCS. */
    uint32_t *sp = (uint32_t *)((uintptr_t)(&tcb->stack[TASK_STACK_WORDS]) & ~0x7UL);

    // initialize context registers
    /* ---------------------------------------------------------------------
     * 1. Forge the 8 Hardware-Saved Registers (popped automatically by CPU)
     * --------------------------------------------------------------------- */
    *(--sp) = 0x01000000UL;                  /* xPSR: Bit 24 = 1 (Thumb mode) */
    *(--sp) = (uint32_t)(uintptr_t)entry & ~0x1UL;    /* PC  : Task entry function     */
    *(--sp) = (uint32_t)(uintptr_t)task_exit_trap; /* LR: Trap if task returns */
    *(--sp) = 0x12121212UL;                  /* R12 : Initial debug marker    */
    *(--sp) = 0x03030303UL;                  /* R3  : Initial debug marker    */
    *(--sp) = 0x02020202UL;                  /* R2  : Initial debug marker    */
    *(--sp) = 0x01010101UL;                  /* R1  : Initial debug marker    */
    *(--sp) = 0x00000000UL;                  /* R0  : 1st argument OF ENTRY */

    /* ---------------------------------------------------------------------
     * 2. Forge the 8 Software-Saved Registers (popped by PendSV_Handler)
     * --------------------------------------------------------------------- */
    *(--sp) = 0x11111111UL;                  /* R11 : Initial debug marker    */
    *(--sp) = 0x10101010UL;                  /* R10 : Initial debug marker    */
    *(--sp) = 0x09090909UL;                  /* R9  : Initial debug marker    */
    *(--sp) = 0x08080808UL;                  /* R8  : Initial debug marker    */
    *(--sp) = 0x07070707UL;                  /* R7  : Initial debug marker    */
    *(--sp) = 0x06060606UL;                  /* R6  : Initial debug marker    */
    *(--sp) = 0x05050505UL;                  /* R5  : Initial debug marker    */
    *(--sp) = 0x04040404UL;                  /* R4  : Initial debug marker    */

    /* Save the final stack pointer (pointing right at R4!) into the TCB */
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

/* Choose the next task and report whether PendSV must switch to it.
 * Caller must run in an ISR or with interrupts disabled. */
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

    /* A task that wakes up here preempts a lower-priority one on this very tick */
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

    /* The pended PendSV fires right here and switches away; we resume
     * after this line once SysTick has woken us and we are picked again. */
    __asm volatile ("cpsie i \n isb" ::: "memory");
}

void rtos_start(void)
{
    /* The idle task needs one free slot and must be created last */
    if ((s_num_tasks == 0U) ||
        (task_create(idle_task, "idle", TASK_PRIORITY_IDLE) < 0)) {
        while (1) {
        }
    }
    s_idle_task = &s_tasks[s_num_tasks - 1U];

    /* Start with the most important task, not simply the first one created */
    uint32_t first = sched_pick_next(s_tasks, s_num_tasks, s_num_tasks - 1U);

    g_current_task     = &s_tasks[first];
    g_next_task        = &s_tasks[first];
    s_scheduler_running = true;

    /* Hand off to the ARM Cortex-M3 port to switch to PSP and launch the first task */
    port_start_first_task();
}
