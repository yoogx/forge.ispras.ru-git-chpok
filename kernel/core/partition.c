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
 * \file partition.c
 * \brief This file provides functions for partitioning services
 * \author Julien Delange
 *
 * The definition of useful structures can be found in partition.h
 * header file. To enable partitioning services, you must set the
 * POK_NEEDS_PARTITIONS maccro.
 */

#include <config.h>

#ifdef POK_NEEDS_PARTITIONS

#include <arch.h>
#include <bsp.h>
#include <errno.h>
#include <dependencies.h>
#include <core/sched.h>
#include <core/sched_fps.h>
#include <core/error.h>
#include <core/debug.h>
#include <core/thread.h>
#include <core/partition.h>
#include <core/instrumentation.h>
#include <core/time.h>
#include <middleware/port.h>

#include <libc.h>
#include <assert.h>

uint8_t			 pok_partitions_index = 0;


/**
 **\brief Setup the scheduler used in partition pid
 */
void pok_partition_setup_scheduler (pok_partition_id_t pid)
{
    // TODO other schedulers?
    pok_partitions[pid].scheduler = &pok_fps_scheduler_ops;
    pok_partitions[pid].scheduler->initialize();
}

/**
 * Reinitialize partition.
 *
 * Is used when partition is restarted.
 * Also indirectly called by pok_partition_init,
 * when it's initialized for the first time.
 *
 * Note that caller is responsible for setting appropriate mode
 * beforehand (either warm or cold init).
 *
 */

void pok_partition_reinit (pok_partition_id_t pid)
{
   /*
    * FIXME: reset lockobjects (and their queues, too)
    * FIXME: free/reuse allocated memory (notably, kernel stacks)
    */
   pok_partition_t *part = &pok_partitions[pid];

   assert(part->mode == POK_PARTITION_MODE_INIT_COLD || part->mode == POK_PARTITION_MODE_INIT_WARM);

   pok_partition_setup_scheduler (pid);

   part->thread_index = part->thread_index_low;
   part->current_thread = IDLE_THREAD;
   part->prev_thread = IDLE_THREAD; 

#ifdef POK_NEEDS_ERROR_HANDLING
   part->thread_error_created = FALSE;
#endif

#if defined(POK_NEEDS_PORTS_QUEUEING) || defined(POK_NEEDS_PORTS_SAMPLING)
   pok_port_reset(pid); 
#endif
      
   part->lock_level = 1; // in init mode, lock level is always >0

   // reload code
   uintptr_t tmp;
   pok_arch_load_partition(pid, &tmp);
   part->thread_main_entry = tmp;

   pok_partition_setup_main_thread (pid);
   part->current_thread = part->thread_main;
}

/**
 * Setup the main thread of partition with number \a pid
 */
void pok_partition_setup_main_thread (pok_partition_id_t pid)
{
   pok_thread_id_t main_thread;
   pok_thread_attr_t attr;

   attr.entry = (void *) pok_partitions[pid].thread_main_entry;
   attr.priority = 1;
   attr.deadline = DEADLINE_SOFT;
   attr.period   = -1;
   attr.time_capacity = -1;
   attr.stack_size = POK_USER_STACK_SIZE;

   pok_partition_thread_create (&main_thread, &attr, pid);
   pok_partitions[pid].thread_main = main_thread;

   // hack
   pok_threads[main_thread].state = POK_STATE_RUNNABLE;
}

/**
 * \brief Initialize all partitions.
 *
 * It initializes everything, load the program, set thread
 * and lockobjects bounds.
 */
