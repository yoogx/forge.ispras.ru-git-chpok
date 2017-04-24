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

#include <config.h>

#include <types.h>

#include <core/debug.h>
#include <core/error.h>
#include <core/thread.h>
#include <core/sched_arinc.h>
#include <core/partition.h>
#include <core/time.h>
#include <libc.h>

#include "thread_internal.h"
#include <core/uaccess.h>

#include <system_limits.h>
#include <core/syscall.h>


/*
 * Find thread by name.
 *
 * Note: Doesn't require disable local preemption.
 *
 * Note: Name should be located in kernel space.
 */
static pok_thread_t* find_thread(const char name[MAX_NAME_LENGTH])
{
    pok_partition_arinc_t* part = current_partition_arinc;

    pok_thread_t* t;
    pok_thread_t* t_end = part->threads + part->nthreads_used;

    for(t = &part->threads[POK_PARTITION_ARINC_MAIN_THREAD_ID + 1];
        t != t_end;
        t++)
    {
        if(part->thread_error == t) continue; /* error thread is not searchable. */
        if(!pok_compare_names(t->name, name)) return t;
    }

    return NULL;
}

/*
 * Return thread by id.
 *
 * Return NULL if no such thread created.
 *
 * Note: Doesn't require disable local preemption.
 */
static pok_thread_t* get_thread_by_id(pok_thread_id_t id)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    if(id == 0 /* main thread have no id*/
        || id >= part->nthreads_used /* thread is not created yet */
        || part->thread_error == &part->threads[id] /* error thread has no id */
        )
        return NULL;

    return &part->threads[id];
}


/*
 * Create thread with given name and parameters.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - JET_INVALID_MODE - current partition mode is NORMAL
 *  - EFAULT - 'name', 'attr' or 'thread_id' points to inaccessible memory.
 *  - JET_INVALID_CONFIG
 *    -- no more threads can be created
 *    -- thread's stack of given size cannot be created
 *    -- thread's period isn't multiplied of partition's period
 *  - EINVAL
 *    -- thread's period is 0
 *    -- thread's capacity is 0
 *    -- or thread's period is given (positive), but capacity is not or
 *       capacity is less than period.
 *    -- incorrect value for base_priority has been given
 *    -- entry point cannot be executed [NON-ARINC]
 *  - EEXIST - thread with given name already exists.
 */
jet_ret_t pok_thread_create (const char* __user name,
    void* __user                    entry,
    const pok_thread_attr_t* __user attr,
    pok_thread_id_t* __user         thread_id)
{
    pok_thread_t* t;
    pok_partition_arinc_t* part = current_partition_arinc;

    /*
     * We can create a thread only if the partition is in INIT mode
     */
    if (part->mode == POK_PARTITION_MODE_NORMAL) {
        return JET_INVALID_MODE;
    }

    const pok_thread_attr_t* __kuser k_attr = jet_user_to_kernel_typed_ro(attr);
    if(!k_attr) return EFAULT;

    pok_thread_id_t* __kuser k_thread_id = jet_user_to_kernel_typed(thread_id);
    if(!k_thread_id) return EFAULT;

    const char* __kuser k_name = jet_user_to_kernel_ro(name, MAX_NAME_LENGTH);
    if(!k_name) return EFAULT;

    // Check quota for normal threads
    if (part->nthreads_normal_used == part->nthreads_normal) {
        return JET_INVALID_CONFIG;
    }

    assert(part->nthreads_used < part->nthreads);

    t = &part->threads[part->nthreads_used];

    t->entry = entry;
    t->base_priority = k_attr->priority;
    t->period = k_attr->period;
    t->time_capacity = k_attr->time_capacity;
    t->deadline = k_attr->deadline;

    if (t->base_priority > MAX_PRIORITY_VALUE ||
        t->base_priority < MIN_PRIORITY_VALUE) return EINVAL;

    if (t->period == 0) {
        return EINVAL;
    }
    if (t->time_capacity == 0) {
        return EINVAL;
    }

    if(!pok_time_is_infinity(t->period))
    {
        if(pok_time_is_infinity(t->time_capacity)) {
            // periodic process must have definite time capacity
            return EINVAL;
        }

        if(t->time_capacity > t->period) {
            // for periodic process, time capacity <= period
            return EINVAL;
        }


        if(t->period % part->base_part.period) {
            // Process period should be multiple to partition's period.
            return JET_INVALID_CONFIG;
        }
   }

    // do at least basic check of entry point
    if (!jet_check_access_exec(t->entry)) {
        return EINVAL;
    }

    memcpy(t->name, k_name, MAX_NAME_LENGTH);

    if(find_thread(t->name)) return EEXIST;

    t->user_stack_size = k_attr->stack_size;

    if(!thread_create(t)) return JET_INVALID_CONFIG;

    *k_thread_id = part->nthreads_used;

    part->nthreads_used++;
    part->nthreads_normal_used++;

    return EOK;
}

