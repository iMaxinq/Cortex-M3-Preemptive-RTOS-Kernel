#include <stdint.h>

/* Section boundaries from linker/linker.ld; only their addresses matter */
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

/* Weak: any handler not defined elsewhere falls back to Default_Handler */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

/* mps2-an385 UART0 interrupts */
void UART0RX_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void UART0TX_Handler(void)    __attribute__((weak, alias("Default_Handler")));

/* linker.ld places .isr_vector at 0x00000000 */
__attribute__((section(".isr_vector"), used))
const uintptr_t vector_table[] = {
    /* Cortex-M3 system exceptions */
    (uintptr_t)&_estack,            /* 0: Initial MSP */
    (uintptr_t)Reset_Handler,       /* 1 */
    (uintptr_t)NMI_Handler,         /* 2 */
    (uintptr_t)HardFault_Handler,   /* 3 */
    (uintptr_t)MemManage_Handler,   /* 4 */
    (uintptr_t)BusFault_Handler,    /* 5 */
    (uintptr_t)UsageFault_Handler,  /* 6 */
    0,                              /* 7: Reserved */
    0,                              /* 8: Reserved */
    0,                              /* 9: Reserved */
    0,                              /* 10: Reserved */
    (uintptr_t)SVC_Handler,         /* 11 */
    (uintptr_t)DebugMon_Handler,    /* 12 */
    0,                              /* 13: Reserved */
    (uintptr_t)PendSV_Handler,      /* 14 */
    (uintptr_t)SysTick_Handler,     /* 15 */

    /* External interrupts: entry 16 + IRQ number */
    (uintptr_t)UART0RX_Handler,     /* 16: IRQ 0 */
    (uintptr_t)UART0TX_Handler,     /* 17: IRQ 1 */
};

void Reset_Handler(void)
{
    /* Copy .data initial values from FLASH to RAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* Zero .bss */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0U;
    }

    main();

    while (1) {
    }
}

void Default_Handler(void)
{
    while (1) {
    }
}