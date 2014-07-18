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
 * This file also incorporates work covered by the following 
 * copyright and license notice:
 *
 *  Copyright (C) 2013-2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Created by julien on Thu Jan 15 23:34:13 2009
 */

/**
 **\\file   sched.c
 **\\brief  Function for partitions and kernel scheduling
 **\\author Julien Delange
 */

#if defined (POK_NEEDS_SCHED) || defined (POK_NEEDS_THREADS)

#include <types.h>
#include <arch.h>

#include <core/time.h>
#include <core/sched.h>
#include <core/thread.h>

#ifdef POK_NEEDS_PARTITIONS
#include <core/partition.h>
#endif

#ifdef POK_NEEDS_MIDDLEWARE
#include <middleware/port.h>
#endif

#include <dependencies.h>

#include <core/debug.h>
#include <core/instrumentation.h>
#include <core/error.h>

extern pok_thread_t       pok_threads[];

#ifdef POK_NEEDS_PARTITIONS
extern pok_partition_t    pok_partitions[];

/**
 * \brief The variable that contains the value of partition currently being executed
 */
uint8_t                   pok_current_partition;

void                      pok_sched_partition_switch();
#endif

#if defined (POK_NEEDS_PORTS_SAMPLING) || defined (POK_NEEDS_PORTS_QUEUEING)
void pok_port_flushall (void);
#endif

uint64_t           pok_sched_slots[POK_CONFIG_SCHEDULING_NBSLOTS]
                              = (uint64_t[]) POK_CONFIG_SCHEDULING_SLOTS;
uint8_t             pok_sched_slots_allocation[POK_CONFIG_SCHEDULING_NBSLOTS]
                              = (uint8_t[]) POK_CONFIG_SCHEDULING_SLOTS_ALLOCATION;

uint64_t            pok_sched_next_deadline;
uint64_t            pok_sched_next_major_frame;
uint8_t             pok_sched_current_slot = 0; /* Which slot are we executing at this time ?*/
pok_thread_id_t     current_thread = KERNEL_THREAD;

void pok_sched_thread_switch (void);

/**
 *\\brief Init scheduling service
 */

void pok_sched_init (void)
{
#ifdef POK_NEEDS_PARTITIONS 
#if defined (POK_NEEDS_ERROR_HANDLING) || defined (POK_NEEDS_DEBUG)
   /*
    * We check that the total time of time frame
    * corresponds to the sum of each slot
    */
   uint64_t                      total_time;
   uint8_t                       slot;

   total_time = 0;

   for (slot = 0 ; slot < POK_CONFIG_SCHEDULING_NBSLOTS ; slot++)
   {
      total_time = total_time + pok_sched_slots[slot];
   }

   if (total_time != POK_CONFIG_SCHEDULING_MAJOR_FRAME)
   {
#ifdef POK_NEEDS_DEBUG
      printf ("Major frame is not compliant with all time slots\n");
#endif
#ifdef POK_NEEDS_ERROR_HANDLING
      pok_error_raise_kernel (POK_ERROR_KIND_KERNEL_CONFIG);
#endif
   }
#endif
#endif

   pok_sched_current_slot        = 0;
   pok_sched_next_major_frame    = POK_CONFIG_SCHEDULING_MAJOR_FRAME;
   pok_sched_next_deadline       = pok_sched_slots[0];
   pok_current_partition         = pok_sched_slots_allocation[0];
}

#ifdef POK_NEEDS_PARTITIONS
pok_partition_id_t pok_elect_partition(void)
{
  pok_partition_id_t next_partition = POK_SCHED_CURRENT_PARTITION;
  uint64_t now = POK_GETTICK();

  if (pok_sched_next_deadline <= now)
  {
      /* Here, we change the partition */
#  if defined (POK_NEEDS_PORTS_SAMPLING) || defined (POK_NEEDS_PORTS_QUEUEING)
    if (pok_sched_next_major_frame <= now)
    {
      pok_sched_next_major_frame = pok_sched_next_major_frame +	POK_CONFIG_SCHEDULING_MAJOR_FRAME;
      pok_port_flushall();
    }
#  endif /* defined (POK_NEEDS_PORTS....) */

    pok_sched_current_slot = (pok_sched_current_slot + 1) % POK_CONFIG_SCHEDULING_NBSLOTS;
    pok_sched_next_deadline = pok_sched_next_deadline + pok_sched_slots[pok_sched_current_slot];
/*
    *  FIXME : current debug session about exceptions-handled
      printf ("Switch from partition %d to partition %d\n", pok_current_partition, pok_sched_current_slot);
      printf ("old current thread = %d\n", POK_SCHED_CURRENT_THREAD);

      printf ("new current thread = %d\n", pok_partitions[pok_sched_current_slot].current_thread);
      printf ("new prev current thread = %d\n", pok_partitions[pok_sched_current_slot].prev_thread);
      */
    next_partition = pok_sched_slots_allocation[pok_sched_current_slot];
  }

  return next_partition;
}
#endif /* POK_NEEDS_PARTITIONS */

