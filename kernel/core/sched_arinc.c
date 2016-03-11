#include <core/sched_arinc.h>
#include <core/delayed_event.h>
#include <core/thread.h>
#include "thread_internal.h"

//pok_thread_t* current_thread;

//uint8_t current_space_id;

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

// Called with local preemption disabled.
static void sched_arinc(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    
    pok_thread_t* oldThread = part->thread_current;
    
    uint32_t* sp_old = oldThread? &oldThread->sp : &part->base_part.sp;
    uint32_t* sp_new;
    
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

    assert(part->mode != POK_PARTITION_MODE_IDLE);
    
    if(part->mode != POK_PARTITION_MODE_NORMAL)
    {
        part->thread_current = &part->threads[POK_PARTITION_ARINC_MAIN_THREAD_ID];
        assert(part->thread_current->state == POK_STATE_RUNNABLE);
    }
#ifdef POK_NEEDS_ERROR_HANDLING
    else if(part->thread_error->state != POK_STATE_STOPPED)
    {
        part->thread_current = part->thread_error;
    }
    else if(!list_empty(&part->error_list))
    {
        // Start error handler (aperiodic, no timeout).
        pok_thread_t* thread = part->thread_error;
        
        thread->priority = thread->base_priority;
        thread->sp = 0;
      
        thread->state = POK_STATE_RUNNABLE;
        
        part->thread_current = thread;
    }
#endif
    else if(part->lock_level)
    {
        part->thread_current = part->thread_locked;
    }
    else if(!list_empty(&part->eligible_threads))
    {
        part->thread_current = list_first_entry(&part->eligible_threads,
            pok_thread_t, eligible_elem);
    }
    else
    {
        part->thread_current = NULL;
    }

    if(oldThread == part->thread_current) return; // TODO: Process thread restarting. Is it needed?
    
    sp_new = oldThread? &oldThread->sp : &part->base_part.sp;
    
    pok_context_switch(sp_old, *sp_new);
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

void pok_sched_arinc_on_time_changed(void)
{
    // Just set corresponded flag and let generic mechanism to work.
    flag_set(current_partition_arinc->base_part.state.bytes.time_changed);
    
    pok_preemption_local_enable();
}
void pok_sched_arinc_on_control_returned(void)
{
    // Just set corresponded flag and let generic mechanism to work.
    flag_set(current_partition_arinc->base_part.state.bytes.control_returned);
    
    pok_preemption_local_enable();

}
