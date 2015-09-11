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

#ifndef __POK_SYSCALL_H__
#define __POK_SYSCALL_H__

#include <config.h>

#include <types.h>
#include <errno.h>

typedef enum
{
   POK_SYSCALL_CONSWRITE                           =  10,
   POK_SYSCALL_GETTICK                             =  20,
   POK_SYSCALL_INT_NUMBER                          =  42,
   POK_SYSCALL_THREAD_CREATE                       =  50,
   POK_SYSCALL_THREAD_SLEEP_UNTIL                  =  51,
   POK_SYSCALL_THREAD_SLEEP                        =  52,
   POK_SYSCALL_THREAD_SUSPEND                      =  53,
   //POK_SYSCALL_THREAD_RESTART                      =  54,
   POK_SYSCALL_THREAD_STOP                         =  55,
   POK_SYSCALL_THREAD_PERIOD                       =  56,
   POK_SYSCALL_THREAD_STOPSELF                     =  57,
   POK_SYSCALL_THREAD_ID                           =  58,
   POK_SYSCALL_THREAD_STATUS                       =  59,
   POK_SYSCALL_THREAD_SET_PRIORITY								 =  60,
   POK_SYSCALL_THREAD_RESUME                       =  61,
   POK_SYSCALL_THREAD_SUSPEND_TARGET               =  62,
   //POK_SYSCALL_THREAD_DEADLINE                     =  63,
   //POK_SYSCALL_THREAD_STATE                        =  64,
   POK_SYSCALL_THREAD_DELAYED_START                =  65,
   POK_SYSCALL_THREAD_YIELD                        =  66,
   POK_SYSCALL_THREAD_REPLENISH                    =  67,
#ifdef POK_NEEDS_PORTS_SAMPLING
   POK_SYSCALL_MIDDLEWARE_SAMPLING_ID              = 101,
   POK_SYSCALL_MIDDLEWARE_SAMPLING_READ            = 102,
   POK_SYSCALL_MIDDLEWARE_SAMPLING_STATUS          = 103,
   POK_SYSCALL_MIDDLEWARE_SAMPLING_WRITE           = 104,
   POK_SYSCALL_MIDDLEWARE_SAMPLING_CREATE          = 105,
   POK_SYSCALL_MIDDLEWARE_SAMPLING_CHECK           = 106,
#endif
#ifdef POK_NEEDS_PORTS_QUEUEING
   POK_SYSCALL_MIDDLEWARE_QUEUEING_CREATE          = 110,
   POK_SYSCALL_MIDDLEWARE_QUEUEING_SEND            = 111,
   POK_SYSCALL_MIDDLEWARE_QUEUEING_RECEIVE         = 112,
   POK_SYSCALL_MIDDLEWARE_QUEUEING_ID              = 113,
   POK_SYSCALL_MIDDLEWARE_QUEUEING_STATUS          = 114,
#endif
#ifdef POK_NEEDS_PORTS_VIRTUAL
   POK_SYSCALL_MIDDLEWARE_VIRTUAL_CREATE           = 150,
   POK_SYSCALL_MIDDLEWARE_VIRTUAL_NB_DESTINATIONS  = 151,
   POK_SYSCALL_MIDDLEWARE_VIRTUAL_DESTINATION      = 152,
   POK_SYSCALL_MIDDLEWARE_VIRTUAL_GET_GLOBAL       = 153,
#endif
#if defined (POK_NEEDS_LOCKOBJECTS) || defined (POK_NEEDS_MUTEXES) || defined (POK_NEEDS_SEMAPHORES) || defined (POK_NEEDS_EVENTS) || defined (POK_NEEDS_BUFFERS) || defined (POK_NEEDS_BLACKBOARDS)
   POK_SYSCALL_LOCKOBJ_CREATE                      = 201,
   POK_SYSCALL_LOCKOBJ_OPERATION                   = 202,
   POK_SYSCALL_LOCKOBJ_STATUS                      = 203,
#endif
#ifdef POK_NEEDS_ERROR_HANDLING
   POK_SYSCALL_ERROR_HANDLER_CREATE                = 301,
   POK_SYSCALL_ERROR_RAISE_APPLICATION_ERROR       = 303,
   POK_SYSCALL_ERROR_GET                           = 304,
   POK_SYSCALL_ERROR_IS_HANDLER                    = 305,
#endif
#ifdef POK_NEEDS_PARTITIONS
   POK_SYSCALL_PARTITION_SET_MODE                  = 404,
   POK_SYSCALL_PARTITION_GET_ID                    = 405,
   POK_SYSCALL_PARTITION_GET_PERIOD                = 406,
   POK_SYSCALL_PARTITION_GET_DURATION              = 407,
   POK_SYSCALL_PARTITION_GET_LOCK_LEVEL            = 408,
   POK_SYSCALL_PARTITION_GET_OPERATING_MODE        = 409,
   POK_SYSCALL_PARTITION_GET_START_CONDITION       = 410,
   POK_SYSCALL_PARTITION_INC_LOCK_LEVEL            = 411,
   POK_SYSCALL_PARTITION_DEC_LOCK_LEVEL            = 412,
#endif
#ifdef POK_NEEDS_IO
   POK_SYSCALL_INB                                 = 501,
   POK_SYSCALL_OUTB                                = 502,
#endif
#ifdef POK_NEEDS_PCI
   POK_SYSCALL_PCI_REGISTER                        = 601,
#endif

   POK_SYSCALL_MEM_VIRT_TO_PHYS                    = 701,
   POK_SYSCALL_MEM_PHYS_TO_VIRT                    = 702,
} pok_syscall_id_t;

typedef struct
{
   uint32_t             nargs;
   uint32_t             arg1;
   uint32_t             arg2;
   uint32_t             arg3;
   uint32_t             arg4;
   uint32_t             arg5;
} pok_syscall_args_t;

typedef struct
{
   pok_partition_id_t   partition;
   uint32_t             thread;
   uint32_t             base_addr;
}pok_syscall_info_t;



/**
 *  Function that performs the syscall. It is called by the 
 *  architecture interruption handler.
 *
 *  @param syscall_id
 *  This param correspond to the syscal which should be performed.
 *  The list of available syscalls is available in
 *  the definition of the pok_syscall_id_t type
 *
 *  @param args
 *  Arguments of the syscall. It corresponds to data useful
 *  to perform the syscall.
 *
 *  @param infos
 *  Informations about the syscall: which partition/thread
 *  initiates the syscall, etc ...
 *
 *  @return
 *  Returns an error code, which is defined in include/errno.h
 */

pok_ret_t	pok_core_syscall (const pok_syscall_id_t     syscall_id,
                              const pok_syscall_args_t*  args,
                              const pok_syscall_info_t*  infos);

/*
 * Initiate syscalls.
 * This part is defined in low-level layers, each architecture/bsp
 * initiate the syscall in a different way.
 */
pok_ret_t pok_syscall_init();

#define POK_CHECK_PTR_OR_RETURN(pid,ptr)	\
   if (!POK_CHECK_PTR_IN_PARTITION(pid,ptr))	\
{						\
   return POK_ERRNO_EINVAL;					\
}

#endif /* __POK_SYSCALL_H__ */