/*
 * Turn current thread into the sleep for a while or forever.
 *
 * (ARINC prohibits sleeping without timeout; this should be checked in user space).
 *
 * Return EOK if time is zero and thread yielding has been performed.
 * Return ETIMEDOUT if timeout expires.
 * Return JET_CANCELLED if thread has been STOP()ed or IPPC handler has
 * been cancelled.
 *
 * Possible error codes:
 *
 *  - JET_INVALID_MODE - thread is not allowed to sleep.
 *  - EFAULT - 'time' points to inaccessible memory.
 */
jet_ret_t pok_thread_sleep(const pok_time_t* __user time)
{
    if(!thread_is_waiting_allowed()) return JET_INVALID_MODE;

    const pok_time_t* __kuser k_time = jet_user_to_kernel_typed_ro(time);
    if(!k_time) return EFAULT;
    pok_time_t kernel_time = *k_time;

    pok_preemption_local_disable();

    if(kernel_time == 0) {
        thread_yield(current_thread);
        current_thread->wait_result = EOK;
    }
    else {
        thread_wait_common(current_thread, kernel_time);
    }

    pok_preemption_local_enable();

    return current_thread->wait_result;
}

/*
 * Give a chance for other threads to execute.
 *
 * Return EOK.
 */
jet_ret_t pok_thread_yield (void)
{
    pok_preemption_local_disable();
    thread_yield(current_thread);
    pok_preemption_local_enable();

    return EOK;
}


// Called with local preemption disabled
static jet_ret_t thread_delayed_start_internal (pok_thread_t* thread,
                                                pok_time_t delay)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    assert(!pok_time_is_infinity(delay));

    if(thread->ippc_server_connection != NULL) {
        return JET_INVALID_MODE_TARGET;
    }

    if (thread->state != POK_STATE_STOPPED) {
        return JET_NOACTION;
    }

    if (!pok_time_is_infinity(thread->period) && delay >= thread->period) {
        return EINVAL;
    }

    thread_start_prepare(thread);

    if(part->mode != POK_PARTITION_MODE_NORMAL)
    {
        /* Delay thread's starting until normal mode. */
        thread->delayed_time = delay;
        thread->state = POK_STATE_WAITING;

        return EOK;
    }

    // Normal mode.
    thread_start_normal(thread, delay);

    return EOK;
}

/*
 * Start given thread with a delay.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL
 *    -- thread identificator is incorrect.
 *    -- 'delay_time' is infinite.
 *    -- thread is periodic, and 'delay_time' is more than period.
 *  - EFAULT - 'delay_time' points to inaccessible memory.
 *  - JET_NOACTION - thread is not in DORMANT state.
 *  - [IPPC] JET_INVALID_MODE_TARGET - thread is an IPPC handler.
 *
 */
jet_ret_t pok_thread_delayed_start (pok_thread_id_t id,
    const pok_time_t* __user delay_time)
{
    jet_ret_t ret;

    pok_thread_t *thread = get_thread_by_id(id);
    if(!thread) return EINVAL;

    const pok_time_t* __kuser k_delay_time = jet_user_to_kernel_typed_ro(delay_time);
    if(!k_delay_time) return EFAULT;
    pok_time_t kernel_delay_time = *k_delay_time;

    if (pok_time_is_infinity(kernel_delay_time))
        return EINVAL;

    pok_preemption_local_disable();
    ret = thread_delayed_start_internal(thread, kernel_delay_time);
    pok_preemption_local_enable();

    return ret;
}

