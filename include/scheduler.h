#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "task.h"

/* Scheduling policy with no hardware access, so it can be unit-tested on the host */

/* Returns the index of the highest-priority READY task. Ties go to the first one
 * after `current`, giving round-robin. Returns `current` if none is READY. */
uint32_t sched_pick_next(const tcb_t *tasks, uint32_t num_tasks, uint32_t current);

/* Makes READY every BLOCKED task whose wake_tick has been reached (wrap-safe) */
void sched_wake_sleepers(tcb_t *tasks, uint32_t num_tasks, uint32_t now);

#endif /* SCHEDULER_H */