#ifdef POK_TEST_SUPPORT_PRINT_WHEN_ALL_THREADS_STOPPED
static void check_all_threads_stopped(void) {
    // flag used to print message only once, not in loop
    static pok_bool_t check_passed = FALSE; 

    if (check_passed) return;

    size_t part_id;
    for (part_id = 0; part_id < POK_CONFIG_NB_PARTITIONS; part_id++) {
        const pok_partition_t *part = &pok_partitions[part_id];
        
        switch (part->mode) {
            case POK_PARTITION_MODE_INIT_COLD:
            case POK_PARTITION_MODE_INIT_WARM:
                // check that main thread is not stopped
                if (pok_threads[part->thread_main].state != POK_STATE_STOPPED) {
                    return;
                }
                break;
            case POK_PARTITION_MODE_NORMAL:
                // check all threads
                {
                    size_t low = part->thread_index_low,
                           high= low + part->thread_index;
                    size_t thread;
                    for (thread = low; thread < high; thread++) {
                        if (pok_threads[thread].state != POK_STATE_STOPPED) {
                            return;
                        }
                    }
                }
                break;
            case POK_PARTITION_MODE_IDLE:
                break;
        }
    }
    check_passed = TRUE;
    printf("POK: all threads have stopped\n");
}
#endif

#ifdef POK_NEEDS_PARTITIONS

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
#ifdef POK_NEEDS_INSTRUMENTATION
          if ( (elected != IDLE_THREAD) && (elected != new_partition->thread_main))
          {
            pok_instrumentation_running_task (elected);
          }
#endif
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
#endif /* POK_NEEDS_PARTITIONS */

#ifdef POK_NEEDS_PARTITIONS
void pok_sched()
{
    pok_thread_id_t elected_thread = 0;
    pok_partition_id_t elected_partition = POK_SCHED_CURRENT_PARTITION;
    
    elected_partition = pok_elect_partition();

    // set global
    pok_current_partition = elected_partition;

#ifdef POK_NEEDS_ERROR_HANDLING
    pok_error_check_deadlines();
#endif
    
    elected_thread = pok_elect_thread();
   
    if (pok_partitions[pok_current_partition].current_thread != elected_thread) {
        if (pok_partitions[pok_current_partition].current_thread != IDLE_THREAD) {
            // prev_thread is never IDLE_THREAD (because it's not relevant)
	    pok_partitions[pok_current_partition].prev_thread = pok_partitions[pok_current_partition].current_thread;
	}
	pok_partitions[pok_current_partition].current_thread = elected_thread;
    }
    pok_sched_context_switch(elected_thread);
}
#else
void pok_sched_thread_switch ()
{
   int i;
   uint64_t now;
   pok_thread_id_t elected;

   now = POK_GETTICK();
   for (i = 0; i <= POK_CONFIG_NB_THREADS; ++i)
   {
     if ((pok_threads[i].state == POK_STATE_WAITING) &&
	 (pok_threads[i].wakeup_time <= now))
      {
         pok_threads[i].state = POK_STATE_RUNNABLE;
      }
   }

   elected = pok_sched_part_election (0, POK_CONFIG_NB_THREADS);
   /*
    *  FIXME : current debug session about exceptions-handled
   printf ("switch to thread %d\n", elected);
   */
   pok_sched_context_switch(elected);
}
#endif /* POK_NEEDS_PARTITIONS */

/*
 * Context-switch function to switch from one thread to another
 * Rely on architecture-dependent functionnalities (must include arch.h)
 */
void pok_sched_context_switch(pok_thread_id_t elected_id)
{
   uint32_t *current_sp;
   uint32_t new_sp;
   

   if (POK_SCHED_CURRENT_THREAD == elected_id && !POK_CURRENT_THREAD.force_restart)
   {
      return;
   }

   current_sp = &POK_CURRENT_THREAD.sp;
   new_sp = pok_threads[elected_id].sp;
/*
    *  FIXME : current debug session about exceptions-handled
   printf("switch from thread %d, sp=0x%x\n",POK_SCHED_CURRENT_THREAD, current_sp);
   printf("switch to thread %d, sp=0x%x\n",elected_id, new_sp);
   */
   pok_space_switch(POK_CURRENT_THREAD.partition,
		    pok_threads[elected_id].partition);

   current_thread = elected_id;
   
   POK_CURRENT_THREAD.force_restart = FALSE;

   pok_context_switch(current_sp, new_sp);
}

