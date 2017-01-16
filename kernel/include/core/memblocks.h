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

#ifndef __JET_KERNEL_CORE_MEMBLOCKS_H__
#define __JET_KERNEL_CORE_MEMBLOCKS_H__

#include <uapi/memblock_types.h>
#include <common.h>
#include <errno.h>

/* How user *intend* to access the memory block. */
#define MEMORY_BLOCK_ACCESS_READ 1
#define MEMORY_BLOCK_ACCESS_WRITE 2
#define MEMORY_BLOCK_ACCESS_EXEC 4

struct memory_block
{
    const char* name;

    size_t size;

    int maccess;

    /*
     * Virtual address of the beginning of the memory block.
     *
     * Note: Memory block is always *virtually* contiguous.
     */
    uintptr_t vaddr;

    /*
     * Whether memory block is physically contiguous.
     *
     * Set only when it is requested by configuration.
     */
    pok_bool_t is_contiguous;

    /*
     * Physical address of the beginning of the memory block.
     *
     * Has a sence only when 'is_contiguous' is true.
     */
    uint64_t paddr;

    /* This memory block coincides with some block of other partition. */
    pok_bool_t is_shared;

    /*
     * Kernel address of the beginning of the memory block.
     *
     * This address is needed only for user_to_kernel() transformations.
     *
     * Note: This implies, that memory block is always virtually contiguous
     * not only in user space, but in kernel space too.
     */
    uintptr_t kaddr;
};

pok_ret_t pok_memory_block_get_status(
        const char* __user name,
        jet_memory_block_status_t* __user status);

#endif /* __JET_KERNEL_CORE_MEMBLOCKS_H__ */
