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
#include <core/partition_arinc.h>


// TODO: Revisit (see also pok_space_* code in arch.h)
pok_bool_t ja_check_access(const void* __user addr, size_t size)
{
    unsigned long start = (unsigned long)addr;
    unsigned long end = start + size;
    pok_partition_arinc_t* part = current_partition_arinc;

    // Assume partition area doesn't cross 0.
    return (start >= part->base_vaddr)
        && (end >= start) // Check that area doesn't cross 0.
        && end < (part->base_vaddr + part->size);
}

pok_bool_t ja_check_access_read(const void* __user addr, size_t size)
{
     return ja_check_access(addr, size);
}


pok_bool_t ja_check_access_write(void* __user addr, size_t size)
{
     return ja_check_access(addr, size);
}

void* ja_user_to_kernel(void* __user addr)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    return (void*)((unsigned long)addr - part->base_vaddr + part->base_addr);
}

const void* ja_user_to_kernel_ro(const void* __user addr)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    return (const void*)((unsigned long)addr - part->base_vaddr + part->base_addr);
}
