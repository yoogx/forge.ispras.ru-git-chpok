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

/*
 * Access to user space from kernel space.
 */

#include <core/uaccess.h>
#include <core/partition.h>

/* 
 * Return kernel address which can be used in the kernel for
 * r/w from/to current partition.
 * 
 * If given area cannot be write by the user space, returns NULL.
 */
void* __kuser jet_user_to_kernel(void* __user addr, size_t size)
{
    return ja_user_to_kernel_space(addr, size, current_partition->space_id);
}

/* 
 * Return kernel address which can be used in the kernel for
 * read from current partition.
 * 
 * If given area cannot be read by the user space, returns NULL.
 */
const void* __kuser jet_user_to_kernel_ro(const void* __user addr, size_t size)
{
    return ja_user_to_kernel_ro_space(addr, size, current_partition->space_id);
}

/*
 * Return true if address *may* contain instruction, which can be
 * executed by the user in the current partition.
 * 
 * Used mainly for additional checks.
 * Kernel doesn't perform r/w to given address.
 */
pok_bool_t jet_check_access_exec(void* __user addr)
{
    return ja_check_access_exec(addr, current_partition->space_id);
}

void pok_copy_name_to_user(char* to, const char* name)
{
    pok_bool_t null_found = FALSE;

    for(int i = 0; !null_found && (i < MAX_NAME_LENGTH); i++)
    {
        to[i] = name[i];
        if(name[i] == '\0') null_found = TRUE;
    }
}
