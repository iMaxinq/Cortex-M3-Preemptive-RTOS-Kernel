#include "kprintf.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static custom_kputc_function kputc_current = NULL;

void kprintf_init(custom_kputc_function kputc_passed)
{
    kputc_current = kputc_passed;
}

static void kputc(char c)
{
    if (kputc_current) {
        kputc_current(c);
    }
}

static void kputs(const char *str)
{
    if (!str) {
        str = "(null)";
    }
    while (*str) {
        kputc(*str++);
    }
}

static void kprint_uint(uint32_t num, uint32_t base)
{
    char buf[12];  /* Fits any 32-bit value */
    const char *digits = "0123456789abcdef";
    int i = 0;

    if (num == 0U) {
        kputc('0');
        return;
    }

    while (num > 0U) {
        buf[i++] = digits[num % base];
        num /= base;
    }

    while (i > 0) {
        kputc(buf[--i]);
    }
}

static void kprint_int(int32_t num)
{
    if (num < 0) {
        kputc('-');
        /* Negate in unsigned arithmetic so INT32_MIN works */
        kprint_uint(0U - (uint32_t)num, 10U);
    } else {
        kprint_uint((uint32_t)num, 10U);
    }
}

void kprintf(const char *format_string, ...)
{
    if (!format_string || !kputc_current) {
        return;
    }

    va_list args;
    va_start(args, format_string);

    while (*format_string) {
        if (*format_string != '%') {
            kputc(*format_string++);
            continue;
        }

        format_string++;
        if (*format_string == '\0') {
            break;
        }

        switch (*format_string) {
            case 'c': {
                char c = (char)va_arg(args, int);
                kputc(c);
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);
                kputs(s);
                break;
            }
            case 'd': {
                int32_t d = (int32_t)va_arg(args, int);
                kprint_int(d);
                break;
            }
            case 'u': {
                uint32_t u = (uint32_t)va_arg(args, unsigned int);
                kprint_uint(u, 10U);
                break;
            }
            case 'x': {
                uint32_t x = (uint32_t)va_arg(args, unsigned int);
                kprint_uint(x, 16U);
                break;
            }
            case 'p': {
                uintptr_t p = (uintptr_t)va_arg(args, void *);
                kputs("0x");
                kprint_uint((uint32_t)p, 16U);
                break;
            }
            case '%': {
                kputc('%');
                break;
            }
            default: {
                kputc('%');
                kputc(*format_string);
                break;
            }
        }
        format_string++;
    }

    va_end(args);
}