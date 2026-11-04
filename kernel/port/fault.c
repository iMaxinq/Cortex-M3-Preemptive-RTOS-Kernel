#include <stdint.h>
#include "kprintf.h"

/* Prints the frame the CPU stacked on exception entry and halts */
void hardfault_c_handler(uint32_t *stack_frame)
{
    uint32_t r0   = stack_frame[0];
    uint32_t r1   = stack_frame[1];
    uint32_t r2   = stack_frame[2];
    uint32_t r3   = stack_frame[3];
    uint32_t r12  = stack_frame[4];
    uint32_t lr   = stack_frame[5];
    uint32_t pc   = stack_frame[6];
    uint32_t xpsr = stack_frame[7];

    /* Configurable Fault Status and HardFault Status registers */
    uint32_t cfsr = *(volatile uint32_t *)0xE000ED28UL;
    uint32_t hfsr = *(volatile uint32_t *)0xE000ED2CUL;

    kprintf("\n========================================\n");
    kprintf("!!! HARDFAULT CRASH REPORT !!!\n");
    kprintf("========================================\n");
    kprintf("  PC   (Crash Addr) : 0x%x\n", pc);
    kprintf("  LR   (Caller Addr): 0x%x\n", lr);
    kprintf("  xPSR (Status Reg) : 0x%x\n", xpsr);
    kprintf("  R0                : 0x%x\n", r0);
    kprintf("  R1                : 0x%x\n", r1);
    kprintf("  R2                : 0x%x\n", r2);
    kprintf("  R3                : 0x%x\n", r3);
    kprintf("  R12               : 0x%x\n", r12);
    kprintf("  CFSR / HFSR       : 0x%x / 0x%x\n", cfsr, hfsr);
    kprintf("========================================\n");
    kprintf("System Halted.\n");

    while (1) {
    }
}

/* Naked so SP is untouched; LR bit 2 tells whether the frame is on MSP or PSP */
__attribute__((naked))
void HardFault_Handler(void)
{
    __asm volatile (
        "tst lr, #4                 \n"
        "ite eq                     \n"
        "mrseq r0, msp              \n"
        "mrsne r0, psp              \n"
        "b hardfault_c_handler      \n"
    );
}