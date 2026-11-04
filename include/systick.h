#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>


/* Starts the 1 ms SysTick interrupt */
void systick_init(void);

/* Ticks (ms) since systick_init() */
uint32_t systick_get_ticks(void);

#endif /* SYSTICK_H */
