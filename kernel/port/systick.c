#include "systick.h"
#include "task.h"

/* SysTick registers */
typedef struct {
    volatile uint32_t CSR;   /* 0x00: Control and status */
    volatile uint32_t RVR;   /* 0x04: Reload value (24-bit) */
    volatile uint32_t CVR;   /* 0x08: Current value (24-bit) */
    volatile uint32_t CALIB; /* 0x0C: Calibration value */
} systick_regs_t;

#define SYSTICK_BASE        (0xE000E010UL)
#define SYSTICK             ((systick_regs_t *)SYSTICK_BASE)

/* CSR bits */
#define SYSTICK_CSR_ENABLE      (1UL << 0)
#define SYSTICK_CSR_TICKINT     (1UL << 1) /* Raise SysTick_Handler on wrap */
#define SYSTICK_CSR_CLKSOURCE   (1UL << 2) /* Count the CPU clock */

/* MPS2-AN385 clock */
#define SYSCLK_HZ           (25000000UL)
#define TICK_RATE_HZ        (1000UL)       /* 1 ms per tick */

/* Written by SysTick_Handler, read by tasks */
static volatile uint32_t s_ticks = 0;

void systick_init(void)
{
    /* Stop while configuring */
    SYSTICK->CSR = 0UL;

    /* Counts N..0, so N + 1 cycles per tick */
    SYSTICK->RVR = (SYSCLK_HZ / TICK_RATE_HZ) - 1UL;

    /* Any write clears the counter and COUNTFLAG */
    SYSTICK->CVR = 0UL;

    SYSTICK->CSR = SYSTICK_CSR_CLKSOURCE |
                   SYSTICK_CSR_TICKINT   |
                   SYSTICK_CSR_ENABLE;
}

uint32_t systick_get_ticks(void)
{
    return s_ticks;
}

void SysTick_Handler(void)
{
    s_ticks++;

    if (task_wake_and_reschedule(s_ticks)) {
        port_trigger_pendsv();
    }
}