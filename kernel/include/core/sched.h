/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

#ifndef __POK_SCHED_H__
#define __POK_SCHED_H__

#include <config.h>

#include <types.h>
#include <errno.h>
#include <core/schedvalues.h>

extern uint8_t pok_sched_current_slot;

typedef struct
{
    //pok_sched_slot_type_t type;

    uint64_t duration;
    uint64_t offset;

    pok_partition_t* partition;

    pok_bool_t periodic_processing_start;

    char name[128];
} pok_sched_slot_t;

// as usual, defined in deployment.c
extern const pok_sched_slot_t pok_module_sched[];

void pok_sched_init(void); /* Initialize scheduling stuff */


/**
 *  Starts critical section, when scheduler will not run.
 * 
 * Shouldn't be called from interrupt handler
 * (see pok_preemption_disable_from_interrupt below).
 * 
 * DEV: Currently critical sections are reentrant (pok_premption_disable,
 * can be called within critical section) but use feature this feature
 * only if it is really needed.
 */
void pok_preemption_disable(void);

/**
 *  Ends critical section, and runs scheduler.
 * 
 * Should be paired with pok_preemption_disable or
 * pok_preemption_disable_from_interrupt.
 * 
 * DEV: If ends *inner* critical section, scheduler is not run.
 */
void pok_preemption_enable(void);

/** 
 * Mark scheduler state as needed to rechecked.
 * 
 * If called outside of critical section, action will be taken at next
 * scheduler run.
 */
void pok_sched_invalidate(void);

/**
 * Disables preemption in interrupt handler.
 * 
 * If preemption is not disabled by normal context, disable it
 * and return TRUE.
 * 
 * Otherwise return FALSE.
 * 
 * DEV: If preemption will be implemented via disabling interrupts,
 * this will return TRUE always.
 */
pok_bool_t pok_preemption_disable_from_interrupt(void);

/**
 * Set `invalidate` flag for scheduler.
 * 
 * Can be called outside of critical section. So scheduler will found
 * the flag set on the next call.
 */
void pok_sched_invalidate(void)

/**
 * Set `invalidate` flag for local scheduler.
 * 
 * This flag can be checked with pok_sched_local_check_invalidated().
 * 
 * The flag is set automatically when time goes and when new time slot
 * for partition is started.
 */
void pok_sched_local_invalidate(void);

/**
 * Return true if data for local scheduler has changed.
 * 
 * Clear `invalidate` flag for local scheduler.
 * 
 * DEV: Intended to be used only within local scheduler.
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
 * Disable preemption before scheduler (re)start.
 * 
 * DEV: When kernel is initialized, this function should be called
 * before enabling interrupts. So, triggering timer interrupt will
 * not call scheduler until it will be ready.
 */
void pok_preemption_off(void);

/**
 * Start scheduler.
 * 
 * Never returns.
 * 
 * Should be called from context, different from ones used by partitions.
 * 
 * Assume partitions to be initialized at this step.
 */
void pok_sched_start(void);

/**
 * Retart scheduler.
 * 
 * Never returns.
 * 
 * Should be called from context, different from ones used by partitions.
 * 
 * Note, that partitions are not cleared here. You need to clear them before.
 */
void pok_sched_restart(void);

/**
 * Return next release point for periodic process in current partition.
 */
pok_time_t get_next_periodic_processing_start(void);

#endif /* !__POK_SCHED_H__ */
