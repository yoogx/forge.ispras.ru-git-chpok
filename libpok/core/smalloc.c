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

#include <smalloc.h>
#include <kernel_shared_data.h>
#include <core/assert_os.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils.h>

#include <types.h>

#include <memblocks.h>

/*
 * Initialize allocator with given memory region and maximum alignment
 * allowed.
 */
void jet_sallocator_init(struct jet_sallocator* sallocator,
    void* addr, size_t size, size_t max_alignment)
{
    sallocator->addr_current = addr;
    sallocator->addr_end = sallocator->addr_current + size;
    sallocator->max_alignment = max_alignment;
}

/*
 * Initialize allocator from memory region defined by the memory block.
 */
void jet_sallocator_init_from_memblock(struct jet_sallocator* sallocator,
    struct _jet_memory_block_status* mb_status)
{
    sallocator->addr_current = (void*)mb_status->addr;
    sallocator->addr_end = sallocator->addr_current + mb_status->size;
    sallocator->max_alignment = mb_status->align;
}

void* jet_sallocator_alloc_aligned(struct jet_sallocator* sallocator, size_t size,
    size_t alignment)
{
    assert_os(kshd->partition_mode != POK_PARTITION_MODE_NORMAL);

    if(size == 0) return NULL;

    if(alignment > sallocator->max_alignment) {
        printf ("ERROR:Request for allocate memory of alignment %d, while allocation region at %p allows at most %d alignment.",
            (int)alignment, sallocator->addr_current, (int)sallocator->max_alignment);
        abort();
    }

    char* obj_start = ALIGN_PTR(sallocator->addr_current, alignment);
    char* obj_end = obj_start + size;

    if(obj_end > sallocator->addr_end) return NULL;

    sallocator->addr_current = obj_end;

    return obj_start;
}

void* jet_sallocator_alloc(struct jet_sallocator* sallocator, size_t size)
{
    return jet_sallocator_alloc_aligned(sallocator, size,
        libjet_mem_get_alignment(size));
}

void* jet_sallocator_alloc_array(struct jet_sallocator* sallocator,
    size_t n_elems, size_t size)
{
    return jet_sallocator_alloc_aligned(sallocator, size * n_elems,
        libjet_mem_get_alignment(size));
}

void* jet_sallocator_get_state(struct jet_sallocator* sallocator)
{
    return sallocator->addr_current;
}

void jet_sallocator_set_state(struct jet_sallocator* sallocator,
    void* state)
{
    sallocator->addr_current = state;
}



static struct jet_sallocator smalloc_allocator;

void smalloc_init(void)
{
    jet_memory_block_status_t heap_status;

    pok_ret_t ret = pok_memory_block_get_status(".HEAP", &heap_status);
    if(ret != POK_ERRNO_OK) {
        printf("ERROR: Memory block for heap is not created.\n");
        printf("NOTE: Report this error to the developers.\n");
        abort();
    }

    jet_sallocator_init_from_memblock(&smalloc_allocator, &heap_status);
}

void* smalloc_aligned(size_t size, size_t alignment)
{
    void* res = jet_sallocator_alloc_aligned(&smalloc_allocator, size, alignment);

    if(res == NULL)
    {
        printf("ERROR: Failed to allocate memory from the heap.\n");
        printf("NOTE: Probably, you need to configure more heap for the partition.\n");
        abort();
    }

    return res;
}

void* smalloc(size_t size)
{
    return smalloc_aligned(size, libja_mem_get_alignment(size));
}

void* scalloc(size_t nmemb, size_t size)
{
    return smalloc_aligned(nmemb * size, libja_mem_get_alignment(size));
}
