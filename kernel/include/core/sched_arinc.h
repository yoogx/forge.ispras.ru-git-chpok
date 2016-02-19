#ifndef __POK_SCHED_ARINC_H__
#define __POK_SCHED_ARINC_H__

#include <core/sched.h>

/* 
 * Local scheduler for ARINC partition.
 */

/**
 * Begins critical section, local to given ARINC partition.
 * 
 * Only preemption between current partition's threads is disabled.
 * 
 * Preemption between partitions and between threads on other partitions
 * is still enabled.
 * 
 * Shouldn't be called from interrupt handler.
 */
void pok_preemption_local_disable(void);

void pok_preemption_local_enable(void);


/**
 * Disable local preemption, may be called from interrupt handler.
 * 
 * If local preemption is enabled, disable it and return TRUE.
 * Otherwise return FALSE.
 * 
 * DEV: This function MAY be called from normal context. But if normal
 * context disables preemption, it will return FALSE. That is, this
 * function is not reentrant.
 * 
 * The main intention for the function to be automatically called after
 * global scheduler, when enable global preemption.
 */
pok_bool_t pok_preemption_local_disable_from_interrupt(void);


pok_ret_t pok_sched_end_period(void);
pok_ret_t pok_sched_replenish(int64_t budget);

uint32_t pok_sched_get_current(pok_thread_id_t *thread_id);


#endif /* ! __POK_SCHED_ARINC_H__ */