pok_ret_t pok_partition_init ()
{
   pok_partition_id_t i;
   pok_thread_id_t    threads_index = 0;
   pok_thread_id_t    total_threads = 0;

   uint32_t	*partition_size = POK_CONFIG_PARTITIONS_SIZE;

#ifdef POK_NEEDS_LOCKOBJECTS
   uint8_t lockobj_index = 0;
#endif

   for (i = 0 ; i < POK_CONFIG_NB_PARTITIONS ; i++)
   {
      size_t size = partition_size[i];
      uintptr_t base_addr = (uintptr_t) pok_bsp_alloc_partition(partition_size[i]);
      uintptr_t program_entry;
      uintptr_t base_vaddr = pok_space_base_vaddr(base_addr);
   
      pok_partition_t *part = &pok_partitions[i];
       
      /*
       * Set partition name
       */
        
      for (int j=0; j<POK_CONFIG_SCHEDULING_NBSLOTS; j++){
            if (pok_module_sched[j].type == POK_SLOT_PARTITION)
                    if(pok_module_sched[j].partition.id == i)
                            part->name=pok_module_sched[j].partition.name;
      }


      /*
       * Partition is not paused
       */
      
      part->is_paused=FALSE;
    
      /* 
       * Memory.
       */
      part->base_addr   = base_addr;
      part->base_vaddr  = base_vaddr;
      part->size        = size;
      
      pok_create_space (i, base_addr, size);
      
      pok_arch_load_partition(i, &program_entry);
      
      /*
       * Allocate threads
       */
      part->thread_index_low  = threads_index;
      part->nthreads          = POK_CONFIG_PARTITIONS_NTHREADS[i];

      total_threads += part->nthreads;

#ifdef POK_NEEDS_ERROR_HANDLING
      if (part->nthreads <= 1)
      {
         pok_error_raise_partition(i, POK_ERROR_KIND_PARTITION_CONFIGURATION);
      }
#endif

      part->thread_index_high = part->thread_index_low + POK_CONFIG_PARTITIONS_NTHREADS[i];
      part->activation        = 0; // FIXME that can't be right
      part->period            = POK_CONFIG_SCHEDULING_MAJOR_FRAME;
      part->duration          = POK_CONFIG_SCHEDULING_MAJOR_FRAME;
      part->current_thread    = IDLE_THREAD;
      part->prev_thread       = IDLE_THREAD; // breaks the rule of prev_thread not being idle, but it's just for init

      threads_index                       = threads_index + part->nthreads;

      /*
       * Allocate lock objects
       */
#ifdef POK_NEEDS_LOCKOBJECTS
      part->lockobj_index_low    = lockobj_index;
      part->lockobj_index_high   = lockobj_index + POK_CONFIG_PARTITIONS_NLOCKOBJECTS[i];
      part->nlockobjs            = POK_CONFIG_PARTITIONS_NLOCKOBJECTS[i];
      lockobj_index                          = lockobj_index + part->nlockobjs;
#endif
      
      /* Misc. variables */
      part->thread_main_entry = program_entry;
      part->start_condition = POK_START_CONDITION_NORMAL_START;
      part->mode = POK_PARTITION_MODE_INIT_COLD;

#ifdef POK_NEEDS_INSTRUMENTATION
      pok_instrumentation_partition_archi (i);
#endif

      pok_partition_reinit(i);
   }


#if defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_ERROR_HANDLING)
   // verify number of threads
   if (total_threads != (POK_CONFIG_NB_THREADS - POK_KERNEL_THREADS))
   {
#ifdef POK_NEEDS_DEBUG
      printf ("Error in configuration, bad number of threads\n");
#endif
#ifdef POK_NEEDS_ERROR_HANDLING
      pok_error_raise_kernel(POK_ERROR_KIND_KERNEL_CONFIG);
#endif
   }
#endif

   return POK_ERRNO_OK;
}

static uint64_t get_next_periodic_processing_start(void)
{
    int i;

    uint64_t offset = pok_sched_next_deadline;
    for (i = 0; i < POK_CONFIG_SCHEDULING_NBSLOTS; i++) {
        // check all time slots
        // note that we ignore current activation of _this_ slot
        // e.g. if we're currently in periodic processing window,
        // and it's the only one in schedule, we say that next one
        // will be major frame time units later

        int time_slot_index = (i + 1 + pok_sched_current_slot) % POK_CONFIG_SCHEDULING_NBSLOTS;
        const pok_sched_slot_t *slot = &pok_module_sched[time_slot_index];

        if (slot->type == POK_SLOT_PARTITION) {
            if (slot->partition.id == POK_SCHED_CURRENT_PARTITION && 
                slot->partition.periodic_processing_start) 
            {
                return offset;
            }
        }

        offset += slot->duration;
    }

    assert(FALSE && "Couldn't find next periodic processing window (configurator shouldn't have allowed that)");
}

/**
 * Change the current mode of the partition. Possible mode
 * are describe in core/partition.h. Returns
 * POK_ERRNO_PARTITION_MODE when requested mode is invalid.
 * Else, returns POK_ERRNO_OK
 */

static pok_ret_t pok_partition_set_normal_mode(pok_partition_id_t pid)
{
    pok_partition_t *part = &pok_partitions[pid];
    if (part->mode == POK_PARTITION_MODE_IDLE) {
        return POK_ERRNO_PARTITION_MODE; // XXX shouldn't happen anyway
    }
    if (part->mode == POK_PARTITION_MODE_NORMAL) {
        return POK_ERRNO_UNAVAILABLE; // TODO revise error codes
    }
    
    part->mode = POK_PARTITION_MODE_NORMAL;
    part->lock_level = 0;

    pok_thread_id_t thread_id;
    for (thread_id = part->thread_index_low; thread_id < part->thread_index; thread_id++) {
        pok_thread_t *thread = &pok_threads[thread_id];

        if (thread->period < 0) {
            // aperiodic process
            if (thread->state == POK_STATE_DELAYED_START) {
                if (thread->wakeup_time <= 0) {
                    thread->state = POK_STATE_RUNNABLE;
                    thread->wakeup_time = POK_GETTICK(); // XXX this smells, though
                } else {
                    thread->state = POK_STATE_WAITING;
                    thread->wakeup_time += POK_GETTICK(); // XXX what does arinc say about this?
                }

                if (thread->time_capacity < 0) {
                    // infinite time capacity
                    thread->end_time = (uint64_t) -1; // XXX find a better way
                } else {
                    thread->end_time = thread->wakeup_time + thread->time_capacity;
                }
            }
        } else {
            // periodic process
            if (thread->state == POK_STATE_DELAYED_START) {
                thread->next_activation = get_next_periodic_processing_start() + thread->wakeup_time;
                
                if (thread->time_capacity < 0) {
                    thread->end_time = (uint64_t) -1; // XXX
                } else {
                    thread->end_time = thread->next_activation + thread->time_capacity;
                }

                thread->state = POK_STATE_WAIT_NEXT_ACTIVATION;
            }
        }
    }
         
    // stop master thread (ARINC permits it)
    pok_thread_stop();

    // note: it doesn't return
    pok_sched();

    return POK_ERRNO_OK;
}

