#ifndef UART_H
#define UART_H

#include <stdbool.h>

/* Sets up UART0 at 115200 baud with the RX interrupt enabled */
void uart_init(void);

/* Sends one character, waiting while the TX buffer is full */
void uart_putc(char c);

/* Non-blocking: returns false if no character has been received */
bool uart_getc(char *c);

#endif /* UART_H */
