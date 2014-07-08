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
#include <core/loader.h>
#include <core/partition.h>
#include <core/instrumentation.h>
#include <core/time.h>

#include <libc.h>

/**
 * \brief The array that contains ALL partitions in the system.
 */
pok_partition_t pok_partitions[POK_CONFIG_NB_PARTITIONS];


uint8_t			 pok_partitions_index = 0;

extern uint64_t		pok_sched_slots[];


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
 * \brief Reinitialize a partition from scratch
 *
 * This service is only used when we have to retrieve
 * and handle errors.
 */

#ifdef POK_NEEDS_ERROR_HANDLING
void pok_partition_reinit (pok_partition_id_t pid)
{
   uint32_t tmp;
   /*
    * FIXME: reset queueing/sampling ports too
    */
   pok_partition_setup_scheduler (pid);

   pok_partitions[pid].thread_index = 0;
   pok_partitions[pid].current_thread = pok_partitions[pid].thread_index_low;
   pok_partitions[pid].prev_thread =  IDLE_THREAD; // breaks the rule of prev_thread not being idle, but it's just for init

#ifdef POK_NEEDS_ERROR_HANDLING
   pok_partitions[pid].thread_error = 0;
   pok_partitions[pid].error_status.failed_thread = 0;
   pok_partitions[pid].error_status.failed_addr   = 0;
   pok_partitions[pid].error_status.error_kind    = POK_ERROR_KIND_INVALID;
   pok_partitions[pid].error_status.msg_size      = 0;
#endif

   pok_loader_load_partition (pid, pok_partitions[pid].base_addr - pok_partitions[pid].base_vaddr, &tmp);

   pok_partitions[pid].thread_main_entry = tmp;

   pok_partition_setup_main_thread (pid);
}
#endif

/**
 * Setup the main thread of partition with number \a pid
 */
void pok_partition_setup_main_thread (const uint8_t pid)
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

   const uint32_t	partition_size[POK_CONFIG_NB_PARTITIONS] = POK_CONFIG_PARTITIONS_SIZE;
#ifdef POK_CONFIG_PARTITIONS_LOADADDR
   const uint32_t	program_loadaddr[POK_CONFIG_NB_PARTITIONS]
      = POK_CONFIG_PROGRAM_LOADADDR;
#endif
#ifdef POK_NEEDS_LOCKOBJECTS
   uint8_t lockobj_index = 0;