/*
 * Start given thread.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL
 *    -- thread identificator is incorrect.
 *  - JET_NOACTION - thread is not in DORMANT state.
 *  - [IPPC] JET_INVALID_MODE_TARGET - thread is an IPPC handler.
 */
jet_ret_t pok_thread_start (pok_thread_id_t id)
{
    jet_ret_t ret;

    pok_thread_t *thread = get_thread_by_id(id);
    if(!thread) return EINVAL;

    pok_preemption_local_disable();
    ret = thread_delayed_start_internal(thread, 0);
    pok_preemption_local_enable();

    return ret;
}

/*
 * Get status of given thread.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL
 *    -- thread identificator is incorrect.
 *  - EFAULT - 'name', 'entry' or 'status' points to inaccessible memory.
 */
jet_ret_t pok_thread_get_status (pok_thread_id_t id,
    char* __user name,
    void* __user *entry,
    pok_thread_status_t* __user status)
{
    pok_thread_t *t = get_thread_by_id(id);
    if(!t) return EINVAL;

    pok_thread_status_t* __kuser k_status = jet_user_to_kernel_typed(status);
    if(!k_status) return EFAULT;

    void* __kuser *k_entry = jet_user_to_kernel_typed(entry);
    if(!k_entry) return EFAULT;

    char* __kuser k_name = jet_user_to_kernel(name, MAX_NAME_LENGTH);
    if(!k_name) return EFAULT;

    memcpy(k_name, t->name, MAX_NAME_LENGTH);
    *k_entry = t->entry;

    k_status->attributes.priority = t->base_priority;
    k_status->attributes.period = t->period;
    k_status->attributes.deadline = t->deadline;
    k_status->attributes.time_capacity = t->time_capacity;
    k_status->attributes.stack_size = t->user_stack_size;

    pok_preemption_local_disable();

    k_status->current_priority = t->priority;

    if(t->state == POK_STATE_RUNNABLE)
    {
        if(t->suspended)
            k_status->state = POK_STATE_WAITING;
        else if(current_partition_arinc->thread_current == t)
            k_status->state = POK_STATE_RUNNING;
        else
            k_status->state = t->state;
    } else
        k_status->state = t->state;

    if(pok_time_is_infinity(t->time_capacity))
        k_status->deadline_time = POK_TIME_INFINITY;
    else
        k_status->deadline_time = t->thread_deadline_event.timepoint;

    pok_preemption_local_enable();

    return EOK;
}

/*
 * Set priority for given thread.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL
 *    -- thread identificator is incorrect
 *    -- priority value is incorrect
 *  - JET_INVALID_MODE_TARGET - thread is not started.
 *  - [IPPC] JET_INVALID_MODE_TARGET - thread is an IPPC handler.
 */
jet_ret_t pok_thread_set_priority(pok_thread_id_t id, uint32_t priority)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    jet_ret_t ret;

    pok_thread_t *t = get_thread_by_id(id);
    if(!t) return EINVAL;

    if(t->ippc_server_connection) return JET_INVALID_MODE_TARGET; // Priority has no sence for server threads.

    if(priority > MAX_PRIORITY_VALUE) return EINVAL;
    if(priority < MIN_PRIORITY_VALUE) return EINVAL;

    pok_preemption_local_disable();

    ret = JET_INVALID_MODE_TARGET;
    if (t->state == POK_STATE_STOPPED) goto out;

    t->priority = priority;

    struct jet_thread_shared_data* tshd = part->kshd->tshd
        + (t - part->threads);
    tshd->priority = priority;

    thread_yield(t);

    ret = EOK;
out:
    pok_preemption_local_enable();

    return ret;
}

/*
 * Resume given thread.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL
 *    -- thread identificator is incorrect
 *    -- thread is a current thread
 *  - JET_INVALID_MODE_TARGET
 *    -- thread is not started
 *    -- thread is periodic
 *  - JET_NOACTION - thread is not suspended
 */
jet_ret_t pok_thread_resume(pok_thread_id_t id)
{
    jet_ret_t ret;

    pok_thread_t *t = get_thread_by_id(id);
    if(!t) return EINVAL;

    // can't resume self, lol
    if (t == current_thread) return EINVAL;

    // although periodic process can never be suspended anyway,
    // ARINC-653 requires different error code
    if (pok_thread_is_periodic(t)) return JET_INVALID_MODE_TARGET;

    pok_preemption_local_disable();

    ret = JET_INVALID_MODE_TARGET;
    if (t->state == POK_STATE_STOPPED) goto out;

    ret = JET_NOACTION;
    if (!t->suspended) goto out;

    thread_resume(t);

    ret = EOK;
out:
    pok_preemption_local_enable();

    return ret;
}

