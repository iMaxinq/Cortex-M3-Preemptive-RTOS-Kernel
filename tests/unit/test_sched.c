#include "unity.h"
#include "scheduler.h"
#include <stdint.h>

static tcb_t s_tasks[4];

/* Small helper so each test reads like a table of tasks */
static void set_task(uint32_t idx, uint8_t priority, task_state_t state)
{
    s_tasks[idx].pid       = idx;
    s_tasks[idx].priority  = priority;
    s_tasks[idx].state     = state;
    s_tasks[idx].wake_tick = 0U;
}

void setUp(void)
{
    for (uint32_t i = 0U; i < 4U; i++) {
        set_task(i, 1U, TASK_READY);
    }
}

void tearDown(void)
{
}

void test_pick_highest_priority_ready_task(void)
{
    set_task(0, 0U, TASK_READY);   /* idle */
    set_task(1, 1U, TASK_READY);
    set_task(2, 3U, TASK_READY);
    set_task(3, 2U, TASK_READY);

    TEST_ASSERT_EQUAL_UINT32(2, sched_pick_next(s_tasks, 4, 0));
    TEST_ASSERT_EQUAL_UINT32(2, sched_pick_next(s_tasks, 4, 3));
}

void test_pick_keeps_current_when_it_is_the_only_top_priority(void)
{
    set_task(0, 0U, TASK_READY);
    set_task(1, 5U, TASK_READY);
    set_task(2, 1U, TASK_READY);
    set_task(3, 1U, TASK_READY);

    TEST_ASSERT_EQUAL_UINT32(1, sched_pick_next(s_tasks, 4, 1));
}

void test_pick_skips_blocked_tasks(void)
{
    set_task(0, 0U, TASK_READY);
    set_task(1, 1U, TASK_READY);
    set_task(2, 9U, TASK_BLOCKED);
    set_task(3, 2U, TASK_READY);

    TEST_ASSERT_EQUAL_UINT32(3, sched_pick_next(s_tasks, 4, 2));
}

void test_pick_falls_back_to_idle_when_all_others_sleep(void)
{
    set_task(0, 2U, TASK_BLOCKED);
    set_task(1, 1U, TASK_BLOCKED);
    set_task(2, 3U, TASK_BLOCKED);
    set_task(3, 0U, TASK_READY);   /* idle */

    TEST_ASSERT_EQUAL_UINT32(3, sched_pick_next(s_tasks, 4, 2));
}

void test_pick_round_robins_equal_priorities(void)
{
    set_task(0, 0U, TASK_READY);   /* idle */
    set_task(1, 2U, TASK_READY);
    set_task(2, 1U, TASK_READY);   /* lower: never picked */
    set_task(3, 2U, TASK_READY);

    TEST_ASSERT_EQUAL_UINT32(3, sched_pick_next(s_tasks, 4, 1));
    TEST_ASSERT_EQUAL_UINT32(1, sched_pick_next(s_tasks, 4, 3));
    TEST_ASSERT_EQUAL_UINT32(1, sched_pick_next(s_tasks, 4, 0));
}

void test_pick_returns_current_when_nothing_is_ready(void)
{
    for (uint32_t i = 0U; i < 4U; i++) {
        set_task(i, 1U, TASK_BLOCKED);
    }

    TEST_ASSERT_EQUAL_UINT32(2, sched_pick_next(s_tasks, 4, 2));
}

void test_wake_only_tasks_whose_time_has_come(void)
{
    set_task(0, 1U, TASK_BLOCKED);
    s_tasks[0].wake_tick = 100U;
    set_task(1, 1U, TASK_BLOCKED);
    s_tasks[1].wake_tick = 101U;

    sched_wake_sleepers(s_tasks, 4, 99U);
    TEST_ASSERT_EQUAL(TASK_BLOCKED, s_tasks[0].state);

    sched_wake_sleepers(s_tasks, 4, 100U);
    TEST_ASSERT_EQUAL(TASK_READY, s_tasks[0].state);
    TEST_ASSERT_EQUAL(TASK_BLOCKED, s_tasks[1].state);
}

void test_wake_handles_tick_counter_wrap(void)
{
    set_task(0, 1U, TASK_BLOCKED);
    s_tasks[0].wake_tick = 5U;      /* scheduled just after the wrap */

    sched_wake_sleepers(s_tasks, 4, 0xFFFFFFF0UL);
    TEST_ASSERT_EQUAL(TASK_BLOCKED, s_tasks[0].state);

    sched_wake_sleepers(s_tasks, 4, 5U);
    TEST_ASSERT_EQUAL(TASK_READY, s_tasks[0].state);
}

void test_woken_high_priority_task_preempts_low_one(void)
{
    set_task(0, 0U, TASK_READY);   /* idle */
    set_task(1, 1U, TASK_READY);   /* low, currently running */
    set_task(2, 3U, TASK_BLOCKED); /* high, sleeping */
    s_tasks[2].wake_tick = 50U;
    set_task(3, 0U, TASK_BLOCKED);

    sched_wake_sleepers(s_tasks, 4, 49U);
    TEST_ASSERT_EQUAL_UINT32(1, sched_pick_next(s_tasks, 4, 1));

    sched_wake_sleepers(s_tasks, 4, 50U);
    TEST_ASSERT_EQUAL_UINT32(2, sched_pick_next(s_tasks, 4, 1));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_pick_highest_priority_ready_task);
    RUN_TEST(test_pick_keeps_current_when_it_is_the_only_top_priority);
    RUN_TEST(test_pick_skips_blocked_tasks);
    RUN_TEST(test_pick_falls_back_to_idle_when_all_others_sleep);
    RUN_TEST(test_pick_round_robins_equal_priorities);
    RUN_TEST(test_pick_returns_current_when_nothing_is_ready);
    RUN_TEST(test_wake_only_tasks_whose_time_has_come);
    RUN_TEST(test_wake_handles_tick_counter_wrap);
    RUN_TEST(test_woken_high_priority_task_preempts_low_one);
    return UNITY_END();
}
