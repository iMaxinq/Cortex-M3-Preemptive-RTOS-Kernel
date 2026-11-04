#include "minirtos.h"
#include "uart.h"
#include "kprintf.h"
#include "systick.h"
#include <stdint.h>
#include <stdbool.h>

/* Set while the low-priority task is in the middle of its busy work, so the
 * high-priority task can prove it preempted it. */
static volatile bool s_low_busy = false;

/* Burn CPU for `ticks` ms without sleeping (simulates real computation) */
static void busy_work(uint32_t ticks)
{
    uint32_t start = systick_get_ticks();
    while ((systick_get_ticks() - start) < ticks) {
    }
}

/* Priority 3: wakes every 300 ms and reports how late it was scheduled */
static void task_high(void)
{
    while (1) {
        uint32_t due = systick_get_ticks() + 300U;
        task_sleep(300U);
        uint32_t now = systick_get_ticks();
        kprintf("[HIGH] woke at tick %u, late by %u ticks%s\n",
                now, now - due, s_low_busy ? " (preempted LOW mid-work)" : "");
    }
}

/* Priority 2: every second, reports how much of that second the CPU was idle */
static void task_monitor(void)
{
    uint32_t last_idle = task_get_idle_ticks();
    while (1) {
        task_sleep(1000U);
        uint32_t idle = task_get_idle_ticks();
        kprintf("[MON ] idle %u of last 1000 ticks\n", idle - last_idle);
        last_idle = idle;
    }
}

/* Priority 1: 200 ms of busy work, then 300 ms of sleep (~40% CPU load) */
static void task_low(void)
{
    uint32_t round = 0;
    while (1) {
        s_low_busy = true;
        busy_work(200U);
        s_low_busy = false;
        kprintf("[LOW ] finished work round %u\n", round++);
        task_sleep(300U);
    }
}

int main(void)
{
    uart_init();
    kprintf_init(uart_putc);

    kprintf("========================================\n");
    kprintf("Booting miniRTOS - Milestone %d\n", 5);
    kprintf("Priorities: HIGH=3, MON=2, LOW=1, idle=0\n");
    kprintf("========================================\n");

    task_create(task_low, "Low", 1U);
    task_create(task_monitor, "Monitor", 2U);
    task_create(task_high, "High", 3U);

    rtos_start();

    return 0;
}
