#include "scheduler.h"
#include <stdbool.h>

uint32_t sched_pick_next(const tcb_t *tasks, uint32_t num_tasks, uint32_t current)
{
    uint32_t best  = current;
    bool     found = false;

    /* Scan starting just after `current` and finish on `current` itself, so
     * a strict '>' comparison makes the first equal-priority task we meet win. */
    for (uint32_t i = 1U; i <= num_tasks; i++) {
        uint32_t idx = (current + i) % num_tasks;

        if (tasks[idx].state != TASK_READY) {
            continue;
        }
        if (!found || (tasks[idx].priority > tasks[best].priority)) {
            best  = idx;
            found = true;
        }
    }

    return best;
}

void sched_wake_sleepers(tcb_t *tasks, uint32_t num_tasks, uint32_t now)
{
    for (uint32_t i = 0U; i < num_tasks; i++) {
        /* Signed difference stays correct when the tick counter wraps */
        if ((tasks[i].state == TASK_BLOCKED) &&
            ((int32_t)(now - tasks[i].wake_tick) >= 0)) {
            tasks[i].state = TASK_READY;
        }
    }
}
