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

#include <core/sched_arinc.h>
#include <core/delayed_event.h>
#include <core/thread.h>
#include "thread_internal.h"

#include <cswitch.h>
#include <core/space.h>
#include <asp/arch.h>
#include <core/syscall.h>
#include <core/uaccess.h>

static void thread_start_func(void)
{
    pok_thread_t* thread_current = current_thread;

    pok_partition_jump_user(
        thread_current->entry,
        thread_current->init_stack_addr,
        thread_current->initial_sp);
}

void sched_arinc_start(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    part->idle_sp = 0;

    pok_thread_t* thread_main
        = &part->threads[POK_PARTITION_ARINC_MAIN_THREAD_ID];
#ifdef POK_NEEDS_GDB
    thread_main->entry_sp_user = NULL;
#endif
    part->lock_level = 1;
	part->thread_locked = thread_main;

	thread_main->state = POK_STATE_RUNNABLE;
    part->thread_current = thread_main;
    // Update kernel shared data.
    part->kshd->current_thread_id = POK_PARTITION_ARINC_MAIN_THREAD_ID;

	// Direct jump into main thread.
    part->base_part.fp_store_current = thread_main->fp_store;

	jet_context_restart_and_save(thread_main->initial_sp,
        &thread_start_func, &thread_main->sp);
}


/* Notification is received for given queuing port. */
static void port_queuing_fired(pok_port_queuing_t* port_queuing)
{
    if(port_queuing->direction == POK_PORT_DIRECTION_IN)
    {
        while(!pok_thread_wq_is_empty(&port_queuing->waiters))
        {
            pok_thread_t* t;
            pok_message_size_t message_size; // Just for function's call.

            if(!pok_channel_queuing_r_get_message(port_queuing->channel,
                &message_size, TRUE))
                break; // wait again

            t = pok_thread_wq_wake_up(&port_queuing->waiters);

            port_queuing_receive(port_queuing, t);
        }
    }
    else // if(port_queuing->direction == POK_PORT_DIRECTION_OUT)
    {
        while(!pok_thread_wq_is_empty(&port_queuing->waiters))
        {
            pok_thread_t* t;

            if(!pok_channel_queuing_s_get_message(port_queuing->channel, TRUE))
                break; // wait again

            t = pok_thread_wq_wake_up(&port_queuing->waiters);

            port_queuing_send(port_queuing, t);
        }
    }
}

/*
 * Function which is executed in kernel-only partition's context when
 * no threads can be executed at this moment.
 */
static void do_nothing_func(void)
{
    pok_preemption_local_enable();

    ja_inf_loop();
}

/*
 * Select given thread.
 * 
 * Return thread which should be run.
 */
static pok_thread_t* select_thread(pok_thread_t* selected_thread)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    part->thread_selected = selected_thread;
    if(part->waiting_section)
    {
        // Clear waiting section for possibly set it after.
        part->waiting_section->msection_kernel_flags = 0;
        part->waiting_section = NULL;
    }

    if(selected_thread == NULL) return NULL;

    pok_thread_t* new_thread = selected_thread;

    if(new_thread->relations_stop.donate_target)
    {
        /* We should wait for other thread to stop. */
        new_thread = new_thread->relations_stop.donate_target;
    }

    // Check that given thread is able to continue.
    struct msection* waiting_section = new_thread->msection_entering;
    if(waiting_section != NULL)
    {
        /* Limit number of attempts to determine current thread. TODO: this should be configurable constant. */
        int n_attempts = 5;

        do {
            pok_thread_id_t owner = waiting_section->owner;
            if(owner == JET_THREAD_ID_NONE || owner == (new_thread - part->threads))
            {
                waiting_section->owner = (new_thread - part->threads);
                new_thread->msection_entering = NULL;
                break;
            }

            assert_os(owner < part->nthreads_used);
            // Contention on msection is possible only in NORMAL mode, when main thread is not executed.
            assert_os(owner > POK_PARTITION_ARINC_MAIN_THREAD_ID);

            new_thread = part->threads + owner;
            assert_os(new_thread->state == POK_STATE_RUNNABLE);
            // TODO: other consistency checks
            part->waiting_section = waiting_section;

            waiting_section = new_thread->msection_entering;

            assert(n_attempts);
            n_attempts--;

        } while(waiting_section != NULL);
    }

    if(part->waiting_section)
    {
        part->waiting_section->msection_kernel_flags = MSECTION_KERNEL_FLAG_RESCHED_AFTER_LEAVE;
    }

    return new_thread;
}

