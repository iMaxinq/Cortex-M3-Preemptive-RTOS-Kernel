#include "ringbuffer.h"

void ringbuf_init(ringbuf_t *rb, uint8_t *buffer, size_t capacity)
{
    if (!rb || !buffer || capacity < 2U) {
        return;
    }
    rb->buffer = buffer;
    rb->capacity = capacity;
    rb->head = 0U;
    rb->tail = 0U;
}

bool ringbuf_is_empty(const ringbuf_t *rb)
{
    return rb->head == rb->tail;
}

bool ringbuf_is_full(const ringbuf_t *rb)
{
    return ((rb->head + 1U) % rb->capacity) == rb->tail;
}

bool ringbuf_push(ringbuf_t *rb, uint8_t data)
{
    size_t next_head = (rb->head + 1U) % rb->capacity;

    if (next_head == rb->tail) {
        return false; /* Full */
    }

    rb->buffer[rb->head] = data;
    rb->head = next_head;
    return true;
}

bool ringbuf_pop(ringbuf_t *rb, uint8_t *data)
{
    if (rb->head == rb->tail) {
        return false; /* Empty */
    }

    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1U) % rb->capacity;
    return true;
}