pok_ret_t pok_partition_set_mode(pok_partition_id_t pid, pok_partition_mode_t mode)
{
   // TODO check all the conditions specified in ARINC

   switch (mode)
   {
      case POK_PARTITION_MODE_NORMAL:
        return pok_partition_set_normal_mode(pid);
        break;

      case POK_PARTITION_MODE_IDLE:
         pok_partitions[pid].mode = POK_PARTITION_MODE_IDLE;  /* Here, we change the mode */
         pok_sched ();
         break;

      case POK_PARTITION_MODE_INIT_WARM:
      case POK_PARTITION_MODE_INIT_COLD:
        // XXX the following might be incorrect for SET_PARTITION_MODE
        //     but HM might take this action freely
	if (pok_partitions[pid].mode == POK_PARTITION_MODE_INIT_COLD && mode == POK_PARTITION_MODE_INIT_WARM)
          { 
             return POK_ERRNO_PARTITION_MODE;
          }

         /*
          * The partition fallback in the INIT_WARM mode when it
          * was in the NORMAL mode. So, we check the previous mode
          */

         // TODO support for partition restart is likely broken

         pok_partitions[pid].mode = mode;  /* Here, we change the mode */

         if (pok_partitions[pid].start_condition != POK_START_CONDITION_HM_MODULE_RESTART &&
             pok_partitions[pid].start_condition != POK_START_CONDITION_HM_PARTITION_RESTART)
         {
             pok_partitions[pid].start_condition = POK_START_CONDITION_PARTITION_RESTART;
		 }

         pok_partitions[pid].lock_level = 1; 

         pok_partition_reinit (pid);
         POK_CURRENT_THREAD.force_restart = TRUE;

         pok_sched ();

         break;

      default:
         return POK_ERRNO_EINVAL;
         break;
   }
   return POK_ERRNO_OK;
}

/**
 * Change the mode of the current partition (the partition being executed)
 */
pok_ret_t pok_partition_set_mode_current(pok_partition_mode_t mode)
{
   return (pok_partition_set_mode(POK_SCHED_CURRENT_PARTITION, mode));
}

/**
 * Get partition information. Used for ARINC GET_PARTITION_STATUS function.
 */
pok_ret_t pok_current_partition_get_id(pok_partition_id_t *id)
{
  *id = POK_SCHED_CURRENT_PARTITION;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_period (pok_time_t *period)
{
  *period = POK_CURRENT_PARTITION.period;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_duration (pok_time_t *duration)
{
  *duration = POK_CURRENT_PARTITION.duration;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_operating_mode (pok_partition_mode_t *op_mode)
{
  *op_mode = POK_CURRENT_PARTITION.mode;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_lock_level (int32_t *lock_level)
{
  *lock_level = POK_CURRENT_PARTITION.lock_level;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_start_condition (pok_start_condition_t *start_condition)
{
  *start_condition = POK_CURRENT_PARTITION.start_condition;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_inc_lock_level(int32_t *lock_level)
{
  if (POK_CURRENT_PARTITION.mode != POK_PARTITION_MODE_NORMAL ||
      pok_thread_is_error_handling(&POK_CURRENT_THREAD)) 
  {
    return POK_ERRNO_MODE;
  }
  if (POK_CURRENT_PARTITION.lock_level >= 16) { // XXX
    return POK_ERRNO_EINVAL;
  }
  *lock_level = ++POK_CURRENT_PARTITION.lock_level;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_dec_lock_level(int32_t *lock_level)
{
  if (POK_CURRENT_PARTITION.mode != POK_PARTITION_MODE_NORMAL ||
      pok_thread_is_error_handling(&POK_CURRENT_THREAD)) 
  {
    return POK_ERRNO_MODE;
  }
  if (POK_CURRENT_PARTITION.lock_level == 0) {
    return POK_ERRNO_EINVAL;
  }
  *lock_level = --POK_CURRENT_PARTITION.lock_level;
  if (POK_CURRENT_PARTITION.lock_level == 0) {
    pok_sched();
  }
  return POK_ERRNO_OK;
}

#endif
