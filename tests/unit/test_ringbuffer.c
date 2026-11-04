#include "unity.h"
#include "ringbuffer.h"
#include <stdint.h>

static uint8_t s_storage[4];
static ringbuf_t s_rb;

void setUp(void)
{
    ringbuf_init(&s_rb, s_storage, sizeof(s_storage));
}

void tearDown(void)
{
}

void test_ringbuf_starts_empty(void)
{
    uint8_t out = 0;
    TEST_ASSERT_TRUE(ringbuf_is_empty(&s_rb));
    TEST_ASSERT_FALSE(ringbuf_is_full(&s_rb));
    TEST_ASSERT_FALSE(ringbuf_pop(&s_rb, &out));
}

void test_ringbuf_push_pop_and_full(void)
{
    uint8_t out = 0;

    /* Capacity 4 holds at most 3 elements */
    TEST_ASSERT_TRUE(ringbuf_push(&s_rb, 'A'));
    TEST_ASSERT_TRUE(ringbuf_push(&s_rb, 'B'));
    TEST_ASSERT_TRUE(ringbuf_push(&s_rb, 'C'));

    TEST_ASSERT_TRUE(ringbuf_is_full(&s_rb));
    TEST_ASSERT_FALSE(ringbuf_push(&s_rb, 'D'));

    TEST_ASSERT_TRUE(ringbuf_pop(&s_rb, &out));
    TEST_ASSERT_EQUAL_UINT8('A', out);

    TEST_ASSERT_TRUE(ringbuf_pop(&s_rb, &out));
    TEST_ASSERT_EQUAL_UINT8('B', out);

    TEST_ASSERT_TRUE(ringbuf_pop(&s_rb, &out));
    TEST_ASSERT_EQUAL_UINT8('C', out);

    TEST_ASSERT_TRUE(ringbuf_is_empty(&s_rb));
}

void test_ringbuf_wrap_around(void)
{
    uint8_t out = 0;

    /* Advance head and tail to index 2 */
    TEST_ASSERT_TRUE(ringbuf_push(&s_rb, 10));
    TEST_ASSERT_TRUE(ringbuf_push(&s_rb, 20));
    TEST_ASSERT_TRUE(ringbuf_pop(&s_rb, &out));
    TEST_ASSERT_EQUAL_UINT8(10, out);
    TEST_ASSERT_TRUE(ringbuf_pop(&s_rb, &out));
    TEST_ASSERT_EQUAL_UINT8(20, out);

    /* Push 3 items to wrap head past index 3 -> 0 -> 1 */
    TEST_ASSERT_TRUE(ringbuf_push(&s_rb, 30));
    TEST_ASSERT_TRUE(ringbuf_push(&s_rb, 40));
    TEST_ASSERT_TRUE(ringbuf_push(&s_rb, 50));
    TEST_ASSERT_TRUE(ringbuf_is_full(&s_rb));

    TEST_ASSERT_TRUE(ringbuf_pop(&s_rb, &out));
    TEST_ASSERT_EQUAL_UINT8(30, out);
    TEST_ASSERT_TRUE(ringbuf_pop(&s_rb, &out));
    TEST_ASSERT_EQUAL_UINT8(40, out);
    TEST_ASSERT_TRUE(ringbuf_pop(&s_rb, &out));
    TEST_ASSERT_EQUAL_UINT8(50, out);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ringbuf_starts_empty);
    RUN_TEST(test_ringbuf_push_pop_and_full);
    RUN_TEST(test_ringbuf_wrap_around);
    return UNITY_END();
}