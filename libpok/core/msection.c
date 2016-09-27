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

#include <msection.h>
#include <core/syscall.h>
#include <kernel_shared_data.h>

void msection_init(struct msection* section)
{
    section->owner = JET_THREAD_ID_NONE;
    section->msection_kernel_flags = 0;
}

void msection_enter(struct msection* section)
{
    struct jet_thread_shared_data* tshd_current = kshd.tshd + kshd.current_thread_id;

    tshd_current->msection_count++;
    tshd_current->msection_entering = section;

    if(section->owner == JET_THREAD_ID_NONE)
    {
        section->owner = kshd.current_thread_id;
    }
    else if(section->owner != kshd.current_thread_id)
    {
        jet_msection_enter_helper(section); //syscall
    }

    tshd_current->msection_entering = NULL;
}

void msection_leave(struct msection* section)
{
    struct jet_thread_shared_data* tshd_current = kshd.tshd + kshd.current_thread_id;
    // TODO: assert(tshd_current->msection_count > 0);

    section->owner = JET_THREAD_ID_NONE;
    tshd_current->msection_count--;

    if(tshd_current->msection_count == 0
        && tshd_current->thread_kernel_flags & THREAD_KERNEL_FLAG_KILLED)
    {
        // Let us being killed.
        jet_resched();
    }
    else if(section->msection_kernel_flags & MSECTION_KERNEL_FLAG_RESCHED_AFTER_LEAVE)
    {
        // Someone waits on the section.
        jet_resched();
    }
}

