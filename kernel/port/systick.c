#include "systick.h"
#include "task.h"

/* =========================================================================
 * ARM Cortex-M3 SysTick Register Map (Base Address: 0xE000E010)
 * ========================================================================= */
typedef struct {
    volatile uint32_t CSR;   /* 0x00: Control and Status Register */
    volatile uint32_t RVR;   /* 0x04: Reload Value Register (24-bit) */
    volatile uint32_t CVR;   /* 0x08: Current Value Register (24-bit) */
    volatile uint32_t CALIB; /* 0x0C: Calibration Value Register */
} systick_regs_t;

#define SYSTICK_BASE        (0xE000E010UL)
#define SYSTICK             ((systick_regs_t *)SYSTICK_BASE)

/* SYST_CSR Bit Masks */
#define SYSTICK_CSR_ENABLE      (1UL << 0) /* Bit 0: Enable counter */
#define SYSTICK_CSR_TICKINT     (1UL << 1) /* Bit 1: Enable SysTick interrupt */
#define SYSTICK_CSR_CLKSOURCE   (1UL << 2) /* Bit 2: Clock source = CPU (25 MHz) */

/* Clock Constants for MPS2-AN385 */
#define SYSCLK_HZ           (25000000UL)
#define TICK_RATE_HZ        (1000UL)       /* 1000 Hz = 1 ms per tick */

/* Global tick counter — MUST be volatile because SysTick_Handler modifies it
 * asynchronously in the background while main() reads it! */
static volatile uint32_t s_ticks = 0;

void systick_init(void)
{
    /* disable SysTick during configuration */
    SYSTICK->CSR = 0UL;

    /* set reload value for 1 ms: (25,000,000 / 1,000) - 1 = 24,999 */
    SYSTICK->RVR = (SYSCLK_HZ / TICK_RATE_HZ) - 1UL;

    /* writing ANY value to CVR clears the counter to 0 and clears COUNTFLAG */
    SYSTICK->CVR = 0UL;

    /* select CPU clock (25 MHz), enable interrupt, and start the timer */
    SYSTICK->CSR = SYSTICK_CSR_CLKSOURCE |
                   SYSTICK_CSR_TICKINT   |
                   SYSTICK_CSR_ENABLE;
}

uint32_t systick_get_ticks(void)
{
    return s_ticks;
}

/* =========================================================================
 * SysTick Exception Handler (wired to vector_table[15] in startup.c)
 * Automatically called by hardware every 1.000 millisecond.
 * ========================================================================= */
void SysTick_Handler(void)
{
    s_ticks++;

    /* Every 1 ms tick, wake sleepers, pick the highest-priority ready task and pend PendSV */
    if (task_wake_and_reschedule(s_ticks)) {
        port_trigger_pendsv();
    }
}