#if defined (POK_NEEDS_LOCKOBJECTS) || defined (POK_NEEDS_PORTS_QUEUEING) || defined (POK_NEEDS_PORTS_SAMPLING)
void pok_sched_unlock_thread(pok_thread_id_t thread_id)
{
   pok_threads[thread_id].state = POK_STATE_RUNNABLE;
}
#endif

#if defined (POK_NEEDS_LOCKOBJECTS) || defined (POK_NEEDS_PORTS_QUEUEING) || defined (POK_NEEDS_PORTS_SAMPLING)
void pok_sched_lock_current_thread(void)
{
   pok_threads[current_thread].state = POK_STATE_LOCK;
}

// TODO remove this function
void pok_sched_lock_current_thread_timed(uint64_t time)
{
   pok_threads[current_thread].state = POK_STATE_WAITING;
   pok_threads[current_thread].wakeup_time = time;
}
#endif



#ifdef POK_NEEDS_DEPRECIATED
void pok_sched_lock_thread(pok_thread_id_t thread_id)
{
   pok_threads[thread_id].state = POK_STATE_LOCK;
}
#endif

pok_ret_t pok_sched_end_period(void)
{
    // called by periodic process when it's done its work
    // ARINC-653 PERIODIC_WAIT

    if (!pok_thread_is_periodic(&POK_CURRENT_THREAD)) {
        return POK_ERRNO_MODE;
    }

    if (POK_CURRENT_PARTITION.lock_level > 0) {
        return POK_ERRNO_MODE;
    }

    POK_CURRENT_THREAD.state = POK_STATE_WAIT_NEXT_ACTIVATION;
    POK_CURRENT_THREAD.next_activation += POK_CURRENT_THREAD.period;
    if (POK_CURRENT_THREAD.time_capacity >= 0) {
        // set deadline
        POK_CURRENT_THREAD.end_time = POK_CURRENT_THREAD.next_activation + POK_CURRENT_THREAD.time_capacity;
    }

    pok_sched();
    return POK_ERRNO_OK;
}

pok_ret_t pok_sched_replenish(int64_t budget)
{
    if (pok_thread_is_error_handling(&POK_CURRENT_THREAD)) {
        return POK_ERRNO_UNAVAILABLE;
    }
    if (POK_CURRENT_PARTITION.mode != POK_PARTITION_MODE_NORMAL) {
        return POK_ERRNO_UNAVAILABLE;
    }
    
    int64_t calculated_deadline;
    if (budget < 0) {
        calculated_deadline = -1; // infinite
    } else {
        calculated_deadline = POK_GETTICK() + budget;
    }

    if (pok_thread_is_periodic(&POK_CURRENT_THREAD)) {
        // for periodic processes, deadline must never
        // exceed next release point

        if (calculated_deadline < 0) {
            return POK_ERRNO_MODE;
        }

        // since process replenishes budget of itself,
        // we can be sure that process is running
        // so POK_CURRENT_THREAD.next_activation
        // is the time of _current_ activation
        int64_t next_activation = POK_CURRENT_THREAD.next_activation + POK_CURRENT_THREAD.period;

        if (calculated_deadline > next_activation) {
            return POK_ERRNO_MODE;
        }
    }

    POK_CURRENT_THREAD.end_time = calculated_deadline;

    return POK_ERRNO_OK;
}

#ifdef POK_NEEDS_PARTITIONS

uint32_t pok_sched_get_current(pok_thread_id_t *thread_id)
{
    if (KERNEL_THREAD == POK_SCHED_CURRENT_THREAD 
        || IDLE_THREAD == POK_SCHED_CURRENT_THREAD)
    {
      return POK_ERRNO_THREAD;
    }

    if (POK_CURRENT_PARTITION.thread_main == POK_SCHED_CURRENT_THREAD) {
        return POK_ERRNO_THREAD;
    }

#ifdef POK_NEEDS_ERROR_HANDLING
    if (pok_thread_is_error_handling(&POK_CURRENT_THREAD)) {
        return POK_ERRNO_THREAD;
    }
#endif
    
    *thread_id= POK_SCHED_CURRENT_THREAD;
    return POK_ERRNO_OK;
}
#endif

#endif /* __POK_NEEDS_SCHED */
