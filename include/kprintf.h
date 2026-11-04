#ifndef KPRINTF_H
#define KPRINTF_H

/* Outputs one character */
typedef void (*custom_kputc_function)(char c);

/* Sets the output function (uart_putc on target, a buffer writer in tests) */
void kprintf_init(custom_kputc_function putc_fn);

/* Minimal printf supporting %c %s %d %u %x %p %% */
void kprintf(const char *fmt, ...);

#endif /* KPRINTF_H */
