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
 * \file    partition.h
 * \brief   Definition of structure for partitioning services.
 * \author  Julien Delange
 */

#ifndef __POK_PARTITION_H__
#define __POK_PARTITION_H__

#ifdef POK_NEEDS_PARTITIONS

#include <types.h>
#include <errno.h>
#include <core/error.h>
#include <core/thread.h>
#include <core/sched.h>


/**
 * \enum pok_partition_mode_t
 * \brief The different modes of a partition
 */
typedef enum
{ 
   /*
    * In init mode, only main thread (process) is run.
    * This's the only mode where one can create various resources.
    *
    * There's really no difference between cold and warm init.
    *   
    * When partition is initially started, it's in cold init.
    *
    * HM table and set_partition_mode function may restart 
    * partition into either cold or warm init mode.
    *
    * The exact type of init can be introspected by an application,
    * but otherwise, it makes no difference.
    */
   POK_PARTITION_MODE_INIT_COLD = 1, 
   POK_PARTITION_MODE_INIT_WARM = 2,

   /*
    * In normal mode, all threads except main can be run.
    *
    * No resources can be allocated.
    */
   POK_PARTITION_MODE_NORMAL    = 3, 

   /*
    * Partition is stopped.
    */
   POK_PARTITION_MODE_IDLE      = 4,
}pok_partition_mode_t;

typedef enum
{
  POK_START_CONDITION_NORMAL_START          = 0,
  POK_START_CONDITION_PARTITION_RESTART     = 1,
  POK_START_CONDITION_HM_MODULE_RESTART     = 2,
  POK_START_CONDITION_HM_PARTITION_RESTART  = 3
}pok_start_condition_t;


/*!
 * \struct pok_partition_t
 * \brief This structure contains all needed information for partition management
 */
typedef struct
{
   uint32_t                 base_addr;    /**< The base address inside the whole memory (where the segment is in the whole memory ?) */
   uint32_t                 base_vaddr;   /**< The virtual address of the partition. The address the threads sees when they are
                                        *    executed
                                       */

   uint32_t                 size;           /**< Size of the allocated memory segment */

   const char               *name;          /**< Name of the partition */

   uint32_t                 nthreads;       /**< Number of threads inside the partition */

   uint8_t                  priority;       /**< Priority of the partition (unused at this time */
   uint32_t                 period;         /**< Period of the partition, unused at this time */

   const pok_scheduler_ops  *scheduler;     /**< The scheduler of this partition */

   uint64_t                 activation;     /**< Last activation time of the partition */
   uint32_t                 prev_thread;    /**< member for the scheduler (previous scheduled real thread inside the partition,i.e not the idle thread */
   uint32_t                 current_thread; /**< member for the scheduler (current executed thread inside the partition */

   uint32_t                 thread_index_low;    /**< The low index in the threads table */
   uint32_t                 thread_index_high;   /**< The high index in the threads table */
   uint32_t                 thread_index;        /**< The thread index */

#if defined(POK_NEEDS_LOCKOBJECTS) || defined(POK_NEEDS_ERROR_HANDLING)
   uint8_t                  lockobj_index_low;   /**< The low bound in the lockobject array. */
   uint8_t                  lockobj_index_high;  /**< The high bound in the lockobject array */
   uint8_t                  nlockobjs;           /**< The amount of lockobjects reserved for the partition */
#endif

#ifdef POK_NEEDS_ERROR_HANDLING
   pok_bool_t               thread_error_created; /**< If true, this partition has error handler created */
   pok_thread_id_t          thread_error;       /**< The thread identifier used for error handling */
   pok_error_status_t       error_status;       /**< A pointer used to store information about errors */
#endif
   pok_thread_id_t          thread_main;        /**< The thread identifier of the main thread (initialization thread) */
   uintptr_t                thread_main_entry;  /**< The entry-point of the main thread (useful for re-init) */
   pok_partition_mode_t     mode;               /**< Current mode of the partition */

#ifdef POK_NEEDS_IO
  uint16_t		    io_min;             /**< If the partition is allowed to perform I/O, the lower bound of the I/O */
  uint16_t		    io_max;             /**< If the partition is allowed to perform I/O, the uppder bound of the I/O */
#endif

  uint32_t		    lock_level;
  pok_start_condition_t	    start_condition;
} pok_partition_t;

