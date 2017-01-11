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

#include <core/memblocks.h>
#include <libc.h>
#include <core/partition.h>
#include <core/uaccess.h>

#include <types.h>

const struct memory_block* jet_partition_find_memory_block(pok_partition_t* part,
    const char* name)
{
    for(int i = 0; i < part->nmemory_blocks; i++)
    {
        const struct memory_block* mblock = &part->memory_blocks[i];

        if(strcmp(mblock->name, name) == 0) return mblock;
    }

    return NULL;
}


pok_ret_t pok_memory_block_get_status(
        const char* __user name,
        jet_memory_block_status_t* __user status)
{
    pok_partition_t* part = current_partition;

    jet_memory_block_status_t* status_kernel = jet_user_to_kernel_typed(status);

    if(status_kernel == NULL) return POK_ERRNO_EFAULT;

    // Assume name of the memory block at most MAX_NAME_LENGTH length.
    char name_kernel[MAX_NAME_LENGTH];

    if(kstrncpy(name_kernel, name, MAX_NAME_LENGTH) == NULL)
        return POK_ERRNO_EFAULT;

    const struct memory_block* mblock = jet_partition_find_memory_block(part,
        name_kernel);

    if(mblock == NULL) return POK_ERRNO_EINVAL;

    if(mblock->maccess | MEMORY_BLOCK_ACCESS_WRITE)
        status_kernel->mode = JET_MEMORY_BLOCK_READ_WRITE;
    else if(mblock->maccess | MEMORY_BLOCK_ACCESS_READ)
        status_kernel->mode = JET_MEMORY_BLOCK_READ;
    else
        return POK_ERRNO_EINVAL; // Function returns only readable blocks.

    status_kernel->addr = mblock->vaddr;
    status_kernel->size = mblock->size;

    if(mblock->is_contiguous) {
        status_kernel->is_contiguous = TRUE;
        status_kernel->paddr = mblock->paddr;
    }
    else {
        status_kernel->is_contiguous = FALSE;
    }

    return POK_ERRNO_OK;
}
