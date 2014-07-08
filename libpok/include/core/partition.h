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

#include <core/dependencies.h>

#ifdef POK_NEEDS_PARTITIONS

#include <types.h>
#include <errno.h>
#include <core/syscall.h>

typedef enum
{
   POK_PARTITION_MODE_INIT_COLD = 1,
   POK_PARTITION_MODE_INIT_WARM = 2,
   POK_PARTITION_MODE_NORMAL    = 3,
   POK_PARTITION_MODE_IDLE      = 4,
   POK_PARTITION_MODE_RESTART   = 5,
   POK_PARTITION_MODE_STOPPED   = 6,
}pok_partition_mode_t;

typedef enum
{
  POK_START_CONDITION_NORMAL_START          = 0,
  POK_START_CONDITION_PARTITION_RESTART     = 1,
  POK_START_CONDITION_HM_MODULE_RESTART     = 2,
  POK_START_CONDITION_HM_PARTITION_RESTART  = 3
}pok_start_condition_t;

static inline 
pok_ret_t pok_partition_set_mode(pok_partition_mode_t mode) 
{
    return pok_syscall2(POK_SYSCALL_PARTITION_SET_MODE, (uint32_t)mode, 0);
}

static inline
pok_ret_t pok_current_partition_get_id(pok_partition_id_t *idptr)
{
    return pok_syscall2(POK_SYSCALL_PARTITION_GET_ID, (uint32_t)idptr, 0);
}

static inline
pok_ret_t pok_current_partition_get_period(uint64_t *period) 
{
    return pok_syscall2(POK_SYSCALL_PARTITION_GET_PERIOD, (uint32_t)period, 0);
}

static inline
pok_ret_t pok_current_partition_get_duration(uint64_t *duration) 
{
    return pok_syscall2(POK_SYSCALL_PARTITION_GET_DURATION, (uint32_t)duration, 0);
}

static inline
pok_ret_t pok_current_partition_get_lock_level(uint32_t *lock_level) 
{
    return pok_syscall2(POK_SYSCALL_PARTITION_GET_LOCK_LEVEL, (uint32_t)lock_level, 0);
}

static inline
pok_ret_t pok_current_partition_get_operating_mode(pok_partition_mode_t *op_mode)
{
    return pok_syscall2(POK_SYSCALL_PARTITION_GET_OPERATING_MODE, (uint32_t)op_mode, 0);
}

static inline
pok_ret_t pok_current_partition_get_start_condition(pok_start_condition_t *start_condition)
{
    return pok_syscall2(POK_SYSCALL_PARTITION_GET_START_CONDITION, (uint32_t)start_condition, 0);
}

#endif
