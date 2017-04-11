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

#include "thread_internal.h"
#include <core/sched_arinc.h>
#include <core/space.h>

#include <common.h>

// Allocate stack for thread.
static pok_bool_t thread_allocate_stack(pok_thread_t* t)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    t->init_stack_addr = ALIGN_VAL(part->stacks_current + t->user_stack_size,
        jet_ustack_get_alignment());

    if(t->init_stack_addr > part->stacks_end) return FALSE;
    part->stacks_current = t->init_stack_addr;

    return TRUE;
}

/* Fill thread structure assuming stack is already allocated. */
static void thread_create_no_stack(pok_thread_t* t)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    // Initialize thread shared data
    struct jet_thread_shared_data* tshd_t = part->kshd->tshd
        + (t - part->threads);
    tshd_t->msection_count = 0;
    tshd_t->msection_entering = NULL;

    /*
     * Do not modify stack here: it will be filled when thread will run.
     */
    t->sp = 0;
#ifdef POK_NEEDS_GDB
    t->entry_sp_user = NULL;
#endif

    t->priority = t->base_priority;
    t->state = POK_STATE_STOPPED;

    t->suspended = FALSE;

    t->relations_stop.donate_target = NULL;
    t->relations_stop.first_donator = NULL;
    t->relations_stop.next_donator = NULL;

    t->msection_entering = NULL;

    delayed_event_init(&t->thread_deadline_event);
    delayed_event_init(&t->thread_delayed_event);
    INIT_LIST_HEAD(&t->wait_elem);
    INIT_LIST_HEAD(&t->eligible_elem);
    INIT_LIST_HEAD(&t->error_elem);
}

pok_bool_t thread_create(pok_thread_t* t)
{
    if(!thread_allocate_stack(t)) return FALSE;

    thread_create_no_stack(t);

    return TRUE;
}

pok_bool_t thread_create_several(pok_thread_t* t, int n_threads)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    uintptr_t stacks_old = part->stacks_current;

    // First allocate stacks for all threads.
    for(int i = 0; i < n_threads; i++) {
        if(!thread_allocate_stack(t + i)) {
            part->stacks_current = stacks_old; // Rollback stacks allocations.
            return FALSE;
        }
    }

    // Initialize all thread structures.
    for(int i = 0; i < n_threads; i++) {
        thread_create_no_stack(t + i);
    }

    return TRUE;
}

/*
 * Return true if thread is eligible.
 *
 * Note: Main thread and error thread are NEVER eligible.
 */
static pok_bool_t thread_is_eligible(pok_thread_t* t)
{
    assert(!t->ippc_server_connection);

    return !list_empty(&t->eligible_elem);
}

/*
 * Set thread eligible for running.
 *
 * Thread shouldn't be eligible already.
 *
 * Should be executed with local preemption disabled.
 */
static void thread_set_eligible(pok_thread_t* t)
{
    pok_thread_t* other_thread;
    pok_partition_arinc_t* part = current_partition_arinc;

    assert(!t->ippc_server_connection);

    assert(part->mode == POK_PARTITION_MODE_NORMAL);
    assert(!thread_is_eligible(t));

    list_for_each_entry(other_thread,
        &part->eligible_threads,
        eligible_elem)
    {
        if(other_thread->priority < t->priority)
        {
            list_add_tail(&t->eligible_elem, &other_thread->eligible_elem);
            goto out;
        }

    }
    list_add_tail(&t->eligible_elem, &part->eligible_threads);

out:
    if(t->eligible_elem.prev == &part->eligible_threads)
    {
        // Thread is inserted into the first position.
        pok_sched_local_invalidate();
    }
}

void thread_yield(pok_thread_t *t)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    assert(!t->ippc_server_connection);

    if(thread_is_eligible(t))
    {
        // TODO: Improve performance
        if(t->eligible_elem.prev == &part->eligible_threads)
        {
            // Thread is removed from the first position.
            pok_sched_local_invalidate();
        }
        list_del_init(&t->eligible_elem);
        thread_set_eligible(t);
    }
}