// Called with local preemption disabled.
static void sched_arinc(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;

again:
    if(flag_test_and_reset(part->base_part.is_event))
    {
        struct jet_partition_event event;

        while(pok_partition_get_event(&event))
        {
            switch(event.event_type) {
                case JET_PARTITION_EVENT_TYPE_TIMER:
                    delayed_event_queue_check(&part->partition_delayed_events, jet_system_time());
                    break;
                case JET_PARTITION_EVENT_TYPE_PORT_SEND_AVAILABLE:
                case JET_PARTITION_EVENT_TYPE_PORT_RECEIVE_AVAILABLE:
                    port_queuing_fired(&part->ports_queuing[event.handler_id]);
                    break;
                default:
                    unreachable();
            }
        }
    }

    // Update timer.
    pok_time_t timer_new = delayed_event_queue_get_check_time(
        &part->partition_delayed_events);

    pok_partition_set_timer(&part->base_part, timer_new);

    if(timer_new != 0) {
        /* 
         * Check wether timer is not expired before we set it.
         * 
         * Such a way we eliminate consiquenses of possible race between
         * setting the timer and checking it in the global scheduler.
         */
        pok_time_t time_now = jet_system_time();
        if(time_now >= timer_new) {
            // Recheck delayed events.
            delayed_event_queue_check(&part->partition_delayed_events, time_now);
            // Need to redrain events before setting timer again.
            goto again;
        }
    }

    if(!flag_test_and_reset(part->sched_local_recheck_needed)) return;

    part->base_part.is_error_handler = FALSE; // Will be set if needed.

    pok_thread_t* old_thread = part->thread_current;
    pok_thread_t* new_thread;

    assert(part->mode != POK_PARTITION_MODE_IDLE); // Idle mode is implemented via function with preemption disabled.

    if(part->mode != POK_PARTITION_MODE_NORMAL)
    {
        new_thread = &part->threads[POK_PARTITION_ARINC_MAIN_THREAD_ID];
        if(new_thread->state == POK_STATE_STOPPED)
        {
            /*
             * Stopped main thread means that partition can do nothing.
             * 
             * But ARINC doesn't specify this state as IDLE.
             * 
             * So we treat this case as general "none of the threads are ready now".
             */
            new_thread = NULL;
        }
    }
#ifdef POK_NEEDS_ERROR_HANDLING
    else if(part->thread_error && part->thread_error->state != POK_STATE_STOPPED)
    {
        // Continue error handler
        new_thread = part->thread_error;
        part->base_part.is_error_handler = TRUE;
    }
    else if(!list_empty(&part->error_list))
    {
        // Start error handler (aperiodic, no timeout).
        new_thread = part->thread_error;

        new_thread->priority = new_thread->base_priority;
        new_thread->sp = NULL;

        new_thread->state = POK_STATE_RUNNABLE;

        part->base_part.is_error_handler = TRUE;
    }
#endif
    else if(part->lock_level)
    {
        new_thread = part->thread_locked;
    }
    else if(!list_empty(&part->eligible_threads))
    {
        new_thread = list_first_entry(&part->eligible_threads,
            pok_thread_t, eligible_elem);
    }
    else
    {
        new_thread = NULL;
    }

    new_thread = select_thread(new_thread);

    if(new_thread == old_thread)
    {
        if(new_thread == NULL) return; // "do_nothing" continues

        if(new_thread->sp != 0) return; // Thread continues its execution.

        /* 
         * None common thread can restart itself: someone *other* should
         * call START function for it.
         * 
         * The only exception is error handler: If it calls STOP and
         * error list is not empty, error handler is automatically restarted.
         */
        assert(new_thread == part->thread_error);

#ifdef POK_NEEDS_GDB
        new_thread->entry_sp_user = NULL;
#endif
        jet_context_restart_and_save(new_thread->initial_sp,
            &thread_start_func, &new_thread->sp);
    }

    // Switch between different threads
    part->thread_current = new_thread;
    // Update kernel shared data
    if(new_thread)
        part->kshd->current_thread_id = new_thread - part->threads;

    struct jet_context** old_sp = old_thread? &old_thread->sp : &part->idle_sp;
    struct jet_context* new_sp;

    if(new_thread)
    {
        new_sp = new_thread->sp;
        if(new_sp == NULL)
        {
            new_sp = jet_context_init(new_thread->initial_sp,
                &thread_start_func);
            new_thread->sp = new_sp;
        }
    }
    else
    {
        // New thread is "do_nothing"
        new_sp = part->idle_sp;
        if(new_sp == NULL)
        {
            new_sp = jet_context_init(part->base_part.initial_sp,
                &do_nothing_func);
            part->idle_sp = new_sp;
        }
    }

    /*
     * If old_thread->sp is 0, this should be processed upon
     * returning to given thread, not now.
     */
    if(*old_sp == NULL) old_sp = NULL;

#ifdef POK_NEEDS_GDB
    if(old_thread && part->base_part.entry_sp_user)
    {
        /*
         * Copy address of frame with user space addresses locally
         * and clear original.
         */
        old_thread->entry_sp_user = part->base_part.entry_sp_user;
        part->base_part.entry_sp_user = NULL;
    }
#endif /* POK_NEEDS_GDB */
    part->base_part.fp_store_current = new_thread->fp_store;

    if(old_sp)
    {
        jet_context_switch(old_sp, new_sp);
    }
    else
    {
        jet_context_jump(new_sp);
    }
}

