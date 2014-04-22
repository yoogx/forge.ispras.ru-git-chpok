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
 * \file    core/thread.c
 * \author  Julien Delange
 * \date    2008-2009
 * \brief   Thread management in kernel
 */

#include <types.h>

#include <arch.h>
#include <core/debug.h>
#include <core/error.h>
#include <core/thread.h>
#include <core/sched.h>
#include <core/partition.h>
#include <core/time.h>

#include <core/instrumentation.h>

#ifdef POK_NEEDS_THREADS

/**
 * We declare an array of threads. The amount of threads
 * is fixed by the software developper and we add two theads
 *    - one for the kernel thread (this code)
 *    - one for the idle task
 */
pok_thread_t		         pok_threads[POK_CONFIG_NB_THREADS];

extern pok_partition_t     pok_partitions[POK_CONFIG_NB_PARTITIONS];


/**
 * Initialize threads array, put their default values
 * and so on
 */
void pok_thread_init(void)
{
   pok_thread_id_t i;

   pok_threads[KERNEL_THREAD].priority	   = 0;
   pok_threads[KERNEL_THREAD].base_priority	   = 0;
   pok_threads[KERNEL_THREAD].state		   = POK_STATE_RUNNABLE;
   pok_threads[KERNEL_THREAD].next_activation = 0;

   pok_threads[IDLE_THREAD].period                     = 0;
   pok_threads[IDLE_THREAD].deadline                   = 0;
   pok_threads[IDLE_THREAD].time_capacity              = 0;
   pok_threads[IDLE_THREAD].next_activation            = 0;
   pok_threads[IDLE_THREAD].remaining_time_capacity    = 0;
   pok_threads[IDLE_THREAD].wakeup_time		       = 0;
   pok_threads[IDLE_THREAD].entry		       = pok_arch_idle;
   pok_threads[IDLE_THREAD].base_priority		       = 0;
   pok_threads[IDLE_THREAD].state		       = POK_STATE_RUNNABLE;

   pok_threads[IDLE_THREAD].sp			       = pok_context_create
                                                   (IDLE_THREAD,								             IDLE_STACK_SIZE,
						   (uint32_t)pok_arch_idle);

   for (i = 0; i < POK_CONFIG_NB_THREADS; ++i)
   {
      pok_threads[i].period                     = 0;
      pok_threads[i].deadline                   = 0;
      pok_threads[i].time_capacity              = 0;
      pok_threads[i].remaining_time_capacity    = 0;
      pok_threads[i].next_activation            = 0;
      pok_threads[i].wakeup_time                = 0;
      pok_threads[i].state                      = POK_STATE_STOPPED;
  }
}

#ifdef POK_NEEDS_PARTITIONS
/**
 * Create a thread inside a partition
 * Return POK_ERRNO_OK if no error.
 * Return POK_ERRNO_TOOMANY if the partition cannot contain
 * more threads.
 */
