/*
 * COPIED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify original one (kernel/include/uapi/syscall_types.h).
 */
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

#ifndef __LIBJET_SYSCALL_TYPES_H__
#define __LIBJET_SYSCALL_TYPES_H__

#include <config.h>

typedef enum
{
     POK_SYSCALL_CONSWRITE                           =  10,
     POK_SYSCALL_CLOCK_GETTIME                       =  20,
     POK_SYSCALL_TIME                                =  21,
     POK_SYSCALL_INT_NUMBER                          =  42,
     POK_SYSCALL_THREAD_CREATE                       =  50,

     POK_SYSCALL_THREAD_SLEEP                        =  52,

     POK_SYSCALL_THREAD_SUSPEND                      =  53,
//     POK_SYSCALL_THREAD_RESTART                      =  54,
     POK_SYSCALL_THREAD_STOP                         =  55,
     POK_SYSCALL_THREAD_PERIOD                       =  56,
     POK_SYSCALL_THREAD_STOPSELF                     =  57,
     POK_SYSCALL_THREAD_ID                           =  58,
     POK_SYSCALL_THREAD_STATUS                       =  59,
     POK_SYSCALL_THREAD_SET_PRIORITY                 =  60,
     POK_SYSCALL_THREAD_RESUME                       =  61,
     POK_SYSCALL_THREAD_SUSPEND_TARGET               =  62,
     POK_SYSCALL_THREAD_DEADLINE                     =  63,
     POK_SYSCALL_THREAD_STATE                        =  64,
     POK_SYSCALL_THREAD_DELAYED_START                =  65,
     POK_SYSCALL_THREAD_YIELD                        =  66,
     POK_SYSCALL_THREAD_REPLENISH                    =  67,
     POK_SYSCALL_THREAD_FIND                         =  68,

     POK_SYSCALL_RESCHED                             =  80,
     POK_SYSCALL_MSECTION_ENTER_HELPER               =  81,
     POK_SYSCALL_MSECTION_WAIT                       =  82,
     POK_SYSCALL_MSECTION_NOTIFY                     =  83,
     POK_SYSCALL_MSECTION_WQ_NOTIFY                  =  84,
     POK_SYSCALL_MSECTION_WQ_SIZE                    =  85,

     POK_SYSCALL_MIDDLEWARE_SAMPLING_ID              = 101,
     POK_SYSCALL_MIDDLEWARE_SAMPLING_READ            = 102,
     POK_SYSCALL_MIDDLEWARE_SAMPLING_STATUS          = 103,
     POK_SYSCALL_MIDDLEWARE_SAMPLING_WRITE           = 104,
     POK_SYSCALL_MIDDLEWARE_SAMPLING_CREATE          = 105,
     POK_SYSCALL_MIDDLEWARE_SAMPLING_CHECK           = 106,

     POK_SYSCALL_MIDDLEWARE_QUEUEING_CREATE          = 110,
     POK_SYSCALL_MIDDLEWARE_QUEUEING_SEND            = 111,
     POK_SYSCALL_MIDDLEWARE_QUEUEING_RECEIVE         = 112,
     POK_SYSCALL_MIDDLEWARE_QUEUEING_ID              = 113,
     POK_SYSCALL_MIDDLEWARE_QUEUEING_STATUS          = 114,
     POK_SYSCALL_MIDDLEWARE_QUEUEING_CLEAR           = 115,

     POK_SYSCALL_ERROR_HANDLER_CREATE                = 301,
     POK_SYSCALL_ERROR_RAISE_APPLICATION_ERROR       = 303,
     POK_SYSCALL_ERROR_GET                           = 304,
     POK_SYSCALL_ERROR_IS_HANDLER                    = 305,

     POK_SYSCALL_ERROR_RAISE_OS_ERROR                = 310,

     POK_SYSCALL_PARTITION_SET_MODE                  = 404,
     POK_SYSCALL_PARTITION_GET_STATUS                = 405,
     POK_SYSCALL_PARTITION_INC_LOCK_LEVEL            = 411,
     POK_SYSCALL_PARTITION_DEC_LOCK_LEVEL            = 412,

     POK_SYSCALL_MEMORY_BLOCK_GET_STATUS             = 701,

     POK_SYSCALL_IPPC_INIT_PORTAL                    = 750,
     POK_SYSCALL_IPPC_CALL                           = 751,

     POK_SYSCALL_IPPC_GET_PORTAL_TYPE_INFO           = 752,
     POK_SYSCALL_IPPC_GET_PORTAL_INFO                = 753,
     POK_SYSCALL_IPPC_CREATE_CONNECTIONS             = 754,
     POK_SYSCALL_IPPC_RETURN                         = 755,

     POK_SYSCALL_GCOV_INIT                           = 801,
} pok_syscall_id_t;

#endif /* __LIBJET_SYSCALL_TYPES_H__ */
