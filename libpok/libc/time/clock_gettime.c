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
 */

#include <time.h>
#include <core/syscall.h>

int clock_gettime(clockid_t clock_id, struct timespec* tp)
{
    pok_time_t t;
    
    jet_ret_t ret = pok_syscall2(POK_SYSCALL_CLOCK_GETTIME, (uintptr_t)clock_id, (uintptr_t)&t);
    
    if(ret != EOK) return -1; // TODO: set errno.
    
    tp->tv_sec = t / 1000000000;
    tp->tv_nsec = t % 1000000000;
    
    return 0;
}