pok_ret_t pok_partition_thread_create (pok_thread_id_t*         thread_id,
                                       const pok_thread_attr_t  *attr,
                                       pok_partition_id_t       partition_id)
{
   pok_thread_id_t id;
   uintptr_t stack_vaddr;
   /**
    * We can create a thread only if the partition is in INIT mode
    */
   if (  (pok_partitions[partition_id].mode != POK_PARTITION_MODE_INIT_COLD) &&
         (pok_partitions[partition_id].mode != POK_PARTITION_MODE_INIT_WARM) )
   {
      return POK_ERRNO_PARTITION_MODE;
   }

   if (pok_partitions[partition_id].thread_index >= pok_partitions[partition_id].thread_index_high)
   {
#ifdef POK_NEEDS_ERROR_HANDLING
      POK_ERROR_CURRENT_PARTITION (POK_ERROR_KIND_PARTITION_CONFIGURATION);
#endif
      return POK_ERRNO_TOOMANY;
   }

   if (attr->stack_size != POK_USER_STACK_SIZE) {
      return POK_ERRNO_PARAM; // XXX refine error codes
   }
   if (attr->period == 0) {
       return POK_ERRNO_PARAM; // XXX same
   }
   if (attr->time_capacity == 0) {
       return POK_ERRNO_EINVAL; // XXX same
   }
   if (attr->time_capacity > 0 && attr->period > 0 && attr->time_capacity > attr->period) {
       // for periodic process, time capacity <= period
       return POK_ERRNO_EINVAL; // XXX same
   }
   if (attr->time_capacity < 0 && attr->period > 0) {
       // periodic process must have definite time capacity
       return POK_ERRNO_EINVAL; // XXX same
   }

   // do at least basic check of entry point
   if (!POK_CHECK_VPTR_IN_PARTITION(partition_id, attr->entry)) {
      return POK_ERRNO_EINVAL;
   }

   id = pok_partitions[partition_id].thread_index_low +  pok_partitions[partition_id].thread_index;
   pok_partitions[partition_id].thread_index =  pok_partitions[partition_id].thread_index + 1;

   // on creation, both priorities are equal
   pok_threads[id].priority      = attr->priority;
   pok_threads[id].base_priority      = attr->priority;

   pok_threads[id].period = attr->period;

   // XXX does it make sense?
   pok_threads[id].next_activation = attr->period;

   pok_threads[id].deadline = attr->deadline;

#ifdef POK_NEEDS_SCHED_HFPPS

   pok_threads[id].payback = 0;
#endif /* POK_NEEDS_SCHED_HFPPS */

   pok_threads[id].time_capacity = attr->time_capacity;
   // XXX does it make sense?
   //pok_threads[id].remaining_time_capacity = attr->time_capacity;

   stack_vaddr = pok_thread_stack_addr (partition_id, pok_partitions[partition_id].thread_index);

   pok_threads[id].state		   = POK_STATE_RUNNABLE;
   pok_threads[id].wakeup_time   = 0;
   pok_threads[id].sp		      = pok_space_context_create (partition_id,
                                                             (uint32_t)attr->entry,
                                                             stack_vaddr,
                                                             0xdead,
                                                             0xbeaf);
   /*
    *  FIXME : current debug session about exceptions-handled
   printf ("thread sp=0x%x\n", pok_threads[id].sp);
   printf ("thread stack vaddr=0x%x\n", stack_vaddr);
   */
   pok_threads[id].partition        = partition_id; 
   pok_threads[id].entry            = attr->entry;
   pok_threads[id].init_stack_addr  = stack_vaddr;
   *thread_id = id;

   POK_CURRENT_PARTITION.scheduler->enqueue_thread(id);

#ifdef POK_NEEDS_INSTRUMENTATION
      pok_instrumentation_task_archi (id);
#endif

   return POK_ERRNO_OK;
}
#endif


/**
 * Start a thread, giving its entry call with \a entry
 * and its identifier with \a id
 */
void pok_thread_start(void (*entry)(), unsigned int id)
{
   (void) id;
   entry();
}

#ifdef POK_NEEDS_THREAD_SLEEP
pok_ret_t pok_thread_sleep(int64_t time)
{
    // TODO forbid sleeping of periodic processes
    
    // TODO forbid sleeping when preemption is disabled
    
    if (time == 0) {
        return POK_ERRNO_OK;
    }
    
    uint64_t wakeup_time;
    if (time < 0) {
        wakeup_time = (uint64_t) -1; // TODO find a better way
    } else {
        wakeup_time = POK_GETTICK() + time;
    }
    
    POK_CURRENT_THREAD.state = POK_STATE_WAITING;
    POK_CURRENT_THREAD.wakeup_time = wakeup_time;

    pok_sched();

    if (POK_GETTICK() >= wakeup_time) {
        return POK_ERRNO_TIMEOUT;
    } else {
        // somebody else woke up this process
        return POK_ERRNO_OK;
    }
}
#endif

#ifdef POK_NEEDS_THREAD_SLEEP_UNTIL
pok_ret_t pok_thread_sleep_until(uint64_t time)
{
   pok_sched_lock_current_thread_timed((uint64_t)time);
   pok_sched();
   return POK_ERRNO_OK;
}
#endif

pok_ret_t pok_thread_yield (void)
{
   pok_sched();
   return POK_ERRNO_OK;
}

