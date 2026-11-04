#include "uart.h"
#include "ringbuffer.h"
#include <stdint.h>

/* CMSDK UART registers, memory-mapped (UART0 at 0x40004000 on mps2-an385) */
typedef struct {
    volatile uint32_t DATA;     /* 0x00: Write sends, read receives */
    volatile uint32_t STATE;    /* 0x04: TX/RX buffer status */
    volatile uint32_t CTRL;     /* 0x08: TX/RX and interrupt enables */
    volatile uint32_t INTCLEAR; /* 0x0C: Write 1 to clear an interrupt */
    volatile uint32_t BAUDDIV;  /* 0x10: Baud rate divider */
} cmsdk_uart_t;

#define UART0_BASE          (0x40004000UL)
#define UART0               ((cmsdk_uart_t *)UART0_BASE)

/* STATE register */
#define UART_STATE_TX_FULL  (1UL << 0)

/* CTRL register */
#define UART_CTRL_TX_EN     (1UL << 0)
#define UART_CTRL_RX_EN     (1UL << 1) 
#define UART_CTRL_RX_IRQ_EN (1UL << 3)

/* INTCLEAR register */
#define UART_INTCLEAR_TX    (1UL << 0)
#define UART_INTCLEAR_RX    (1UL << 1)

/* NVIC interrupt set-enable register for IRQs 0..31 */
#define NVIC_ISER0          (*(volatile uint32_t *)0xE000E100UL)
#define UART0_RX_IRQn       (0U) /* vector_table[16] in startup.c */

#define UART_RX_BUF_SIZE    (64U)
static uint8_t s_rx_buf[UART_RX_BUF_SIZE];
static ringbuf_t s_rx_rb; /* Filled by UART0RX_Handler, drained by uart_getc() */

void uart_init(void)
{
    ringbuf_init(&s_rx_rb, s_rx_buf, UART_RX_BUF_SIZE);

    /* 25 MHz / 115200 baud = 217 */
    UART0->BAUDDIV = 217UL;

    UART0->INTCLEAR = UART_INTCLEAR_RX;

    UART0->CTRL = UART_CTRL_TX_EN | UART_CTRL_RX_EN | UART_CTRL_RX_IRQ_EN;

    NVIC_ISER0 = (1UL << UART0_RX_IRQn);
}

void uart_putc(char c)
{
    /* Terminals need "\r\n" to return to column 0 */
    if (c == '\n') {
        while (UART0->STATE & UART_STATE_TX_FULL) {
        }
        UART0->DATA = (uint32_t)'\r';
    }

    while (UART0->STATE & UART_STATE_TX_FULL) {
    }

    UART0->DATA = (uint32_t)(uint8_t)c;
}

bool uart_getc(char *c)
{
    if (!c) {
        return false;
    }
    return ringbuf_pop(&s_rx_rb, (uint8_t *)c);
}

void UART0RX_Handler(void)
{
    uint8_t data = (uint8_t)(UART0->DATA & 0xFFUL);

    UART0->INTCLEAR = UART_INTCLEAR_RX;

    (void)ringbuf_push(&s_rx_rb, data); /* Dropped if the buffer is full */
}