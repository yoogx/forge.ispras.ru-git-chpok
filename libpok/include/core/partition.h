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
 * Created by julien on Mon Jan 19 14:18:51 2009 
 */

#ifndef __POK_PARTITION_H__
#define __POK_PARTITION_H__

#include <config.h>

#include <core/dependencies.h>

#ifdef POK_NEEDS_PARTITIONS

#include <types.h>
#include <errno.h>

typedef enum
{
   POK_PARTITION_MODE_INIT_COLD = 1,
   POK_PARTITION_MODE_INIT_WARM = 2,
   POK_PARTITION_MODE_NORMAL    = 3,
   POK_PARTITION_MODE_IDLE      = 4,
}pok_partition_mode_t;

typedef enum
{
  POK_START_CONDITION_NORMAL_START          = 0,
  POK_START_CONDITION_PARTITION_RESTART     = 1,
  POK_START_CONDITION_HM_MODULE_RESTART     = 2,
  POK_START_CONDITION_HM_PARTITION_RESTART  = 3
}pok_start_condition_t;

#include <core/syscall.h>

// Rename syscall
#define pok_partition_set_mode pok_partition_set_mode_current
#define pok_partition_inc_lock_level pok_current_partition_inc_lock_level
#define pok_partition_dec_lock_level pok_current_partition_dec_lock_level

// Wrapper around corresponded syscall. Returns whether preemption is disabled.
#define pok_current_partition_preemption_disabled() \
    ({int32_t lock_level; pok_current_partition_get_lock_level(&lock_level); lock_level > 0;})


#endif

#endif /* __POK_PARTITION_H__ */
