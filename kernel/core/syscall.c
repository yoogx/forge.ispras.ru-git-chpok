/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2013-2014, 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

#include <config.h>

#include <asp/arch.h>
#include <asp/bsp_common.h>
#include <types.h>
#include <libc.h>
#include <ioports.h>

#include <errno.h>
#include <core/debug.h>
#include <core/syscall.h>
#include <core/partition_arinc.h>
#include <core/thread.h>
#include <core/time.h>
#include <core/error.h>
#include <asp/memory.h>

#include <cons.h>
#include <core/port.h>

/* Call given function without protection(with enabled interrupts). */
static pok_ret_t unprotected_syscall(
   pok_ret_t (*f)(const pok_syscall_args_t* args),
   const pok_syscall_args_t*    args)
{
   pok_ret_t ret;
   ja_preempt_enable();
   ret = f(args);
   pok_partition_return_user();
   return ret;
}

#define SYSCALL_ENTRY(id) case id: return unprotected_syscall(pok_syscall_wrapper_ ## id, args); break;

/**
 * \file kernel/core/syscalls.c
 * \brief This file implement generic system calls
 * \author Julien Delange
 */

static inline pok_ret_t pok_core_syscall_internal (const pok_syscall_id_t       syscall_id,
                            const pok_syscall_args_t*    args,
                            const pok_syscall_info_t*    infos)
{
   switch (syscall_id)
   {
#if defined (POK_NEEDS_CONSOLE) || defined (POK_NEEDS_DEBUG)
      case POK_SYSCALL_CONSWRITE:
         return jet_console_write_user((const char* __user)args->arg1, args->arg2);
         break;
#endif

#if defined POK_NEEDS_GETTICK
      case POK_SYSCALL_CLOCK_GETTIME:
         return pok_clock_gettime ((clockid_t)args->arg1, (pok_time_t* __user)args->arg2);
         break;
#endif
      case POK_SYSCALL_TIME:
         return jet_time((time_t*)args->arg1);

      SYSCALL_ENTRY(POK_SYSCALL_THREAD_CREATE)

#ifdef POK_NEEDS_THREAD_SLEEP
      SYSCALL_ENTRY(POK_SYSCALL_THREAD_SLEEP)
#endif

#ifdef POK_NEEDS_THREAD_SLEEP_UNTIL
      SYSCALL_ENTRY(POK_SYSCALL_THREAD_SLEEP_UNTIL)
#endif

      SYSCALL_ENTRY(POK_SYSCALL_THREAD_PERIOD)

#if defined (POK_NEEDS_THREAD_SUSPEND) || defined (POK_NEEDS_ERROR_HANDLING)
      SYSCALL_ENTRY(POK_SYSCALL_THREAD_SUSPEND)
#endif

   SYSCALL_ENTRY(POK_SYSCALL_THREAD_STATUS)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_DELAYED_START)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_SET_PRIORITY)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_RESUME)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_SUSPEND_TARGET)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_YIELD)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_REPLENISH)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_STOP)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_STOPSELF)

   SYSCALL_ENTRY(POK_SYSCALL_THREAD_FIND)

   SYSCALL_ENTRY(POK_SYSCALL_RESCHED)
   SYSCALL_ENTRY(POK_SYSCALL_MSECTION_ENTER_HELPER)
   SYSCALL_ENTRY(POK_SYSCALL_MSECTION_WAIT)
   SYSCALL_ENTRY(POK_SYSCALL_MSECTION_NOTIFY)
   SYSCALL_ENTRY(POK_SYSCALL_MSECTION_WQ_NOTIFY)
   SYSCALL_ENTRY(POK_SYSCALL_MSECTION_WQ_SIZE)

#ifdef POK_NEEDS_PARTITIONS
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_SET_MODE)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_GET_STATUS)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_INC_LOCK_LEVEL)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_DEC_LOCK_LEVEL)
#endif

#ifdef POK_NEEDS_ERROR_HANDLING
   SYSCALL_ENTRY(POK_SYSCALL_ERROR_HANDLER_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_ERROR_RAISE_APPLICATION_ERROR)
   SYSCALL_ENTRY(POK_SYSCALL_ERROR_GET)
#endif

   SYSCALL_ENTRY(POK_SYSCALL_ERROR_RAISE_OS_ERROR)

   /* Middleware syscalls */
#ifdef POK_NEEDS_PORTS_SAMPLING
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_WRITE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_READ)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_ID)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_STATUS)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_CHECK)
#endif /* POK_NEEDS_PORTS_SAMPLING */


#ifdef POK_NEEDS_PORTS_QUEUEING
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_SEND)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_RECEIVE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_ID)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_STATUS)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_CLEAR)
#endif /* POK_NEEDS_PORTS_QUEUEING */

#ifdef POK_NEEDS_IO
      case POK_SYSCALL_INB:
         if ((args->arg1 < pok_partitions[infos->partition].io_min) ||
             (args->arg1 > pok_partitions[infos->partition].io_max))
         {
            return -POK_ERRNO_EPERM;
         }
         else
         {
            return inb((unsigned short) args->arg1);
         }
         break;

      case POK_SYSCALL_OUTB:
         if ((args->arg1 < pok_partitions[infos->partition].io_min) ||
             (args->arg1 > pok_partitions[infos->partition].io_max))
         {
            return -POK_ERRNO_EPERM;
         }
         else
         {
            outb((unsigned short) args->arg1, (unsigned char) args->arg2);
            return POK_ERRNO_OK;
         }
       break;
#endif /* POK_NEEDS_IO */

      //TODO rewrite this! This two syscall needs to return pok_ret_t!
      case POK_SYSCALL_MEM_VIRT_TO_PHYS:
         return pok_virt_to_phys(args->arg1);
         break;
      case POK_SYSCALL_MEM_PHYS_TO_VIRT:
         return pok_phys_to_virt(args->arg1);
         break;

      case POK_SYSCALL_GET_BSP_INFO:
         return pok_bsp_get_info((void* __user)args->arg1);
         break;

      SYSCALL_ENTRY(POK_SYSCALL_MEMORY_BLOCK_GET_STATUS)

      default:
       /*
        * Unrecognized system call ID.
        */
#ifdef POK_NEEDS_DEBUG
            printf ("Tried to use syscall %d\n", syscall_id);
#endif
         pok_raise_error(POK_ERROR_ID_ILLEGAL_REQUEST, TRUE, NULL);
         break;
   }

   return POK_ERRNO_EINVAL; // TODO: Unreachable?
}

pok_ret_t pok_core_syscall (const pok_syscall_id_t       syscall_id,
                            const pok_syscall_args_t*    args,
                            const pok_syscall_info_t*    infos)
{
    pok_ret_t ret;
#ifdef POK_NEEDS_GDB
    current_partition->entry_sp_user = global_thread_stack;
    pok_in_user_space = FALSE;
#endif

    ret = pok_core_syscall_internal(syscall_id, args, infos);

#if POK_NEEDS_GDB
    pok_in_user_space = TRUE;
#endif

    return ret;
}