/*
 * Set thread not eligible for running, if it was.
 *
 * Should be executed with local preemption disabled.
 */
static void thread_set_uneligible(pok_thread_t* t)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    assert(!t->ippc_server_connection);

    if(!list_empty(&t->eligible_elem))
    {
        if(t->eligible_elem.prev == &part->eligible_threads)
        {
            // Thread is removed from the first position.
            pok_sched_local_invalidate();
        }
        list_del_init(&t->eligible_elem);
    }
}

/* Calls thread_delayed_event for thread with given id.
 *
 * Callback for delayed_event_add.
 */
static void thread_process_delayed_event(uint16_t handler_id)
{
    pok_thread_t* t = &current_partition_arinc->threads[handler_id];

    assert(!t->ippc_server_connection);

    t->thread_delayed_func(t);
}

void thread_delay_event(pok_thread_t* t, pok_time_t delay_time,
    void (*thread_delayed_func)(pok_thread_t* t))
{
    pok_partition_arinc_t* part = current_partition_arinc;

    assert(!t->ippc_server_connection);

    t->thread_delayed_func = thread_delayed_func;

    delayed_event_add(&part->partition_delayed_events,
        &t->thread_delayed_event, delay_time,
        t - part->threads,
        &thread_process_delayed_event);
}

/*
 * Emit deadline event for the thread.
 *
 * Callback for delayed_event_add.
 */
static void thread_deadline_occured(uint16_t handler_id)
{
    pok_thread_t* thread = &current_partition_arinc->threads[handler_id];
    printf_debug("%s: Deadline missed for thread '%s' (%ld)\n",
                 current_partition->name, thread->name, (long)(thread - current_partition_arinc->threads));
    pok_thread_emit_deadline_missed(thread);

    // TODO: if error was ignored, what to do?
}

void thread_set_deadline(pok_thread_t* t, pok_time_t deadline_time)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    delayed_event_add(&part->partition_delayed_events,
        &t->thread_deadline_event, deadline_time,
        t - part->threads,
        &thread_deadline_occured);
}

void thread_delay_event_cancel(pok_thread_t* t)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    assert(!t->ippc_server_connection);

    delayed_event_remove(&part->partition_delayed_events,
    &t->thread_delayed_event);
}

/* Stop single thread. */
static void thread_stop_single(pok_thread_t* t)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    assert(t->ippc_connection == NULL);
    assert(t->state != POK_STATE_STOPPED);
    t->state = POK_STATE_STOPPED;

    if(t->ippc_server_connection) {
        jet_ippc_connection_terminate(t->ippc_server_connection,
            JET_IPPC_CONNECTION_TERMINATE_STATUS_FAILED);
        if(t->ippc_server_connection == part->base_part.ippc_handled_actual) {
            /* If IPPC request will be started again, local scheduler shouldn't miss that point. */
            pok_sched_local_invalidate();
        }
    }
    else {
        thread_set_uneligible(t);
        // Remove thread from all queues except one for error handler.
        thread_delay_event_cancel(t);
        delayed_event_remove(&part->partition_delayed_events,
            &t->thread_deadline_event);
    }

    pok_thread_wq_remove(t);

    if(part->lock_level && part->thread_locked == t)
    {
        current_partition_arinc->lock_level = 0;
        pok_sched_local_invalidate(); // Thread could be non-highest priority thread when stopped.
    }

    if(t->is_unrecoverable)
    {
        t->is_unrecoverable = FALSE;
        part->nthreads_unrecoverable--;
    }

    t->msection_entering = NULL;
}

