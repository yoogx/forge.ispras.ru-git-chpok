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

// TODO: Revisit (see also pok_space_* code in arch.h)
pok_bool_t ppc_check_access(const void* __user addr, size_t size)
{
    unsigned long start = (unsigned long)addr;
    unsigned long end = start + size;
    
    return start >= POK_PARTITION_MEMORY_BASE
        && end >= start
        && end < (POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE);
}

pok_bool_t ja_check_access_read(const void* __user addr, size_t size)
{
     return ppc_check_access(addr, size);
}


pok_bool_t ja_check_access_write(void* __user addr, size_t size)
{
     return ppc_check_access(addr, size);
}
