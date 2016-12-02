/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#ifndef __POK_THREAD_INTERNAL_H__
#define __POK_THREAD_INTERNAL_H__

#include <types.h>

#include <core/thread.h>
#include <core/partition_arinc.h>
#include <core/time.h>

/* 
 * Functions for ARINC threads, used only in implementation of the
 * kernel itself.
 */

/*
 * Initialize thread structure.
 * 
 * Return TRUE on success, FALSE if failed to allocate stack.
 * 
 * Before given function, next fields should be initialized manually:
 * 
 * - name  [NAME]
 * - entry [ENTRY_POINT]
 * - base_priority [BASE_PRIORITY]
 * - period [PERIOD]
 * - time_capacity [TIME_CAPACITY]
 * - deadline [DEADLINE]
 * - user_stack_size [STACK_SIZE]
 * 
 * Should be executed with local preemption disabled.
 */
pok_bool_t thread_create(pok_thread_t* t);


/*
 * Postpone event for given thread.
 * 
 * Called with local preemption disabled.
 */
void thread_delay_event(pok_thread_t* t, pok_time_t delay_time,
	void (*thread_delayed_func)(pok_thread_t* t));


/*
 * Cancel postponed event for given thread, if it was.
 * 
 * Called with local preemption disabled.
 */
void thread_delay_event_cancel(pok_thread_t* t);

/*
 * Set deadline for given thread.
 * 
 * Called with local preemption disabled.
 */
void thread_set_deadline(pok_thread_t* t, pok_time_t deadline_time);

/* 
 * Mark thread as waiting on any condition except suspension.
 * 
 * Called with local preemption disabled.
 */
void thread_wait(pok_thread_t* t);


/*
 * Turn current thread to wait until specified time.
 * 
 * Should be called with local preemption disabled.
 */
void thread_wait_timed(pok_thread_t *thread, pok_time_t time);


/* 
 * Turn thread to wait for given duration, or, if duration is
 * infinite, forever.
 * 
 * Convinience function, used when implements ports and intra-partition
 * communication mechanisms.
 * 
 * Should be called with local preemption disabled.
 * 
 * NOTE: duration shouldn't be 0.
 */
static inline void thread_wait_common(pok_thread_t* t, pok_time_t duration)
{
    assert(duration != 0);

    if(pok_time_is_infinity(duration)) {
        thread_wait(t);
    }
    else {
        thread_wait_timed(t, jet_system_time() + duration);
    }
}

/*
 * Awoke given thread.
 * 
 * This function can be used when:
 *   1. Thread sleeps (on `thread_delayed_event`),
 *      and sleeping time is over.
 *   2. Thread waits (on `wait_elem`), and waited object become
 *      accesible.
 *   3. Thread waits (on `wait_elem`), and timeout
 *      (on `thread_delayed_event`) is fired.
 * 
 *   4. Thread waits in INIT_* mode, and mode has been set to NORMAL.
 *      (All previouse cases related to NORMAL mode).
 * 
 * NOE: It is assumed that thread is alredy delete itself from queue
 * which is fired.
 * 
 * Called with local preemption disabled.
 * 
 * May be used as callback.
 */
void thread_wake_up(pok_thread_t* t);

/* 
 * Suspend given thread.
 * 
 * Thread shouldn't be suspended already.
 */
void thread_suspend(pok_thread_t* t);

/* 
 * Suspend given thread for given period of time.
 * 
 * ARINC allows timed suspension only for current thread, so
 * suspended thead doesn't wait for anything else.
 * 
 * Thread shouldn't be suspended already.
 */
void thread_suspend_timed(pok_thread_t* t, pok_time_t time);

/* 
 * Resume given thread.
 * 
 * Thread should be suspended.
 */
void thread_resume(pok_thread_t* t);

/*
 * Stop given thread in NORMAL mode.
 * 
 * Called with local preemption disabled.
 * 
 * Note: For stop error handler additional actions should be taken.
 */
void thread_stop(pok_thread_t* t);

/* 
 * Whether waiting is allowed in the current context.
 * 
 * Note: Doesn't require disabled local preemption.
 * 
 * DEV: Not suitable for check waiting on msection.
 */
pok_bool_t thread_is_waiting_allowed(void);

/* 
 * Mark given thread as runnable in NORMAL mode.
 * 
 * Thread shouldn't wait on any time/resource.
 * 
 * Called with local preemption disabled.
 */
void thread_start(pok_thread_t* t);

/*
 * If thread is eligible, reorder it in the eligible queue after
 * other threads with same priority.
 * 
 * May be called after changing thread's priority.
 * 
 * Called with local preemption disabled.
 */
void thread_yield(pok_thread_t *t);
#endif /* __POK_THREAD_INTERNAL_H__ */