#endif

   for (i = 0 ; i < POK_CONFIG_NB_PARTITIONS ; i++)
   {
      uint32_t size = partition_size[i];
#ifndef POK_CONFIG_PARTITIONS_LOADADDR
      uint32_t base_addr = (uint32_t)pok_bsp_mem_alloc(partition_size[i]);
#else
      uint32_t base_addr = program_loadaddr[i];
#endif
      uint32_t program_entry;
      uint32_t base_vaddr = pok_space_base_vaddr(base_addr);

      pok_partitions[i].base_addr   = base_addr;
      pok_partitions[i].size        = size;
     
#ifdef POK_NEEDS_COVERAGE_INFOS
#include <libc.h>
      printf ("[XCOV] Partition %d loaded at addr virt=|%x|, phys=|%x|\n", i, base_vaddr, base_addr);
#endif

      pok_partition_setup_scheduler (i);

      pok_create_space (i, base_addr, size);

      pok_partitions[i].base_vaddr = base_vaddr;
      /* Set the memory space and so on */
      
      pok_partitions[i].thread_index_low  = threads_index;
      pok_partitions[i].nthreads          = ((uint32_t[]) POK_CONFIG_PARTITIONS_NTHREADS) [i];

      total_threads += pok_partitions[i].nthreads;

#ifdef POK_NEEDS_ERROR_HANDLING
      if (pok_partitions[i].nthreads <= 1)
      {
         pok_partition_error (i, POK_ERROR_KIND_PARTITION_CONFIGURATION);
      }
#endif

      pok_partitions[i].thread_index_high = pok_partitions[i].thread_index_low + ((uint32_t[]) POK_CONFIG_PARTITIONS_NTHREADS) [i];
      pok_partitions[i].activation        = 0;
      pok_partitions[i].period            = POK_CONFIG_SCHEDULING_MAJOR_FRAME;
      pok_partitions[i].thread_index      = pok_partitions[i].thread_index_low;
      pok_partitions[i].thread_main       = 0;
      pok_partitions[i].current_thread    = IDLE_THREAD;
      pok_partitions[i].prev_thread       = IDLE_THREAD; // breaks the rule of prev_thread not being idle, but it's just for init

      threads_index                       = threads_index + pok_partitions[i].nthreads;
      /* Initialize the threading stuff */

      pok_partitions[i].mode              = POK_PARTITION_MODE_INIT_COLD;

#ifdef POK_NEEDS_LOCKOBJECTS
      pok_partitions[i].lockobj_index_low    = lockobj_index;
      pok_partitions[i].lockobj_index_high   = lockobj_index + ((uint8_t[]) POK_CONFIG_PARTITIONS_NLOCKOBJECTS[i]);
      pok_partitions[i].nlockobjs            = ((uint8_t[]) POK_CONFIG_PARTITIONS_NLOCKOBJECTS[i]);
      lockobj_index                          = lockobj_index + pok_partitions[i].nlockobjs;
      /* Initialize mutexes stuff */
#endif

#ifdef POK_NEEDS_ERROR_HANDLING
      pok_partitions[i].thread_error_created      = FALSE;
#endif

      pok_loader_load_partition (i, base_addr - base_vaddr, &program_entry);
      /*
       * Load the partition in its address space
       */
      pok_partitions[i].thread_main_entry = program_entry;
      
      pok_partitions[i].lock_level = 1; // in init mode, lock level is always >0
      pok_partitions[i].start_condition = POK_START_CONDITION_NORMAL_START;

#ifdef POK_NEEDS_INSTRUMENTATION
      pok_instrumentation_partition_archi (i);
#endif

      pok_partition_setup_main_thread (i);
      pok_partitions[i].current_thread    = pok_partitions[i].thread_main;
   }


#if defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_ERROR_HANDLING)
   // verify number of threads
   if (total_threads != (POK_CONFIG_NB_THREADS - 2))
   {
#ifdef POK_NEEDS_DEBUG
      printf ("Error in configuration, bad number of threads\n");
#endif
#ifdef POK_NEEDS_ERROR_HANDLING
      pok_kernel_error (POK_ERROR_KIND_KERNEL_CONFIG);
#endif
   }
#endif

   return POK_ERRNO_OK;
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
                uint64_t next_periodic_processing = POK_CONFIG_SCHEDULING_MAJOR_FRAME + POK_CURRENT_PARTITION.activation;

                thread->next_activation = next_periodic_processing + thread->wakeup_time;
                
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
      case POK_PARTITION_MODE_STOPPED:
         pok_partitions[pid].mode = POK_PARTITION_MODE_STOPPED;  /* Here, we change the mode */
         pok_sched ();
         break;

      case POK_PARTITION_MODE_INIT_WARM:
      case POK_PARTITION_MODE_INIT_COLD:
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
         pok_partitions[pid].lock_level = 1; 

         pok_partition_reinit (pid);

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

pok_ret_t pok_current_partition_get_period (uint64_t *period)
{
  *period = POK_CURRENT_PARTITION.period;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_duration (uint64_t *duration)
{
  *duration = pok_sched_slots[POK_SCHED_CURRENT_PARTITION];
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_operating_mode (pok_partition_mode_t *op_mode)
{
  *op_mode = POK_CURRENT_PARTITION.mode;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_lock_level (uint32_t *lock_level)
{
  *lock_level = POK_CURRENT_PARTITION.lock_level;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_start_condition (pok_start_condition_t *start_condition)
{
  *start_condition = POK_CURRENT_PARTITION.start_condition;
  return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_inc_lock_level(uint32_t *lock_level)
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

pok_ret_t pok_current_partition_dec_lock_level(uint32_t *lock_level)
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
