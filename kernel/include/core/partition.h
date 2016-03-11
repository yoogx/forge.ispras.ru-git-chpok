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

#include <config.h>

#include <types.h>
#include <errno.h>
#include <core/error.h>
#include <arch.h>


typedef enum
{
  POK_START_CONDITION_NORMAL_START          = 0,
  POK_START_CONDITION_PARTITION_RESTART     = 1,
  POK_START_CONDITION_HM_MODULE_RESTART     = 2,
  POK_START_CONDITION_HM_PARTITION_RESTART  = 3
}pok_start_condition_t;


/* Operations specific for given partition. */
struct pok_partition_operations
{
    /* 
     * Called when partition is (re)started.
     * 
     * Local preemption is disabled.
     */
    void (*start)(void);  /**< The entry-point for the partition's thread. */

    /*
     * Called when time counter has been changed within current time slot.
     * 
     * Local preemption is disabled.
     * 
     * If local preemption is already disabled, corresponded state
     * byte is set instead of calling this function.
     */
    void (*on_time_changed)(void);
    
    /*
     * Called when partition acquire CPU because time slot have been
     * changed from other partition to given one..
     * 
     * Local preemption is disabled.
     * 
     * If local preemption is already disabled, corresponded state
     * byte is set instead of calling this function.
     */
    void (*on_control_returned)(void);
    
    /* 
     * Process sync error related to given partition.
     * 
     * Local preemption is disabled.
     * 
     * Previous local preemption state is passed as parameter.
     */
    void (*process_partition_error)(pok_system_state_t partition_state,
        pok_error_id_t error_id,
        uint8_t state_byte_preempt_local,
        void* failed_address);
};

/*!
 * \struct pok_partition_t
 * \brief This structure contains all needed information for partition management
 */
typedef struct
{
#ifdef POK_NEEDS_MONITOR
    pok_bool_t               is_paused;      /*Partition paused or not*/
#endif

    const struct pok_partition_operations* part_ops;
    
    /* 
     * State of the partition.
     * 
     * This is like "state register" notion used for architectures.
     */
    union {
        /* 
         * State flags (bytes). Can be 0 or 1.
         * 
         * Because flags are bytes (not bits), they can be set atomically
         * without locked operations.
         */
        struct {
            /*
             * Flag is set by global scheduler, when time is changed but
             * local preemption was disabled at that moment.
             */
            uint8_t time_changed;
            /*
             * Flag is set by global scheduler, after time slot is changed
             * from other partition to given one.
             */
            uint8_t control_returned;

            uint8_t unused1;
            uint8_t unused2;
        } bytes;
        /* 
         * All flags at once.
         * 
         * This value may be checked by partition after enabling preemption
         * for ensure, that it hasn't miss other flags.
         */
        uint32_t bytes_all;
    } state;

    /*
     * Whether local preemption is disabled.
     * 
     * Flag can be read, modified or cleared by the partition itself.
     * 
     * Flag is set and checked by the global scheduler,
     * when it need to call partition's callbacks.
     */
    uint8_t preempt_local_disabled;

    
    /* 
     * Whether currently executed *user space* process is error handler.
     * 
     * This field is used for determine system level in case when
     * error is catched via interrupt.
     * 
     * After partition initialization, this field is FALSE.
     */
    pok_bool_t               is_error_handler;

    const char               *name;          /**< Name of the partition */

    uint8_t                  priority;       /**< Priority of the partition (unused at this time */
    uint32_t                 period;         /**< Period of the partition, unused at this time */
    uint32_t                 duration;       /**< Duration of the partition */

    /*
     * Kernel stack address which is used for enter into the partition.
     * 
     * 0 value in this field means that partition needs to be (re)started.
     *
     */
    uint32_t	        	 sp; 

    /*
     * Initial value of kernel stack (when it was allocated).
     *
     * Used for restarting partition.
     */
    struct dStack            initial_sp;
    
    /* 
     * Identificator of (user) space, corresponded to given partition.
     * Special value 0xff means that no user space is used by this partition.
     * 
     * Set in deployment.c
     * 
     * Used by scheduler when it switch into partition.
     */
    uint8_t                  space_id;


#ifdef POK_NEEDS_IO
  uint16_t		    io_min;             /**< If the partition is allowed to perform I/O, the lower bound of the I/O */
  uint16_t		    io_max;             /**< If the partition is allowed to perform I/O, the uppder bound of the I/O */
#endif

  pok_start_condition_t	    start_condition;
  
  /* 
   * Pointer to Multi partition HM selector.
   * 
   * Bit's value 0 means module level error, 1 - partition level error.
   */
  const pok_error_level_selector_t* multi_partition_hm_selector;
  /*
   *  Pointer to Multi partition HM table.
   */
  const pok_error_module_action_table_t* multi_partition_hm_table;
} pok_partition_t;

/**
 * Pointer to the current partition.
 * 
 * DEV: Readonly for all except scheduler-related stuff.
 */
extern pok_partition_t* current_partition;

/*
 * Raise error from partition.
 * 
 * partition_state should be above POK_SYSTEM_STATE_MAX_MODULE.
 * 
 * Module HM table won't be used when process given error, but
 * Multi-partition HM table will.
 */
void raise_partition_error(pok_system_state_t partition_state,
    pok_error_id_t error_id, void* failed_address);

/* See description of raise_error_fatal about meaning of `_fatal` suffix. */
void raise_partition_error_fatal(pok_system_state_t partition_state,
    pok_error_id_t error_id, void* failed_address);


/**
 * Reset partition state, so scheduler will be able to start it.
 * 
 * Should be called with preemption disabled.
 * 
 * Fields, which should be set(initialized) before given function:
 * 
 *   - @initial_sp
 *   - @sched
 *   - @start
 *   - @name
 * 
 * Fields, which are set by the function itself:
 * 
 *   - @sp = 0
 * 
 * Other fields remains unchanged.
 * 
 * Note, that fields below will be set by the scheduler before @start
 * function will see them:
 *   - @is_slot_started = TRUE
 *   - @sched_local_recheck_needed = TRUE
 */
void pok_partition_reset(pok_partition_t* part);

#endif /* __POK_PARTITION_H__ */
