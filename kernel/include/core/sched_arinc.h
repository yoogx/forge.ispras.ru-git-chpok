#ifndef __POK_SCHED_ARINC_H__
#define __POK_SCHED_ARINC_H__

#include <core/sched.h>

/* 
 * Local scheduler for ARINC partition.
 */

/**
 * Set `invalidate` flag for local scheduler.
 * 
 * This flag can be checked with pok_sched_local_check_invalidated().
 */
void pok_sched_local_invalidate(void);

/**
 * Return true if data for local scheduler has changed.
 * 
 * Clear `invalidate` flag for local scheduler.
 */
pok_bool_t pok_sched_local_check_invalidated(void);


/**
 *  Return true if new time slot is started.
 * 
 * Clear corresponded flag.
 * 
 * DEV: Intended to be used only within local scheduler.
 */
pok_bool_t pok_sched_local_check_slot_started(void);


/**
 * Begins critical section, local to given ARINC partition.
 * 
 * Only preemption between current partition's threads is disabled.
 * 
 * Preemption between partitions and between threads on other partitions
 * is still enabled.
 */
void pok_preemption_local_disable(void);

void pok_preemption_local_enable(void);


void pok_sched_arinc_on_time_changed(void);
void pok_sched_arinc_on_control_returned(void);

#endif /* ! __POK_SCHED_ARINC_H__ */