/*
 * Suspend given thread.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL
 *    -- thread identificator is incorrect
 *    -- thread is the current thread
 *  - JET_INVALID_MODE_TARGET
 *    -- thread is periodic
 *    -- thread is not started
 *    -- thread holds preemption lock
 *  - JET_NOACTION - thread is already suspended
 *  - [IPPC] JET_INVALID_MODE_TARGET - thread is an IPPC handler
 */
jet_ret_t pok_thread_suspend_target(pok_thread_id_t id)
{
    jet_ret_t ret;

    pok_thread_t *t = get_thread_by_id(id);
    if(!t) return EINVAL;

    // can't suspend current process
    // _using this function_
    // (use pok_thread_suspend instead)
    if (t == current_thread) return EINVAL;

    if (pok_thread_is_periodic(t)) return JET_INVALID_MODE_TARGET;

    // Server threads cannot be suspended.
    if (t->ippc_server_connection) return JET_INVALID_MODE_TARGET;

    pok_preemption_local_disable();

    ret = JET_INVALID_MODE_TARGET;
    // can't suspend stopped (dormant) process
    if (t->state == POK_STATE_STOPPED) goto out;

    // Cannot suspend process holded preemption lock.
    if (current_partition_arinc->lock_level > 0
        && current_partition_arinc->thread_locked == t) goto out;

    ret = JET_NOACTION;
    if (t->suspended) goto out;

    thread_suspend(t);

    ret = EOK;
out:
    pok_preemption_local_enable();

    return ret;
}

/*
 * Suspend current thread with a possible timeout.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EFAULT - 'time' points to inaccessible memory
 *  - EINVAL
 *    -- thread is the current thread
 *  - JET_INVALID_MODE
 *    -- thread is periodic
 *    -- thread holds preemption lock
 *    -- thread is an error handler
 *  - JET_NOACTION - thread is already suspended
 *  - [IPPC] JET_INVALID_MODE - thread is an IPPC handler
 */
jet_ret_t pok_thread_suspend(const pok_time_t* __user time)
{
    pok_thread_t *t = current_thread;

    const pok_time_t* __kuser k_time = jet_user_to_kernel_typed_ro(time);
    if(!k_time) return EFAULT;

    pok_time_t kernel_time = *k_time;

    // although periodic process can never be suspended anyway,
    // ARINC-653 requires different error code
    if (pok_thread_is_periodic(t)) return JET_INVALID_MODE;

    // Server threads cannot be suspended.
    if (t->ippc_server_connection) return JET_INVALID_MODE;

    if (!thread_is_waiting_allowed()) return JET_INVALID_MODE;

    pok_preemption_local_disable();

    if(kernel_time == 0) goto out; // Nothing to do with 0 timeout.

    thread_suspend(t);

    if(!pok_time_is_infinity(kernel_time)) goto suspend_timed;

out:
    pok_preemption_local_enable();

    return EOK;

suspend_timed:
    thread_suspend_timed(t, kernel_time);

    pok_preemption_local_enable();

    return t->wait_result;
}

/*
 * Stop given thread.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL
 *    -- thread identificator is invalid
 *    -- thread is a current thread
 *
 *  - JET_NOACTION - target thread is already stopped.
 */