void thread_stop(pok_thread_t* t)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    thread_stop_single(t);

    if(t->relations_stop.first_donator != NULL)
    {
        // Resume donators.
        pok_thread_t* donator = t->relations_stop.first_donator;

        t->relations_stop.first_donator = NULL;

        do {
            pok_thread_t* next_donator;

            donator->relations_stop.donate_target = NULL;
            if(donator == part->thread_selected)
                pok_sched_local_invalidate();

            next_donator = donator->relations_stop.next_donator;

            if(donator->relations_stop.first_donator)
            {
                donator->relations_stop.first_donator = NULL;
                thread_stop_single(donator); // Donator itself needs to be terminated.
            }
            donator->relations_stop.next_donator = NULL;
            donator = next_donator;
        } while(donator != NULL);
    }
}

pok_bool_t thread_is_waiting_allowed(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* thread_current = part->thread_current;

    if(thread_current->ippc_server_connection) {
        // TODO: What about checking other flags?
        return !thread_current->ippc_server_connection->cannot_wait;
    }

    if(part->lock_level // In the INIT_* mode lock level is positive, no need to check it explicitely.
#ifdef POK_NEEDS_ERROR_HANDLING
        || part->thread_error == thread_current /* error thread cannot wait */
#endif
    ) {
        return FALSE;
    }

    struct jet_thread_shared_data* kshd_current = part->kshd->tshd
        + (thread_current - part->threads);

    // It is prohibited to call waiting function inside msection.
    assert_os(kshd_current->msection_count == 0);

    /*
     * It is prohibited to call waiting function when someone waits us for terminate.
     *
     * Yes, we doesn't trust 'msection_count', which can be set from user space.
     */
    assert_os(thread_current->relations_stop.first_donator == NULL);

    return TRUE;
}


void thread_start_prepare(pok_thread_t* t)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    struct jet_thread_shared_data* tshd = part->kshd->tshd
        + (t - part->threads);

    t->priority = t->base_priority;
    t->sp = 0;

    tshd->msection_count = 0;
    tshd->msection_entering = NULL;
    tshd->priority = t->priority;
    tshd->thread_kernel_flags = 0;
}

/* Start thread in normal mode. */
void thread_start_normal(pok_thread_t* t, pok_time_t delay)
{
    if (pok_time_is_infinity(t->period)) {
        // aperiodic process
        t->release_point = jet_system_time() + delay;
    }
    else {
        // periodic process
        t->release_point = get_next_periodic_processing_start() + delay;
    }

    if(!pok_time_is_infinity(t->time_capacity))
        thread_set_deadline(t, t->release_point + t->time_capacity);

    /* Only non-delayed aperiodic process starts immediately */
    if(delay == 0 && pok_time_is_infinity(t->period))
        thread_start(t);
    else
        thread_wait_timed(t, t->release_point);
}


void thread_start(pok_thread_t* t)
{
    t->state = POK_STATE_RUNNABLE;

    if(!t->ippc_server_connection && !t->suspended)
    {
        thread_set_eligible(t);
    }
}

static void server_thread_wait_generic(pok_thread_t* t, pok_time_t timeout)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    assert(t->ippc_server_connection);

    if(jet_ippc_connection_pause(t->ippc_server_connection, timeout)) {
        t->state = POK_STATE_WAITING;
        if(t->ippc_server_connection == part->base_part.ippc_handled_actual) {
            /* If IPPC request will be continued, local scheduler shouldn't miss that point. */
            pok_sched_local_invalidate();
        }
    } else {
        // The only reason of failing to stop is cancelled state.
        // In case of .cannot_wait flag waiting shouldn't be attempted.
        assert(t->ippc_server_connection->is_cancelled);
        t->wait_result = POK_ERRNO_CANCELLED;
    }
}

void thread_wait(pok_thread_t* t)
{
    assert(current_partition_arinc->mode == POK_PARTITION_MODE_NORMAL);

    if(t->ippc_server_connection) {
        server_thread_wait_generic(t, POK_TIME_INFINITY);
    }
    else {
        t->state = POK_STATE_WAITING;
        thread_set_uneligible(t);
    }
}

