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
     * Alignment of the virtual block (virtual).
     */
    size_t align;

    /*
     * Whether memory block is physically contiguous.
     *
     * Set only when it is requested by configuration.
     * 
     * If block is physically contiguous,
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

/* Translate user address to kernel address for given memory block. */
void* __kuser jet_memory_block_get_kaddr(const struct memory_block* mblock,
    const void* __user addr);


pok_ret_t jet_memory_block_get_status(
    const char* __user name,
    jet_memory_block_status_t* __user status);

/* How memory block may be initialized. */
enum jet_memory_block_init_type
{
    JET_MEMORY_BLOCK_INIT_ZERO, /* Fill with zero */
    JET_MEMORY_BLOCK_INIT_ELF, /* Fill from partition's elf. */
    JET_MEMORY_BLOCK_INIT_BINARY, /* Fill from some binary area "as is". */
};

struct _pok_partition_arinc;

/* 
 * Initialize ARINC partition(s) memory blocks.
 * 
 * The function may assume address space is set for given partition,
 * while 'current_partition_arinc()' macro cannot be used yet.
 * 
 * @part - partition to which memory blocks belongs.
 * @mblocks - NULL terminated array of pointers to memory blocks
 * @source_id - string representing additional information of the initialization source.
 */
void jet_memory_block_init(enum jet_memory_block_init_type init_type,
    struct _pok_partition_arinc* part,
    const struct memory_block* const* mblocks,
    const char* source_id);

/* One entry for initialize memory block(s) at MODULE stage. */
struct jet_module_memory_block_init_entry
{
    enum jet_memory_block_init_type init_type;
    const char* source_id;
    struct _pok_partition_arinc* part;
    const struct memory_block* const* mblocks;
};

/* 
 * Array of initialization entries at MODULE stage.
 * 
 * Set in deployment.c.
 */
extern const struct jet_module_memory_block_init_entry module_memory_block_init_entries[];
extern const int module_memory_block_init_entries_n;

/* Initialize memory blocks at MODULE stage. */
void jet_module_memory_blocks_init(void);

#endif /* __JET_KERNEL_CORE_MEMBLOCKS_H__ */