jet_ret_t pok_thread_stop_target(pok_thread_id_t id)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    jet_ret_t ret = EINVAL;

    pok_thread_t *t = get_thread_by_id(id);
    if(!t) return EINVAL;

    pok_thread_t* thread_current = part->thread_current;

    // can's stop self
    // use pok_thread_stop to do that
    if (t == thread_current) return EINVAL;

    struct jet_thread_shared_data* tshd_target = part->kshd->tshd
        + (t - part->threads);

    pok_preemption_local_disable();

    ret = JET_NOACTION;
    if (t->state == POK_STATE_STOPPED)
    {
        // Target process is already stopped.
        goto out;
    }

    if(t->state == POK_STATE_WAITING) {
        // Interrupt waiting.
        thread_wait_cancel(t);
    }

    if(t->relations_stop.donate_target != NULL)
    {
        /* target waits other thread to stop. */
        if(!t->relations_stop.first_donator)
        {
            t->relations_stop.first_donator = thread_current;
            ret = EOK;
        }

        // Add ourselves into the list of "donators" *after* the target.
        thread_current->relations_stop.donate_target = t->relations_stop.donate_target;
        thread_current->relations_stop.next_donator = t->relations_stop.next_donator;
        t->relations_stop.next_donator = thread_current;
    }
    else if(t->relations_stop.first_donator != NULL)
    {
        /* Someone else waits for the target. */
        // Add ourselves into the beginning of the list of "donators".
        thread_current->relations_stop.donate_target = t;
        thread_current->relations_stop.next_donator = t->relations_stop.first_donator;
        t->relations_stop.first_donator = thread_current;
    }
    else if(t->ippc_connection) {
        /*
         * Target thread has IPPC connection opened.
         *
         * Need to wait this connection to terminate and close it.
         */
        jet_ippc_connection_cancel(t->ippc_connection);

        thread_current->relations_stop.donate_target = t;
        thread_current->relations_stop.next_donator = NULL;
        t->relations_stop.first_donator = thread_current;

        ret = EOK;
    }
    else if(tshd_target->msection_count != 0)
    {
        /* target currently owners the section. Cannot kill it immediately. */
        thread_current->relations_stop.donate_target = t;
        thread_current->relations_stop.next_donator = NULL;
        t->relations_stop.first_donator = thread_current;

        ret = EOK;

        tshd_target->thread_kernel_flags = THREAD_KERNEL_FLAG_KILLED;
    }
    else
    {
        // Target thread can be stopped immediately.
        ret = EOK;
        thread_stop(t);

        goto out;
    }
    // Notify scheduler that threads cannot continue its execution now.
    pok_sched_local_invalidate();

out:
    pok_preemption_local_enable();

    return ret;
}

/*
 * Stop current thread.
 *
 * Return EOK on success (never returns actually).
 */
jet_ret_t pok_thread_stop(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* t = part->thread_current;

    pok_preemption_local_disable();

    // Thread cannot execute anything in donation state.
    assert(t->relations_stop.donate_target == NULL);
    // While already stopped, thread shouldn't stop itself.
    assert_os(t->relations_stop.first_donator == NULL);
    /*
     * It is *possible* for thread to be stopped while in msection.
     * But this cannot hurt kernel.
     *
     * TODO: Should additional os-check to be added?
     */
    thread_stop(t);

    if(t == part->thread_error)
    {
        error_check_after_handler();

        error_ignore_sync();
    }
    // Stopping current thread always change scheduling.
    pok_sched_local_invalidate();
    pok_preemption_local_enable();

    return EOK;
}

/*
 * Find a thread by the name.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EFAULT - 'name' or 'id' points to inaccessible memory.
 *  - JET_INVALID_CONFIG - there is no thread with given name.
 */
jet_ret_t pok_thread_find(const char* __user name, pok_thread_id_t* __user id)
{
    char kernel_name[MAX_NAME_LENGTH];
    pok_thread_t* t;

    const char* __kuser k_name = jet_user_to_kernel_ro(name, MAX_NAME_LENGTH);
    if(!k_name) return EFAULT;

    pok_thread_id_t* __kuser k_id = jet_user_to_kernel_typed(id);
    if(!k_id) return EFAULT;

    memcpy(kernel_name, k_name, MAX_NAME_LENGTH);

    t = find_thread(kernel_name);

    if(!t) return JET_INVALID_CONFIG;

    *k_id = t - current_partition_arinc->threads;

    return EOK;
}

/*
 * Make current thread waiting for the next period.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - JET_INVALID_MODE
 *    -- thread is not periodic
 *    -- thread doesn't allowed to wait
 */
jet_ret_t pok_sched_end_period(void)
{
    pok_thread_t* t = current_thread;

    if(!pok_thread_is_periodic(t)) return JET_INVALID_MODE;

    if(!thread_is_waiting_allowed()) return JET_INVALID_MODE;

    pok_preemption_local_disable();

    t->release_point += t->period;

    thread_wait_timed(t, t->release_point);
    thread_set_deadline(t, t->release_point + t->time_capacity);

    pok_preemption_local_enable();

    return EOK;
}