#ifdef POK_NEEDS_ERROR_HANDLING
pok_ret_t pok_thread_restart(pok_thread_id_t tid)
{
   /**
    * Reinit timing values
    */

   pok_threads[tid].remaining_time_capacity  = pok_threads[tid].time_capacity;
   pok_threads[tid].state                    = POK_STATE_WAIT_NEXT_ACTIVATION;
   pok_threads[tid].wakeup_time              = 0;

   /**
    * Newer solution for later improvements.
    *
    pok_space_context_restart (pok_threads[tid].sp, (uint32_t) pok_threads[tid].entry, pok_threads[tid].init_stack_addr);
    *
    */

   /**
    * At this time, we build a new context for the thread.
    * It is not the best solution but it works at this time
    */
   pok_threads[tid].sp		      = pok_space_context_create (pok_threads[tid].partition,
                                                             (uint32_t)pok_threads[tid].entry,
                                                             pok_threads[tid].init_stack_addr,
                                                             0xdead,
                                                             0xbeaf);

   return POK_ERRNO_OK;
}
#endif

pok_ret_t pok_thread_delayed_start (pok_thread_id_t id, int64_t ms)
{
    if (POK_CURRENT_PARTITION.thread_index_low > id || POK_CURRENT_PARTITION.thread_index_high < id) {
        return POK_ERRNO_THREADATTR;
    }

    if (ms < 0) {
        return POK_ERRNO_EINVAL;
    }
    
    pok_thread_t *thread = &pok_threads[id];

    thread->priority = thread->base_priority;

    if (thread->state != POK_STATE_STOPPED) {
        return POK_ERRNO_UNAVAILABLE;
    }

    if (thread->period >= 0 && ms >= thread->period) {
        return POK_ERRNO_EINVAL;
    }
  
    //reset stack
    pok_context_reset(POK_USER_STACK_SIZE, thread->init_stack_addr);
  
    if (thread->period < 0) { //-1 <==> ARINC INFINITE_TIME_VALUE
        // aperiodic process
        if (pok_partitions[thread->partition].mode == POK_PARTITION_MODE_NORMAL) {
  	    if (ms == 0) {
  	        thread->state = POK_STATE_RUNNABLE;
  	    } else {
  	        thread->state = POK_STATE_WAITING;
  	        thread->wakeup_time = POK_GETTICK() + ms;
  	    }

            // init DEADLINE_TIME (end_time)
            if (thread->time_capacity >= 0) {
                thread->end_time = POK_GETTICK() + ms + thread->time_capacity;
            } else {
                thread->end_time = -1;
            }

	    //the preemption is always enabled so
            // XXX no, it isn't
  	    pok_sched();
  	} else { //the partition mode is cold or warm start
  	    thread->state = POK_STATE_DELAYED_START;
	    thread->wakeup_time = ms; // temporarly storing the delay, see set_partition_mode
  	}
    } else {
        // periodic process
        if (pok_partitions[thread->partition].mode == POK_PARTITION_MODE_NORMAL) { // set the first release point
	    thread->next_activation = ms + POK_GETTICK();
	    thread->end_time = thread->next_activation + thread->deadline;
        } else {
	    thread->state = POK_STATE_DELAYED_START;
	    thread->wakeup_time = ms; // temporarly storing the delay, see set_partition_mode
        }
    }
    return POK_ERRNO_OK;
}

pok_ret_t pok_thread_get_status (pok_thread_id_t id, pok_thread_status_t *status)
{
  if (POK_CURRENT_PARTITION.thread_index_low > id || POK_CURRENT_PARTITION.thread_index_high < id)
    return POK_ERRNO_PARAM;

  // TODO  ensure that thread is created

  pok_thread_t *t = &pok_threads[id];

  status->attributes.priority       = t->base_priority;
  status->attributes.entry          = t->entry;
  status->attributes.period         = t->period;
  status->attributes.deadline       = t->deadline;
  status->attributes.time_capacity  = t->time_capacity;
  status->attributes.stack_size     = POK_USER_STACK_SIZE;

  status->state = t->state;
  status->deadline_time = t->end_time; 
  status->current_priority = t->priority;

  return POK_ERRNO_OK;
}

