#include <core/sched_arinc.h>
#include <core/delayed_event.h>
#include <core/thread.h>
#include "thread_internal.h"

static void thread_start_func(void)
{
    pok_thread_t* thread_current = current_thread;
    
    pok_partition_jump_user(
        thread_current->init_stack_addr,
        thread_current->entry,
        &thread_current->initial_sp);
}

void sched_arinc_start(void)
{
	pok_partition_arinc_t* part = current_partition_arinc;
    
    pok_thread_t* thread_main
        = &part->threads[POK_PARTITION_ARINC_MAIN_THREAD_ID];
    
    part->lock_level = 1;
	part->thread_locked = thread_main;
	
	thread_main->state = POK_STATE_RUNNABLE;
	
	// Direct jump into main thread.
	pok_context_restart(&thread_main->initial_sp, &thread_start_func,
        &thread_main->sp);
}

static void thread_deadline_occured(struct delayed_event* event)
{
    pok_thread_t* thread = container_of(event, typeof(*thread), thread_deadline_event);
    pok_thread_emit_deadline_missed(thread);
    
    // TODO: if error was ignored, what to do?
}

static void thread_delayed_event_func(struct delayed_event* event)
{
    pok_thread_t* thread = container_of(event, typeof(*thread), thread_delayed_event);
    thread_wake_up(thread);
}


/* Notification is received for given queuing port. */
static void port_queuing_fired(pok_port_queuing_t* port_queuing)
{
    if(port_queuing->direction == POK_PORT_DIRECTION_IN)
    {
        while(!pok_thread_wq_is_empty(&port_queuing->waiters))
        {
            pok_thread_t* t;
            int n;
            
            if(!pok_channel_queuing_receive(port_queuing->channel, TRUE))
                break; // wait again
            
            n = pok_channel_queuing_r_n_messages(port_queuing->channel);
            
            for(; n > 0; n--)
            {
                t = pok_thread_wq_wake_up(&port_queuing->waiters);
                if(!t) break;
                
                port_queuing_receive(port_queuing, t);
            }
        }
    }
    else // if(port_queuing->direction == POK_PORT_DIRECTION_OUT)
    {
        while(!pok_thread_wq_is_empty(&port_queuing->waiters))
        {
            pok_message_t* m;
            pok_thread_t* t;
            
            m = pok_channel_queuing_s_get_message(port_queuing->channel, TRUE);
            if(!m) break; // wait again
            
            t = pok_thread_wq_wake_up(&port_queuing->waiters);
            assert(t);
            
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
    
    wait_infinitely();
}


// Called with local preemption disabled.
static void sched_arinc(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    
    if(flag_test_and_reset(part->base_part.state.bytes.control_returned))
    {
        pok_port_queuing_t* port_queuing = part->ports_queuing;
        pok_port_queuing_t* port_queuing_end = port_queuing + part->nports_queuing;
        
        for(;port_queuing < port_queuing_end; port_queuing++)
        {
            if(!port_queuing->is_notified) continue;
            
            port_queuing->is_notified = FALSE; // Acquire semantic
            barrier();
            
            port_queuing_fired(port_queuing);
        }
        
        // As if time has been changed too.
        flag_set(part->base_part.state.bytes.time_changed);
    }
    if(flag_test_and_reset(part->base_part.state.bytes.time_changed))
    {
        pok_time_t now = POK_GETTICK();
        
        delayed_event_queue_check(&part->queue_deadline, now, &thread_deadline_occured);
        
        delayed_event_queue_check(&part->queue_delayed, now, &thread_delayed_event_func);
    }
    
    if(!flag_test_and_reset(part->sched_local_recheck_needed)) return;

    part->base_part.is_error_handler = FALSE; // Will be set if needed.

    pok_thread_t* old_thread = part->thread_current;
    pok_thread_t* new_thread;


    assert(part->mode != POK_PARTITION_MODE_IDLE); // Idle mode is implemented via function with preemption disabled.
    
    if(part->mode != POK_PARTITION_MODE_NORMAL)
    {
        new_thread = &part->threads[POK_PARTITION_ARINC_MAIN_THREAD_ID];
        if(new_thread->state != POK_STATE_STOPPED)
        {
            assert(new_thread->state == POK_STATE_STOPPED);
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
    else if(part->thread_error->state != POK_STATE_STOPPED)
    {
        // Continue error handler
        new_thread = part->thread_error;
        part->base_part.is_error_handler = TRUE;
    }
    else if(!list_empty(&part->error_list))
    {
        // Start error handler (aperiodic, no timeout).
        pok_thread_t* new_thread = part->thread_error;
        
        new_thread->priority = new_thread->base_priority;
        new_thread->sp = 0;
      
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
        
        pok_context_restart(&new_thread->initial_sp, &thread_start_func,
            &new_thread->sp);
    }

    // Switch between different threads
    part->thread_current = new_thread;
    
    uint32_t* old_sp;
    uint32_t new_sp;
    
    if(new_thread)
    {
        new_sp = new_thread->sp;
        if(new_sp == 0)
        {
            new_sp = pok_context_init(
                pok_dstack_get_stack(&new_thread->initial_sp),
                &thread_start_func);
            new_thread->sp = new_sp;
        }
    }
    else
    {
        // New thread is "do_nothing"
        new_sp = pok_context_init(
                pok_dstack_get_stack(&part->base_part.initial_sp),
                &do_nothing_func);
    }

    if(old_thread)
    {
        old_sp = &old_thread->sp;
        /*
         * If old_thread->sp is 0, this should be processed upon
         * returning to given thread, not now.
         */
        if(*old_sp == 0) old_sp = NULL;
    }
    else
    {
        old_sp = NULL;
    }
    
    if(old_sp)
    {
        pok_context_switch(old_sp, new_sp);
    }
    else
    {
        pok_context_jump(new_sp);
    }
}

void pok_preemption_local_disable(void)
{
    assert(!current_partition_arinc->base_part.preempt_local_disabled);
    
    flag_set(current_partition_arinc->base_part.preempt_local_disabled);
}

void pok_preemption_local_enable(void)
{
    assert(current_partition_arinc->base_part.preempt_local_disabled);
    
    sched_arinc();
    
    barrier();
    
    flag_reset(current_partition_arinc->base_part.preempt_local_disabled);
    
    // Check that we do not miss events since sched_arinc() starts.
    
    while(ACCESS_ONCE(current_partition_arinc->base_part.state.bytes_all))
    {
        flag_set(current_partition_arinc->base_part.preempt_local_disabled);

        sched_arinc();
        
        barrier();
        
        flag_reset(current_partition_arinc->base_part.preempt_local_disabled);
    }
}

void pok_sched_arinc_on_event(void)
{
    sched_arinc();
}

void pok_sched_local_invalidate(void)
{
    flag_set(current_partition_arinc->sched_local_recheck_needed);
}