void thread_wait_timeout(pok_thread_t* t)
{
    pok_ret_t wait_result = POK_ERRNO_TIMEOUT;

    assert(t->state == POK_STATE_WAITING);

    t->state = POK_STATE_RUNNABLE;

    if(!list_empty(&t->wait_elem)) {
        // Cancel wait on object.
        list_del_init(&t->wait_elem);
    }

    if(t->ippc_connection) {
        if(!jet_ippc_connection_continue(t->ippc_connection)) {
            wait_result = POK_ERRNO_OK; // Actually, in case of IPPC request wait result has no sence.
        }
    }

    if(!t->ippc_server_connection && !t->suspended) {
        thread_set_eligible(t);
    }

    t->wait_result = wait_result;
}

void thread_wait_cancel(pok_thread_t* t)
{
    pok_ret_t wait_result = POK_ERRNO_CANCELLED;

    assert(t->state == POK_STATE_WAITING);

    t->state = POK_STATE_RUNNABLE;

    thread_delay_event_cancel(t);

    if(!list_empty(&t->wait_elem)) {
        // Cancel wait on object.
        list_del_init(&t->wait_elem);
    }

    if(t->ippc_connection) {
        jet_ippc_connection_cancel(t->ippc_connection);
    }

    if(t->ippc_server_connection) {
        if(!jet_ippc_connection_unpause(t->ippc_server_connection)) {
            if(!t->ippc_server_connection->is_cancelled) {
                wait_result = POK_ERRNO_TIMEOUT;
            }
        }
    }
    else if(!t->suspended) {
        thread_set_eligible(t);
    }

    t->wait_result = wait_result;
}


void thread_wake_up(pok_thread_t* t)
{
    assert(t->state == POK_STATE_WAITING);

    t->state = POK_STATE_RUNNABLE;

    pok_ret_t wait_result = POK_ERRNO_OK;

    if(t->ippc_server_connection) {
        struct jet_ippc_connection* connection = t->ippc_server_connection;

        if(!jet_ippc_connection_unpause(connection)) {
            wait_result = connection->is_cancelled
                ? POK_ERRNO_CANCELLED
                : POK_ERRNO_TIMEOUT;
        }
    }
    else {
        // Cancel possible timeout.
        thread_delay_event_cancel(t);
        wait_result = POK_ERRNO_OK;

        if(!t->suspended) {
            thread_set_eligible(t);
        }
    }

    t->wait_result = wait_result;
}


void thread_wait_timed(pok_thread_t *t, pok_time_t time)
{
    assert(t);
    assert(!pok_time_is_infinity(time));

    if(t->ippc_server_connection) {
        // Server thread.
        server_thread_wait_generic(t, time);
    }
    else {
        // Normal thread.
        thread_wait(t);
        thread_delay_event(t, time, &thread_wait_timeout);
    }
}


void thread_suspend(pok_thread_t* t)
{
    t->suspended = TRUE;
    thread_set_uneligible(t);
}

// Special function for resume thread after suspension time is over.
static void thread_resume_waited(pok_thread_t* t)
{
    assert(t->state == POK_STATE_WAITING);

    t->suspended = FALSE;
    t->state = POK_STATE_RUNNABLE;

    // Set flag that we has been interrupted by timeout.
    t->wait_result = POK_ERRNO_TIMEOUT;

    thread_set_eligible(t);
}

void thread_suspend_timed(pok_thread_t* t, pok_time_t time)
{
    t->suspended = TRUE;
    thread_set_uneligible(t);
    thread_wait(t);
    thread_delay_event(t, jet_system_time() + time, &thread_resume_waited);
}


void thread_resume(pok_thread_t* t)
{
    t->suspended = FALSE;
    if(delayed_event_is_active(&t->thread_delayed_event)
        && t->thread_delayed_func == &thread_resume_waited) {
        // We are waited on timer for suspencion. Cancel that waiting.
        t->state = POK_STATE_RUNNABLE;

        thread_delay_event_cancel(t);

            // Set flag that we doesn't hit timeout.
            t->wait_result = POK_ERRNO_OK;
    }

    if(t->state == POK_STATE_RUNNABLE)
    {
        thread_set_eligible(t);
    }
}
