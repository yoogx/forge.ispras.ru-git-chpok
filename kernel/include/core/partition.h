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
#include <core/thread.h>
#include <core/sched.h>


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
#ifdef POK_NEEDS_MONITOR
    pok_bool_t               is_paused;      /*Partition paused or not*/
#endif

    void (sched*)(void);     /**< Local scheduler for the partition */

    const char               *name;          /**< Name of the partition */

    uint8_t                  priority;       /**< Priority of the partition (unused at this time */
    uint32_t                 period;         /**< Period of the partition, unused at this time */
    uint32_t                 duration;       /**< Duration of the partition */

    void (start*)(void);  /**< The entry-point to the partition's thread. */

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


#ifdef POK_NEEDS_IO
  uint16_t		    io_min;             /**< If the partition is allowed to perform I/O, the lower bound of the I/O */
  uint16_t		    io_max;             /**< If the partition is allowed to perform I/O, the uppder bound of the I/O */
#endif

  pok_start_condition_t	    start_condition;
  
  pok_bool_t is_slot_started;
  pok_bool_t sched_local_recheck_needed;
} pok_partition_t;

/**
 * Pointer to the current partition.
 * 
 * DEV: Readonly for all except scheduler-related stuff.
 */
extern pok_partition_t* current_partition;

/**
 * Current partition structure.
 * 
 * For convenience.
 */
#define POK_CURRENT_PARTITION (*currentPartition)


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