extern pok_partition_t pok_partitions[POK_CONFIG_NB_PARTITIONS];

/**
 * Access to the current partition variable.
 * With that, you can do POK_CURRENT_PARTITION.nthreads of POK_CURRENT_PARTITION.mode
 * It avoids tedious syntax like pok_partitions[my_partition].blablabla
 */
#define POK_CURRENT_PARTITION pok_partitions[POK_SCHED_CURRENT_PARTITION]

/**
 * Chech that pointer \a ptr is located in the address space of partition
 * \a pid
 */

/* TODO dirty as hell */

#ifdef __i386__
#define POK_CHECK_PTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= pok_partitions[pid].base_addr && \
                                             ((uintptr_t)(ptr)) <  pok_partitions[pid].base_addr + pok_partitions[pid].size\
                                             )

#define POK_CHECK_VPTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= pok_partitions[pid].base_vaddr && \
                                             ((uintptr_t)(ptr)) <  pok_partitions[pid].base_vaddr + pok_partitions[pid].size\
                                             )
#elif defined(__PPC__)
#define POK_CHECK_PTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= 0x80000000 && \
                                             ((uintptr_t)(ptr)) <  0x80000000 + 0x1000000ULL\
                                             )

#define POK_CHECK_VPTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= 0x80000000 && \
                                             ((uintptr_t)(ptr)) <  0x80000000 + 0x1000000ULL\
                                             )
#else
#error "POK_CHECK_PTR macros are not implemented for this arch, do it now!"
#endif

/**
 * Initialize all partitions
 */
pok_ret_t pok_partition_init();

pok_ret_t pok_partition_set_mode (pok_partition_id_t pid, const pok_partition_mode_t mode);
pok_ret_t pok_partition_set_mode_current (const pok_partition_mode_t mode);


void pok_partition_reinit (pok_partition_id_t);

void pok_partition_setup_main_thread (pok_partition_id_t);

void pok_partition_setup_scheduler (pok_partition_id_t pid);

pok_ret_t pok_partition_restart_thread (pok_thread_id_t tid);

pok_ret_t pok_current_partition_get_id (pok_partition_id_t *id);

pok_ret_t pok_current_partition_get_period (uint64_t *period);

pok_ret_t pok_current_partition_get_duration (uint64_t *duration);

pok_ret_t pok_current_partition_get_operating_mode (pok_partition_mode_t *op_mode);

pok_ret_t pok_current_partition_get_lock_level (uint32_t *lock_level);

pok_ret_t pok_current_partition_get_start_condition (pok_start_condition_t *start_condition);

pok_ret_t pok_current_partition_inc_lock_level(uint32_t *lock_level);

pok_ret_t pok_current_partition_dec_lock_level(uint32_t *lock_level);

// utility macro-like functions

#ifdef POK_NEEDS_ERROR_HANDLING
static inline 
pok_bool_t pok_thread_is_error_handling(const pok_thread_t *thread)
{
    return pok_partitions[thread->partition].thread_error_created && 
           pok_partitions[thread->partition].thread_error == pok_thread_get_id(thread);
}
#else
static inline
pok_bool_t pok_thread_is_error_handling(const pok_thread_t *thread)
{
    return FALSE;
}
#endif

static inline pok_bool_t
pok_thread_is_valid_and_created(const pok_thread_t *thread, const pok_partition_t *part)
{
    pok_thread_id_t id = pok_thread_get_id(thread);
    return id >= part->thread_index_low && id < part->thread_index;
}

#endif /* __POK_NEEDS_PARTITIONS */

#endif /* __POK_PARTITION_H__ */
