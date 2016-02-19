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
 * POK_NEEDS_PARTITIONS macro.
 */

#include <config.h>

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

void pok_partition_reset(pok_partition_t* part)
{
    part->sp = 0;
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
