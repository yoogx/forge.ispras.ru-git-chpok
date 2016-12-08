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

#include <asp/uaccess.h>
#include "space.h"
#include <assert.h>

// TODO: Revisit (see also pok_space_* code in arch.h)
static pok_bool_t ppc_check_access(const void* __user addr, size_t size,
    jet_space_id space_id)
{
    assert(space_id != 0);
    assert(size != 0);
    return TRUE;
#if 0
    unsigned long start = (unsigned long)addr;
    unsigned long end = start + size;

    struct ja_ppc_space* space = &ja_spaces[space_id - 1];

    /*
     * Currently, there are 2 segments accessible to user:
     * 1. [POK_PARTITION_MEMORY_BASE; POK_PARTITION_MEMORY_BASE + space->size_total)
     *    code and data
     * 2. [space->ustack_state; POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE)
     *    stacks
     */

    if(end < start) return FALSE; // Segments doesn't cross NULL.

    if(end <= space->size_total + POK_PARTITION_MEMORY_BASE)
    {
        return (start >= POK_PARTITION_MEMORY_BASE);
    }
    else if(start >= space->ustack_state)
    {
        return (end <= POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE);
    }
    else
    {
        return FALSE;
    }
#endif
}

void* __kuser ja_user_to_kernel_space(void* __user addr, size_t size,
    jet_space_id space_id)
{   
    //printf("__user addr : %p\n", addr);
    if(ppc_check_access(addr, size, space_id))
       return (void* __kuser)addr;
    else
       return NULL;
}

const void* __kuser ja_user_to_kernel_ro_space(const void* __user addr,
    size_t size, jet_space_id space_id)
{
    //printf("__user addr : %p\n", addr);
    if(ppc_check_access(addr, size, space_id))
       return (const void* __kuser)addr;
    else
       return NULL;
}

pok_bool_t ja_check_access_exec(void* __user addr, jet_space_id space_id)
{
    assert(space_id != 0);

    unsigned long start = (unsigned long)addr;

    struct ja_ppc_space* space = &ja_spaces[space_id - 1];

    /*
     * Only single segment could be executed by user:
     *   [POK_PARTITION_MEMORY_BASE; POK_PARTITION_MEMORY_BASE + space->size_normal)
     *    code
     */

    return (start < POK_PARTITION_MEMORY_BASE + space->size_normal)
        && (start >= POK_PARTITION_MEMORY_BASE);
}

#ifdef POK_NEEDS_GDB

static pok_bool_t ppc_check_access_gdb(const void* addr, size_t size,
    jet_space_id space_id)
{
    // All threads have access to kernel part.
    if((unsigned long)addr >= 0
        && ((unsigned long)addr + size) < 0x4000000UL)
        return TRUE;

    if(space_id != 0)
    {
        // User partitions have access to their user space.
        return ppc_check_access((const void* __user)addr, size, space_id);
    }
    else
    {
        return FALSE;
    }
}


void* ja_addr_to_gdb(void* addr, size_t size,
    jet_space_id space_id)
{
    if(ppc_check_access_gdb(addr, size, space_id))
        return addr;
    else
        return NULL;
}

const void* ja_addr_to_gdb_ro(const void* addr, size_t size,
    jet_space_id space_id)
{
    if(ppc_check_access_gdb(addr, size, space_id))
        return addr;
    else
        return NULL;
}

#endif /* POK_NEEDS_GDB */
