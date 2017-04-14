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
#include <core/partition_arinc.h>
#include <core/memblocks.h>

void* __kuser jet_user_to_kernel(void* __user addr, size_t size)
{
    const struct memory_block* mblock = jet_partition_arinc_get_memory_block_for_addr(
        current_partition_arinc, addr, size);

    if(mblock && mblock->maccess & MEMORY_BLOCK_ACCESS_WRITE)
        return jet_memory_block_get_kaddr(mblock, addr);

    return NULL;
}

/*
 * Return kernel address which can be used in the kernel for
 * read from current partition.
 *
 * If given area cannot be read by the user space, returns NULL.
 */
const void* __kuser jet_user_to_kernel_ro(const void* __user addr, size_t size)
{
    const struct memory_block* mblock = jet_partition_arinc_get_memory_block_for_addr(
        current_partition_arinc, addr, size);

    if(mblock && mblock->maccess & MEMORY_BLOCK_ACCESS_READ)
        return jet_memory_block_get_kaddr(mblock, addr);

    return NULL;
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
    const struct memory_block* mblock = jet_partition_arinc_get_memory_block_for_addr(
        current_partition_arinc, addr, 1);

    if(mblock && mblock->maccess & MEMORY_BLOCK_ACCESS_EXEC)
        return TRUE;

    return FALSE;
}

void* kstrncpy(char* dest, const char* __user src, size_t n)
{
    uintptr_t vaddr = (uintptr_t)src;

    // At least one byte should be mapped.
    const struct memory_block* mblock = jet_partition_arinc_get_memory_block_for_addr(
        current_partition_arinc, src, 1);

    if(!mblock) return NULL;

    // Check that block is readable
    if(!(mblock->maccess | MEMORY_BLOCK_ACCESS_READ)) return NULL;

    // Maximum size available to the end of the block.
    uintptr_t rest_size = mblock->vaddr + mblock->size - vaddr;

    // Maximum number of bytes to check.
    size_t check_size = rest_size > n ? n : (size_t)rest_size;

    const char* __kuser ksrc = jet_memory_block_get_kaddr(mblock, src);

    unsigned i;
    for(i = 0; i < check_size; i++)
    {
        dest[i] = ksrc[i];

        if(ksrc[i] == 0) break;
    }

    // If no null byte is found and not all 'n' bytes are checked, this is seg fault.
    if((i == check_size) && (check_size < n)) return NULL;

    return dest;
}

#ifdef POK_NEEDS_GDB

/* Same function for jet_addr_to_gdb() and jet_addr_to_gdb_ro(). */
static void* __kuser addr_to_gdb_common(const void* __user addr, size_t size,
    pok_partition_t* part)
{
    /*
     * If address is suitable for kernel, treat it as a kernel.
     *
     * This means, that for segment memory model on x86 some user
     * addresses are treated as kernel.
     */
    if(!ja_access_ok(addr, size)) return (void*)addr;

    // Kernel-only partitions cannot access user space.
    if(part->space_id == 0) return NULL;

    void* __kuser kaddr = part->part_sched_ops->uaddr_to_gdb(part, addr, size);

    return kaddr;
}



void* __kuser jet_addr_to_gdb(void* __user addr, size_t size,
    pok_partition_t* part)
{
    return addr_to_gdb_common(addr, size, part);
}

const void* __kuser jet_addr_to_gdb_ro(const void* __user addr, size_t size,
    pok_partition_t* part)
{
    return addr_to_gdb_common(addr, size, part);
}

#endif /* POK_NEEDS_GDB */

void pok_copy_name_to_user(char* to, const char* name)
{
    pok_bool_t null_found = FALSE;

    for(int i = 0; !null_found && (i < MAX_NAME_LENGTH); i++)
    {
        to[i] = name[i];
        if(name[i] == '\0') null_found = TRUE;
    }
}
