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
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */


#ifndef __POK_KERNEL_SEMAPHORE_H__
#define __POK_KERNEL_SEMAPHORE_H__

#include <config.h>

#include <core/dependencies.h>

#ifdef POK_NEEDS_SEMAPHORES

#include <types.h>
#include <errno.h>

typedef struct {
    pok_sem_value_t current_value;
    pok_sem_value_t maximum_value;
    pok_range_t waiting_processes; 
} pok_semaphore_status_t;

// All semaphore-related functions are already defined as syscalls.
#include <core/syscall.h>


#endif

#endif