/*
 * Replenish budget time for current thread.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EFAULT - 'budget' points to inaccessible memory.
 *  - JET_NOACTION - Current is main or error handler.
 *  - JET_INVALID_MODE - thread is periodic, and its deadline would
 *     exceed next release point. This includes "'budget' is infinity" case.
 */
jet_ret_t pok_sched_replenish(const pok_time_t* __user budget)
{
    jet_ret_t ret;

    pok_partition_arinc_t* part = current_partition_arinc;

    const pok_time_t* __kuser k_budget = jet_user_to_kernel_typed_ro(budget);
    if(!k_budget) return EFAULT;
    pok_time_t kernel_budget = *k_budget;

    pok_thread_t* t = current_thread;

    if(t == part->thread_error) return JET_NOACTION;

    if(part->mode != POK_PARTITION_MODE_NORMAL)
        return JET_NOACTION;

    if(pok_time_is_infinity(t->time_capacity)) return EOK; //nothing to do

    pok_preemption_local_disable();

    if(pok_time_is_infinity(kernel_budget))
    {
        if(!pok_time_is_infinity(t->period))
        {
            ret = JET_INVALID_MODE;
            goto out;
        }

        delayed_event_remove(&part->partition_delayed_events,
            &t->thread_deadline_event);
        ret = EOK;
    }
    else
    {
        pok_time_t calculated_deadline = jet_system_time() + kernel_budget;

        if(!pok_time_is_infinity(t->period)
            && (calculated_deadline >= (t->release_point + t->period)))
        {
            // For periodic process new deadline will exceed next release point.
            ret = JET_INVALID_MODE;
            goto out;
        }

        thread_set_deadline(t, calculated_deadline);
        ret = EOK;
    }
out:
    pok_preemption_local_enable();

    return ret;
}


/* Assert that thread id is correct.*/
static void assert_thread_id(pok_thread_id_t id)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    assert_os(id < part->nthreads_used);
}

jet_ret_t jet_msection_enter_helper(struct msection* __user section)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* thread_current = part->thread_current;

    struct msection* __kuser msection_entering = jet_user_to_kernel_typed(section);

    assert_os(msection_entering);

    pok_preemption_local_disable();

    if(msection_entering->owner != JET_THREAD_ID_NONE
        && msection_entering->owner != thread_current - part->threads)
    {
        // Set thread as entering into the section ...
        thread_current->msection_entering = msection_entering;
        // And let scheduler to do all the work.
        pok_sched_local_invalidate();
    }

    pok_preemption_local_enable();

    return EOK;
}

/*
 * Wait on given msection with possible timeout.
 *
 * Return EOK if waiting has been interrupted by notify.
 * Return ETIMEDOUT if timeout has been expired.
 * Return JET_CANCELLED if thread has been STOP()ed or IPPC handler has
 * been cancelled.
 *
 * Possible error codes:
 *
 *  - EFAULT - 'timeout' points to inaccessible memory.
 *  - JET_INVALID_MODE - thread is not allowed to wait
 *
 */
jet_ret_t jet_msection_wait(struct msection* __user section,
    const pok_time_t* __user timeout)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* thread_current = part->thread_current;

    const pok_time_t* __kuser kernel_timeout = jet_user_to_kernel_typed_ro(timeout);

    if(kernel_timeout == NULL) return EFAULT;

    struct msection* __kuser msection_entering = jet_user_to_kernel_typed(section);

    // Passing wrong section is OS error, not a user.
    assert_os(msection_entering);
    // The thread should be a section's owner.
    assert_os(msection_entering->owner == (thread_current - part->threads));

    struct jet_thread_shared_data* tshd_current = part->kshd->tshd
        + (thread_current - part->threads);
    // The only section should be entered.
    assert_os(tshd_current->msection_count == 1);

    // Cannot use thread_is_waiting_allowed for wait on msection.
    if(part->lock_level // In the INIT_* mode lock level is positive, no need to check it explicitely.
        || part->thread_error == thread_current /* error thread cannot wait */
    ) {
        return JET_INVALID_MODE;
    }

    pok_preemption_local_disable();

    if(thread_current->relations_stop.first_donator != NULL)
    {
        thread_current->wait_result = JET_CANCELLED;
        goto out;
    }

    thread_current->msection_entering = msection_entering;
    // Release section...
    msection_entering->owner = JET_THREAD_ID_NONE;
    // ... And use common wait.
    thread_wait_common(thread_current, *kernel_timeout);

    /*
     * It is possible, that current thread wasn't the highest-priority
     * thread. Because of that, `thread_wait_common` may do not cause
     * scheduling invalidation.
     *
     * From the other side, waiting on msection and leaving msection
     * are the only possible state-modifications for non-highest-priority
     * thread. Normal msection leaving is followed by jet_resched(),
     * which invalidates scheduling.
     *
     * So, explicitely invalidate scheduling here.
     */
    pok_sched_local_invalidate();
    // After the releasing we will be in common 'msection_entering' state.