pok_ret_t pok_thread_set_priority(pok_thread_id_t id, uint32_t priority)
{
    // ensure that thread belongs to the partition, and is created
    if (id < POK_CURRENT_PARTITION.thread_index_low || id >= POK_CURRENT_PARTITION.thread_index) {
        return POK_ERRNO_THREADATTR;
    }
    pok_thread_t *thread = &pok_threads[id];
    
    // TODO if thread is stopped?
    if (thread->state == POK_STATE_STOPPED) {
        return POK_ERRNO_UNAVAILABLE;
    }

    thread->priority = priority;
    /* preemption is always enabled so ... */
    // XXX no, it isn't
    pok_sched();
    return POK_ERRNO_OK;
}

pok_ret_t pok_thread_resume(pok_thread_id_t id)
{
    // ensure that thread belongs to the partition, and is created
    if (id < POK_CURRENT_PARTITION.thread_index_low || id >= POK_CURRENT_PARTITION.thread_index) {
        return POK_ERRNO_THREADATTR;
    }

    // can't resume, self, lol
    if (id == POK_SCHED_CURRENT_THREAD) {
        return POK_ERRNO_THREADATTR;
    }
    
    pok_thread_t *thread = &pok_threads[id];

    // TODO check that thread was indeed suspended

    // it wasn't suspended at all
    if (thread->state == POK_STATE_RUNNABLE) {
        return POK_ERRNO_UNAVAILABLE;
    }

    // the thread is not waiting 
    // TODO distinguish between waiting for resource
    // and sleeping for specified timeout
    if (thread->state != POK_STATE_WAITING) {
        return POK_ERRNO_MODE;
    }
	
    thread->wakeup_time = POK_GETTICK();
    thread->state = POK_STATE_RUNNABLE;

    /* preemption is always enabled */
    // XXX no, it isn't
    pok_sched();
    return POK_ERRNO_OK;
}

pok_ret_t pok_thread_suspend_target(pok_thread_id_t id)
{
    // ensure that thread belongs to the partition, and is created
    if (id < POK_CURRENT_PARTITION.thread_index_low || id >= POK_CURRENT_PARTITION.thread_index) {
        return POK_ERRNO_THREADATTR;
    }
    pok_thread_t *thread = &pok_threads[id];

    if (id == POK_SCHED_CURRENT_THREAD) {
        // can't suspend current process
        // _using this function_
        // (use pok_thread_suspend instead)

        return POK_ERRNO_THREADATTR;
    }

    // TODO if preemption is disabled
    // and it's error handling process
    // and target is the process that started the EH process
    // it's an error
    
    // can't suspend stopped (dormant) process
    if (thread->state == POK_STATE_STOPPED) {
        return POK_ERRNO_UNAVAILABLE;
    } 

    // TODO distinguish suspended vs. waiting on lock process
    if (thread->state == POK_STATE_WAITING) {
        return POK_ERRNO_MODE;
    }

    thread->state = POK_STATE_WAITING;
    thread->wakeup_time = (uint64_t)-1; // TODO find a better way

    // XXX call scheduler?
    pok_sched();

    return POK_ERRNO_OK;
}

pok_ret_t pok_thread_suspend(void)
{
    // TODO forbid suspend for error handling process

    // TODO forbid suspend if preemption is locked
    
    POK_CURRENT_THREAD.state = POK_STATE_WAITING;
    POK_CURRENT_THREAD.wakeup_time = (uint64_t)-1; // TODO find a better way
    
    pok_sched();

    return POK_ERRNO_OK;
}

pok_ret_t pok_thread_stop_target(pok_thread_id_t id)
{
    if (id < POK_CURRENT_PARTITION.thread_index_low || id >= POK_CURRENT_PARTITION.thread_index) {
        return POK_ERRNO_THREADATTR;
    } 
    
    if (id == POK_SCHED_CURRENT_THREAD) {
        // can's stop self
        // use pok_thread_stop to do that
        return POK_ERRNO_THREADATTR;
    }
    
    pok_thread_t *thread = &pok_threads[id];
    
    if (thread->state == POK_STATE_STOPPED) {
        return POK_ERRNO_UNAVAILABLE;
    }

    thread->state = POK_STATE_STOPPED;

    return POK_ERRNO_OK;
}

pok_ret_t pok_thread_stop(void)
{
    POK_CURRENT_THREAD.state = POK_STATE_STOPPED;
    pok_sched();
    
    return POK_ERRNO_OK;
}

#endif
