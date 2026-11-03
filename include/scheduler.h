#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "task.h"

/* =========================================================================
 * Pure scheduling policy — no hardware access, so it is unit-testable on
 * the host (see tests/unit/test_sched.c).
 * ========================================================================= */

/**
 * @brief Pick the index of the task that should run next.
 *
 *        The highest-priority READY task wins. Among READY tasks that share
 *        the highest priority, the first one *after* `current` (wrapping)
 *        wins, so equal-priority tasks take turns (round-robin). `current`
 *        itself is only re-picked if no other task of that priority is ready.
 *
 * @param tasks      Task table.
 * @param num_tasks  Number of valid entries in `tasks` (must be > 0).
 * @param current    Index of the task running now.
 * @return           Index of the chosen task, or `current` if nothing is READY.
 */
uint32_t sched_pick_next(const tcb_t *tasks, uint32_t num_tasks, uint32_t current);

/**
 * @brief Move every BLOCKED task whose wake tick has been reached to READY.
 *        Handles 32-bit tick counter wrap-around.
 *
 * @param now  Current tick count.
 */
void sched_wake_sleepers(tcb_t *tasks, uint32_t num_tasks, uint32_t now);

#endif /* SCHEDULER_H */
