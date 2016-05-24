/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
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

#ifndef __LIBPOK_SYSCALL_H__
#define __LIBPOK_SYSCALL_H__

#include <config.h>

#include <core/dependencies.h>

#include <types.h>
#include <errno.h>

typedef enum
{
	 POK_SYSCALL_CONSWRITE                           =  10,
	 POK_SYSCALL_GETTICK                             =  20,
	 POK_SYSCALL_INT_NUMBER                          =  42,
	 POK_SYSCALL_THREAD_CREATE                       =  50,
	 POK_SYSCALL_THREAD_SLEEP_UNTIL                  =  51,
#ifdef POK_NEEDS_THREAD_SLEEP
	 POK_SYSCALL_THREAD_SLEEP                        =  52,
#endif /*POK_NEEDS_THREAD_SLEEP*/
	 POK_SYSCALL_THREAD_SUSPEND                      =  53,
//	 POK_SYSCALL_THREAD_RESTART                      =  54,
	 POK_SYSCALL_THREAD_STOP                         =  55,
	 POK_SYSCALL_THREAD_PERIOD                       =  56,
	 POK_SYSCALL_THREAD_STOPSELF                     =  57,
	 POK_SYSCALL_THREAD_ID                           =  58,
	 POK_SYSCALL_THREAD_STATUS                       =  59,
	 POK_SYSCALL_THREAD_SET_PRIORITY								 =  60,
	 POK_SYSCALL_THREAD_RESUME                       =  61,
	 POK_SYSCALL_THREAD_SUSPEND_TARGET               =  62,
   POK_SYSCALL_THREAD_DEADLINE                     =  63,
   POK_SYSCALL_THREAD_STATE                        =  64,
   POK_SYSCALL_THREAD_DELAYED_START      		   =  65,
   POK_SYSCALL_THREAD_YIELD                        =  66,
   POK_SYSCALL_THREAD_REPLENISH                    =  67,
   POK_SYSCALL_THREAD_FIND                         =  68,

#ifdef POK_NEEDS_BUFFERS
   POK_SYSCALL_INTRA_BUFFER_CREATE                 = 201,
   POK_SYSCALL_INTRA_BUFFER_SEND                   = 202,
   POK_SYSCALL_INTRA_BUFFER_RECEIVE                = 203,
   POK_SYSCALL_INTRA_BUFFER_ID                     = 204,
   POK_SYSCALL_INTRA_BUFFER_STATUS                 = 205,
#endif
#ifdef POK_NEEDS_BLACKBOARDS
   POK_SYSCALL_INTRA_BLACKBOARD_CREATE             = 211,
   POK_SYSCALL_INTRA_BLACKBOARD_READ               = 212,
   POK_SYSCALL_INTRA_BLACKBOARD_DISPLAY            = 213,
   POK_SYSCALL_INTRA_BLACKBOARD_CLEAR              = 214,
   POK_SYSCALL_INTRA_BLACKBOARD_ID                 = 215,
   POK_SYSCALL_INTRA_BLACKBOARD_STATUS             = 216,
#endif
#ifdef POK_NEEDS_SEMAPHORES
   POK_SYSCALL_INTRA_SEMAPHORE_CREATE              = 221,
   POK_SYSCALL_INTRA_SEMAPHORE_WAIT                = 222,
   POK_SYSCALL_INTRA_SEMAPHORE_SIGNAL              = 223,
   POK_SYSCALL_INTRA_SEMAPHORE_ID                  = 224,
   POK_SYSCALL_INTRA_SEMAPHORE_STATUS              = 225,
#endif
#ifdef POK_NEEDS_EVENTS
   POK_SYSCALL_INTRA_EVENT_CREATE                 = 231,
   POK_SYSCALL_INTRA_EVENT_SET                    = 232,
   POK_SYSCALL_INTRA_EVENT_RESET                  = 233,
   POK_SYSCALL_INTRA_EVENT_WAIT                   = 234,
   POK_SYSCALL_INTRA_EVENT_ID                     = 235,
   POK_SYSCALL_INTRA_EVENT_STATUS                 = 236,
#endif

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
#ifdef POK_NEEDS_ERROR_HANDLING
	 POK_SYSCALL_ERROR_HANDLER_CREATE                = 301,
	 POK_SYSCALL_ERROR_RAISE_APPLICATION_ERROR       = 303,
	 POK_SYSCALL_ERROR_GET                           = 304,
         POK_SYSCALL_ERROR_IS_HANDLER                    = 305,
#endif
#ifdef POK_NEEDS_PARTITIONS
	 POK_SYSCALL_PARTITION_SET_MODE                  = 404,
	 POK_SYSCALL_PARTITION_GET_STATUS                = 405,
	 POK_SYSCALL_PARTITION_INC_LOCK_LEVEL		 = 411,
	 POK_SYSCALL_PARTITION_DEC_LOCK_LEVEL		 = 412,
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
         POK_SYSCALL_GET_BSP_INFO                        = 703,
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


//#ifdef POK_ARCH_X86
#ifdef __i386__
/*
 * To reduce the number of functions and improve code coverage, we define
 * only one function to perform the syscall, the other are just maccro
 * This optimization was done only for x86 architecture.
 */
	 pok_ret_t pok_do_syscall (pok_syscall_id_t syscall_id, pok_syscall_args_t* args);

	 #define pok_syscall1(sid,arg1) \
					 pok_do_syscall(sid,&((pok_syscall_args_t){2,arg1,0,0,0,0}))

	 #define pok_syscall2(sid,arg1,arg2) \
					 pok_do_syscall(sid,&((pok_syscall_args_t){2,arg1,arg2,0,0,0}))

	 #define pok_syscall3(sid,arg1,arg2,arg3) \
					 pok_do_syscall(sid,&((pok_syscall_args_t){2,arg1,arg2,arg3,0,0}))

	 #define pok_syscall4(sid,arg1,arg2,arg3,arg4) \
					 pok_do_syscall(sid,&((pok_syscall_args_t){2,arg1,arg2,arg3,arg4,0}))

	 #define pok_syscall5(sid,arg1,arg2,arg3,arg4,arg5) \
					 pok_do_syscall(sid,&((pok_syscall_args_t){2,arg1,arg2,arg3,arg4,arg5}))
#else

pok_ret_t pok_syscall0  (pok_syscall_id_t syscall_id);

pok_ret_t pok_syscall1  (pok_syscall_id_t syscall_id,
												 uint32_t arg1);

pok_ret_t pok_syscall2  (pok_syscall_id_t syscall_id,
        uint32_t         arg1,
        uint32_t         arg2);

pok_ret_t pok_syscall3 (pok_syscall_id_t  syscall_id,
        uint32_t          arg1,
        uint32_t          arg2,
        uint32_t          arg3);

pok_ret_t pok_syscall4 (pok_syscall_id_t  syscall_id,
        uint32_t          arg1,
        uint32_t          arg2,
        uint32_t          arg3,
        uint32_t          arg4);

pok_ret_t pok_syscall5 (pok_syscall_id_t  syscall_id,
        uint32_t arg1,
        uint32_t arg2,
        uint32_t arg3,
        uint32_t arg4,
        uint32_t arg5);
#endif

#include <core/thread.h>
#include <core/partition.h>
#include <middleware/port.h>
#include <middleware/buffer.h>
#include <middleware/blackboard.h>
#include <core/semaphore.h>
#include <core/event.h>
#include <core/error.h>


#include <pok/syscall_map_arinc.h>

#endif /* __LIBPOK_SYSCALL_H__ */