void pok_preemption_local_disable(void)
{
    assert(!current_partition_arinc->base_part.preempt_local_disabled);

    flag_set(current_partition_arinc->base_part.preempt_local_disabled);
}

void pok_preemption_local_enable(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    assert(part->base_part.preempt_local_disabled);

    sched_arinc();

    barrier();

    flag_reset(part->base_part.preempt_local_disabled);

    // Check that we do not miss events since sched_arinc() starts.

    while(ACCESS_ONCE(part->base_part.is_event))
    {
        flag_set(part->base_part.preempt_local_disabled);

        sched_arinc();

        barrier();

        flag_reset(current_partition_arinc->base_part.preempt_local_disabled);
    }
}

void pok_sched_arinc_on_event(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* thread_current = part->thread_current;

    if(thread_current)
    {
        //Update information according to current kernel shared data.
        struct jet_thread_shared_data* tshd_current = part->kshd->tshd
            + (thread_current - part->threads);

        if(tshd_current->msection_entering != NULL)
        {
            struct msection * __kuser msection_entering =
                jet_user_to_kernel_typed(tshd_current->msection_entering);

            assert_os(msection_entering);

            thread_current->msection_entering = msection_entering;
        }
    }
    sched_arinc();
}

void pok_sched_local_invalidate(void)
{
    flag_set(current_partition_arinc->sched_local_recheck_needed);
}

pok_ret_t jet_resched(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    pok_thread_t* thread_current = part->thread_current;

    struct jet_thread_shared_data* tshd_current = part->kshd->tshd
        + (thread_current - part->threads);

    pok_preemption_local_disable();

    if(part->waiting_section != NULL && part->waiting_section->owner == JET_THREAD_ID_NONE)
        pok_sched_local_invalidate(); // msecation we await for has been released.

    if(thread_current->relations_stop.first_donator != NULL
        && tshd_current->msection_count == 0)
    {
        // It is safe to terminate thread now and resume all its donators.
        pok_thread_t* donator = thread_current->relations_stop.first_donator;

        thread_current->relations_stop.first_donator = NULL;
        thread_stop(thread_current);

        // Resume donators.
        do {
            pok_thread_t* next_donator;

            donator->relations_stop.donate_target = NULL;
            if(donator == part->thread_selected)
                pok_sched_local_invalidate();

            next_donator = donator->relations_stop.next_donator;

            if(donator->relations_stop.first_donator)
            {
                donator->relations_stop.first_donator = NULL;
                thread_stop(donator); // Donator itself needs to be terminated.
            }
            donator->relations_stop.next_donator = NULL;
            donator = next_donator;
        } while(donator != NULL);
    }

    pok_preemption_local_enable();

    return POK_ERRNO_OK;
}
