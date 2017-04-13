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
#include <core/partition_arinc.h>
#include <core/uaccess.h>
#include <core/loader.h>

#include <types.h>

#include <conftree.h>

void* __kuser jet_memory_block_get_kaddr(const struct memory_block* mblock,
    const void* __user addr)
{
    return (void* __kuser)((uintptr_t)addr - mblock->vaddr + mblock->kaddr);
}

pok_ret_t jet_memory_block_get_status(
    const char* __user name,
    jet_memory_block_status_t* __user status)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    jet_memory_block_status_t* status_kernel = jet_user_to_kernel_typed(status);

    if(status_kernel == NULL) return POK_ERRNO_EFAULT;

    // Assume name of the memory block at most MAX_NAME_LENGTH length.
    char name_kernel[MAX_NAME_LENGTH];

    if(kstrncpy(name_kernel, name, MAX_NAME_LENGTH) == NULL)
        return POK_ERRNO_EFAULT;

    const struct memory_block* mblock = jet_partition_arinc_find_memory_block(part,
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
    status_kernel->align = mblock->align;

    if(mblock->is_contiguous) {
        status_kernel->is_contiguous = TRUE;
        status_kernel->paddr = mblock->paddr;
    }
    else {
        status_kernel->is_contiguous = FALSE;
    }

    return POK_ERRNO_OK;
}

static void jet_memory_block_init_zero(pok_partition_arinc_t* part,
    const struct memory_block* const* mblocks)
{
    (void) part; //ignore partition

    for(const struct memory_block* const * p_mblock = mblocks;
        *p_mblock != NULL;
        p_mblock++)
    {
        const struct memory_block* mblock = *p_mblock;

        memset((void* __kuser)mblock->kaddr, 0, mblock->size);
    }
}

static void jet_memory_block_init_elf(pok_partition_arinc_t* part,
    const char* source_id, const struct memory_block* const* mblocks)
{
    /* source_id is a path in the configuration tree to the ELF image.*/
    const void* elf_image = jet_pt_get_binary(kernel_config_tree, JET_PT_ROOT, source_id, NULL);
    assert(elf_image);

    jet_loader_elf_load(elf_image, part, mblocks, &part->main_entry);
}

static void jet_memory_block_init_binary(pok_partition_arinc_t* part,
    const char* source_id, const struct memory_block* const* mblocks)
{
    (void) part; //ignore partition
    /* source_id is a path in the configuration tree to the binary data.*/
    size_t binary_size;
    const void* binary_image = jet_pt_get_binary(kernel_config_tree, JET_PT_ROOT, source_id, &binary_size);
    assert(binary_image);

    /* Every memory block is filled with the same data. */
    for(const struct memory_block* const * p_mblock = mblocks;
        *p_mblock != NULL;
        p_mblock++)
    {
        const struct memory_block* mblock = *p_mblock;

        assert(mblock->size >= binary_size); //TODO: This should be INVALID_CONFIG HM error.
        memcpy((char* __kuser)mblock->kaddr, binary_image, binary_size);

        if(mblock->size > binary_size) {
            // Fill the rest with zeros
            memset((char* __kuser)mblock->kaddr + binary_size, 0, mblock->size - binary_size);
        }
    }
}


void jet_memory_block_init(enum jet_memory_block_init_type init_type,
    struct _pok_partition_arinc* part,
    const struct memory_block* const* mblocks,
    const char* source_id)
{
    switch(init_type) {
        case JET_MEMORY_BLOCK_INIT_ZERO:
            jet_memory_block_init_zero(part, mblocks);
        break;
        case JET_MEMORY_BLOCK_INIT_ELF:
            jet_memory_block_init_elf(part, source_id, mblocks);
        break;
        case JET_MEMORY_BLOCK_INIT_BINARY:
            jet_memory_block_init_binary(part, source_id, mblocks);
        break;
    }
}

/* Initialize memory blocks at MODULE stage. */
void jet_module_memory_blocks_init(void)
{
    for(int i = 0; i < module_memory_block_init_entries_n; i++) {
        const struct jet_module_memory_block_init_entry* init_entry
            = &module_memory_block_init_entries[i];

        ja_space_switch(init_entry->part->base_part.space_id);

        jet_memory_block_init(init_entry->init_type,
            init_entry->part,
            init_entry->mblocks,
            init_entry->source_id
        );
    }
}
