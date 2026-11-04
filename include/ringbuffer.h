#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Ring buffer over a caller-supplied array; holds up to capacity - 1 bytes */
typedef struct {
    uint8_t *buffer;
    size_t capacity;
    volatile size_t head;     /* Write index (producer/ISR) */
    volatile size_t tail;     /* Read index (consumer) */
} ringbuf_t;

/* `capacity` must be at least 2 */
void ringbuf_init(ringbuf_t *rb, uint8_t *buffer, size_t capacity);

/* Returns false if the buffer is full */
bool ringbuf_push(ringbuf_t *rb, uint8_t data);

/* Returns false if the buffer is empty */
bool ringbuf_pop(ringbuf_t *rb, uint8_t *data);

bool ringbuf_is_empty(const ringbuf_t *rb);
bool ringbuf_is_full(const ringbuf_t *rb);

#endif /* RINGBUF_H */
