#include "task.h"
#include "systick.h"
#include <stdint.h>

/* Interrupt Control and State Register: pends PendSV */
#define SCB_ICSR            (*(volatile uint32_t *)0xE000ED04UL)
#define SCB_ICSR_PENDSVSET  (1UL << 28)

/* System Handler Priority Register 3: bits [23:16] are PendSV's priority */
#define SCB_SHPR3           (*(volatile uint32_t *)0xE000ED20UL)
#define PENDSV_LOWEST_PRIO  (0xFFUL << 16)

void port_trigger_pendsv(void)
{
    SCB_ICSR = SCB_ICSR_PENDSVSET;
}

void port_start_first_task(void)
{
    /* Masking is fine: PendSV just stays pending (svc would HardFault here) */
    __asm volatile ("cpsid i" ::: "memory");

    SCB_SHPR3 |= PENDSV_LOWEST_PRIO;
    systick_init();

    __asm volatile (
        "mov r0, #0                 \n"
        "msr psp, r0                \n" /* PSP = 0 marks "no outgoing task" */
        "ldr r0, =_estack           \n"
        "msr msp, r0                \n" /* Reclaim main()'s stack for ISRs */
        ::: "r0", "memory"
    );

    port_trigger_pendsv();

    /* PendSV fires here and never returns */
    __asm volatile ("cpsie i \n isb" ::: "memory");

    while (1) {
    }
}

__attribute__((naked))
void PendSV_Handler(void)
{
    __asm volatile (
        "cpsid i                    \n"

        /* 1. Save the outgoing task (skipped on the first switch) */
        "mrs r0, psp                \n"
        "cbz r0, 1f                 \n" /* PSP == 0: no task has run yet */
        "stmdb r0!, {r4-r11}        \n"
        "ldr r1, =g_current_task    \n"
        "ldr r2, [r1]               \n"
        "str r0, [r2]               \n" /* g_current_task->sp = r0 */

        /* 2. g_current_task = g_next_task */
        "1:                         \n"
        "ldr r1, =g_current_task    \n"
        "ldr r3, =g_next_task       \n"
        "ldr r2, [r3]               \n"
        "str r2, [r1]               \n"

        /* 3. Restore the incoming task */
        "ldr r0, [r2]               \n" /* r0 = g_next_task->sp */
        "ldmia r0!, {r4-r11}        \n"
        "msr psp, r0                \n"

        /* 4. Return to Thread mode on PSP. The first switch interrupted main()
         *    on MSP (LR = 0xFFFFFFF9), so force bit 2. */
        "orr lr, lr, #0x04          \n"
        "cpsie i                    \n"
        "bx lr                      \n"
    );
}