out:
    pok_preemption_local_enable();

    return thread_current->wait_result;
}

/*
 * Notify given thread.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - JET_NOACTION - thread has already been awoked.
 */
jet_ret_t jet_msection_notify(struct msection* __user section,
    pok_thread_id_t thread_id)
{
    jet_ret_t ret = JET_NOACTION;

    struct msection* __kuser section_kernel = jet_user_to_kernel_typed(section);

    assert_os(section_kernel);

    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* thread_current = part->thread_current;

    assert_thread_id(thread_id);

    pok_thread_t* t = &part->threads[thread_id];

    pok_preemption_local_disable();

    if(t->state != POK_STATE_WAITING) goto out;

    struct msection* __kuser msection_entering = t->msection_entering;

    /*
     * Currently we require that msection should corresponds to
     * both current and awoken thread.
     */
    assert_os(section_kernel == msection_entering);
    assert_os(section_kernel->owner == (thread_current - part->threads));

    thread_wake_up(t);

    if(t->wait_result == EOK)
        ret = EOK;

out:
    pok_preemption_local_enable();

    return ret;
}

/******************* msection wait queue ******************************/
/* Remove thread from the queue. */
static void msection_wq_del(struct msection_wq* wq,
    struct jet_thread_shared_data* tshd_t)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    pok_thread_id_t next_id = tshd_t->wq_next;
    pok_thread_id_t prev_id = tshd_t->wq_prev;

    pok_thread_id_t *pnext, *pprev;

    if(next_id == JET_THREAD_ID_NONE)
    {
        pnext = &wq->last;
    }
    else
    {
        assert_thread_id(next_id);
        pnext = &part->kshd->tshd[next_id].wq_prev;
    }

    if(prev_id == JET_THREAD_ID_NONE)
    {
        pprev = &wq->first;
    }
    else
    {
        assert_thread_id(prev_id);
        pprev = &part->kshd->tshd[prev_id].wq_next;
    }

    *pnext = prev_id;
    *pprev = next_id;

    tshd_t->wq_next = tshd_t->wq_prev = JET_THREAD_ID_NONE;
}

/*
 * Awoke waiting threads in the waitqueue.
 *
 * Every thread in the queue which hasn't waited at the function's call
 * is removed from the queue.
 *
 * If 'first_only' is TRUE, the first waiting thread only. This thread
 * will be pointed by wq->first after the call.
 * If 'first_only' is FALSE, notify all waiting threads. List of the
 * awoken threads may be iterated directly from user space.
 *
 * May be called only by the owner of the section.
 *
 * Returns:
 *
 *     EOK - at least on thread has been notified.
 *     JET_NOACTION - there is no waiting threads in the waitqueue.
 */
