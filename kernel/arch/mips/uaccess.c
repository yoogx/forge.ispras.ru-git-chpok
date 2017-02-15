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

#define POK_PARTITION_MEMORY_BASE 0x60000000ULL

pok_bool_t ja_access_ok(const void* __user addr, size_t size)
{
    uint32_t vaddr = (uint32_t)addr;

    return (vaddr >= POK_PARTITION_MEMORY_BASE)
        && (vaddr + size >= POK_PARTITION_MEMORY_BASE);
}
