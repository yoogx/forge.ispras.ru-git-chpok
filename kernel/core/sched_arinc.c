#include <core/sched_arinc.h>

pok_thread_t* current_thread;

uint8_t current_space_id;

static void thread_deadline_occured(struct delayed_event* event)
{
    pok_thread_t* thread = container_of(event, typeof(*thread), thread_deadline_event);
    // TODO: fire deadline
}

static void thread_delayed_event(struct delayed_event* event)
{
    pok_thread_t* thread = container_of(event, typeof(*thread), thread_delayed_event.base_event);
    thread->thread_delayed_event.process_thread_event(thread);
}


static void pok_sched_arinc(void)
{
    pok_partition_arinc_t* part = currentPartitionARINC;
    pok_time_t now;
    
    pok_thread_t* oldThread = currentThread;
    
    uint32_t* sp_old = oldThread? &oldThread->sp : &part->base_part.sp;
    uint32_t* sp_new;
    
    if(!pok_sched_local_check_invalidated()) return;

    if(pok_sched_local_check_slot_started())
    {
        // Switch user space
        pok_preemption_disable();
        pok_space_switch(pok_partition_get_space(part);
        pok_preemption_enable();
        
        // TODO: process ports here.
        

    }
    
    now = POK_GETTICK();
    
    timer_check(&part->timer_deadline, now, &thread_deadline_occured);
    
    timer_check(&part->timer_delayed, now, &thread_delyed_event);
    
    if(0)
    {
    }
#ifdef POK_NEEDS_ERROR_HANDLING
    else if(part->thread_error->state != POK_STATE_STOPPED)
    {
        currentThread = part->thread_error;
    }
#endif
    else if(part->lock_level)
    {
        currentThread = part->thread_locked;
    }
    else if(!list_empty(&part->eligible_threads))
    {
        currentThread = list_first_entry(&part->eligible_threads,
            pok_thread_t, eligible_elem);
    }
    else
    {
        currentThread = NULL;
    }

    if(oldThread == currentThread) return; // TODO: Process thread restarting.
    
    sp_new = oldThread? &oldThread->sp : &part->base_part.sp;
    
    pok_context_switch(old_sp, *new_sp);
}

static void pok_unlock_sleeping_threads(pok_partition_t *new_partition)
{
   pok_thread_id_t i;
   uint64_t now = POK_GETTICK();
   for (i = 0; i < new_partition->nthreads; i++)
   {
     pok_thread_t *thread = &pok_threads[new_partition->thread_index_low + i];

#if defined (POK_NEEDS_LOCKOBJECTS) || defined (POK_NEEDS_PORTS_QUEUEING) || defined (POK_NEEDS_PORTS_SAMPLING)
     if ((thread->state == POK_STATE_WAITING) && (thread->wakeup_time <= now))
     {
       thread->state = POK_STATE_RUNNABLE;
     }
#endif

     if ((thread->state == POK_STATE_WAIT_NEXT_ACTIVATION) && (thread->next_activation <= now))
     {
       thread->state = POK_STATE_RUNNABLE;
       thread->next_activation = thread->next_activation + thread->period; 
     }

     if (thread->suspended && thread->suspend_timeout <= now) {
       thread->suspended = FALSE;
     }
   }
}

#ifdef POK_NEEDS_ERROR_HANDLING
static void
pok_error_check_deadlines(void)
{
    pok_thread_id_t i;
    uint64_t now = POK_GETTICK();

    pok_thread_id_t low = POK_CURRENT_PARTITION.thread_index_low,
                    high = POK_CURRENT_PARTITION.thread_index;

    for (i = low; i < high; i++) {
        pok_thread_t *thread = &pok_threads[i];

        if (POK_CURRENT_PARTITION.thread_error_created &&
            pok_threads[POK_CURRENT_PARTITION.thread_error].state != POK_STATE_STOPPED)
        {
            // we're already handling an HM event
            // (be it another deadline or something else)
            // postpone reporting more deadline misses
            break;
        }

        if (thread->state == POK_STATE_STOPPED) {
            continue;
        }
        
        if (thread->end_time >= 0 && (uint64_t) thread->end_time < now) {
			// Implementation dependent (ARINC-653 P.1-3 page 29)
			// This is the second variant
			thread->next_activation += thread->period;
			thread->end_time = thread->next_activation;
            // deadline miss HM event
            pok_error_raise_thread(POK_ERROR_KIND_DEADLINE_MISSED, i, NULL, 0);
        }
    }
}
#endif

static pok_thread_id_t pok_elect_thread(void)
{
   pok_partition_t* new_partition = &POK_CURRENT_PARTITION;
    
   pok_unlock_sleeping_threads(new_partition);

   /*
    * We elect the thread to be executed.
    */
   pok_thread_id_t elected;
   switch (new_partition->mode)
   {
      case POK_PARTITION_MODE_INIT_COLD:
      case POK_PARTITION_MODE_INIT_WARM:
#ifdef POK_NEEDS_ERROR_HANDLING
         // error handler is active in NORMAL mode only
#endif
         elected = new_partition->thread_main;

         if (pok_threads[elected].state != POK_STATE_RUNNABLE) {
            // if main thread is stopped, don't schedule it
            elected = IDLE_THREAD;
         }

         break;

      case POK_PARTITION_MODE_NORMAL:
#ifdef POK_NEEDS_ERROR_HANDLING
         if (new_partition->thread_error_created &&
             pok_threads[new_partition->thread_error].state == POK_STATE_RUNNABLE)
         {

            elected = new_partition->thread_error;
            break;
         }
#endif
         if (new_partition->lock_level > 0 && pok_threads[new_partition->current_thread].state == POK_STATE_RUNNABLE) {
            elected = new_partition->current_thread;
            break;
         }
      
          elected = new_partition->scheduler->elect_thread();
         break;

      default:
         elected = IDLE_THREAD;
         break;
   }

#ifdef POK_TEST_SUPPORT_PRINT_WHEN_ALL_THREADS_STOPPED
   if (elected == IDLE_THREAD) {
      check_all_threads_stopped();
   }
#endif

   return elected;
}