jet_ret_t jet_msection_wq_notify(struct msection* __user section,
   struct msection_wq* __user wq,
   pok_bool_t is_all)
{
    jet_ret_t ret = JET_NOACTION;

    struct msection* __kuser section_kernel = jet_user_to_kernel_typed(section);
    assert_os(section_kernel);

    struct msection_wq* __kuser wq_kernel = jet_user_to_kernel_typed(wq);
    assert_os(wq_kernel);

    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* thread_current = part->thread_current;

    assert_os(section_kernel->owner == (thread_current - part->threads));

    pok_preemption_local_disable();

    pok_thread_id_t thread_id = wq_kernel->first;

    /* TODO: Assert that linkage is correct, so there is no loops in it. */
    while(thread_id != JET_THREAD_ID_NONE)
    {
        assert_thread_id(thread_id);

        pok_thread_t* t = &part->threads[thread_id];
        assert(t->msection_entering == section_kernel);

        struct jet_thread_shared_data* tshd_t = &part->kshd->tshd[thread_id];

        thread_id = tshd_t->wq_next;

        if(t->state == POK_STATE_WAITING) {
            thread_wake_up(t);
            if(t->wait_result == EOK) {
                ret = EOK;
                if(!is_all) break;
                continue;
            }
        }
        // If thread wasn't in waiting state, delete it from the queue.
        msection_wq_del(wq_kernel, tshd_t);
    }

    pok_preemption_local_enable();

    return ret;
}

/*
 * Compute number of waiting threads in the waitqueue.
 *
 * Every thread in the queue which hasn't waited at the function's call
 * is removed from the queue.
 *
 * Returns: EOK.
 */
jet_ret_t jet_msection_wq_size(struct msection* __user section,
   struct msection_wq* __user wq,
   size_t* __user size)
{
    size_t count = 0;

    struct msection* __kuser section_kernel = jet_user_to_kernel_typed(section);
    assert_os(section_kernel);

    struct msection_wq* __kuser wq_kernel = jet_user_to_kernel_typed(wq);
    assert_os(wq_kernel);

    size_t* size_kernel = jet_user_to_kernel_typed(size);
    assert_os(size_kernel);

    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* thread_current = part->thread_current;

    assert_os(section_kernel->owner == (thread_current - part->threads));

    pok_preemption_local_disable();

    pok_thread_id_t thread_id = wq_kernel->first;

    /* TODO: Assert that linkage is correct, so there is no loops in it. */
    while(thread_id != JET_THREAD_ID_NONE)
    {
        assert_thread_id(thread_id);

        pok_thread_t* t = &part->threads[thread_id];
        assert(t->msection_entering == section_kernel);

        struct jet_thread_shared_data* tshd_t = &part->kshd->tshd[thread_id];

        thread_id = tshd_t->wq_next;

        if(t->state != POK_STATE_WAITING)
        {
            msection_wq_del(wq_kernel, tshd_t);
        }
        else
        {
            count++;
        }
    }

    pok_preemption_local_enable();

    *size_kernel = count;

    return EOK;
}


/********************* wait queue for port*****************************/
void pok_thread_wq_init(pok_thread_wq_t* wq)
{
    INIT_LIST_HEAD(&wq->waits);
}

void pok_thread_wq_add(pok_thread_wq_t* wq, pok_thread_t* t)
{
    list_add_tail(&t->wait_elem, &wq->waits);
}

void pok_thread_wq_add_prio(pok_thread_wq_t* wq, pok_thread_t* t)
{
    pok_thread_t* other_thread;
    t->wait_priority = t->priority;
    list_for_each_entry(other_thread, &wq->waits, wait_elem)
    {
        if(other_thread->wait_priority < t->wait_priority)
        {
            list_add_tail(&t->wait_elem, &other_thread->wait_elem);
            return;
        }
    }

    list_add_tail(&t->wait_elem, &wq->waits);
}

void pok_thread_wq_remove(pok_thread_t* t)
{
    list_del_init(&t->wait_elem);
}

pok_thread_t* pok_thread_wq_wake_up(pok_thread_wq_t* wq)
{
    if(!list_empty(&wq->waits))
    {
        pok_thread_t* t = list_first_entry(&wq->waits, pok_thread_t, wait_elem);
        /*
         * First, remove thread from the waiters list.
         *
         * So futher thread_wake_up() will not interpret it as timeouted.
         */
        list_del_init(&t->wait_elem);
        thread_wake_up(t);

        assert(t->wait_result == EOK);

        return t;
    }

    return NULL;
}


int pok_thread_wq_get_nwaits(pok_thread_wq_t* wq)
{
    int count = 0;
    struct list_head* elem;
    list_for_each(elem, &wq->waits)
    {
        count++;
    }

    return count;
}


pok_bool_t pok_thread_wq_is_empty(pok_thread_wq_t* wq)
{
    return list_empty(&wq->waits);
}
