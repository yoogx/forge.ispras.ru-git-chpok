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

#include "arinc_alloc.h"
#include <utils.h>
#include <memblocks.h>
#include <smalloc.h>

#include <stdio.h> /* for printf() */
#include <stdlib.h> /* for abort() */


static struct jet_sallocator arinc_allocator;

void arinc_allocator_init(void)
{
    jet_memory_block_status_t arinc_heap_status;

    jet_ret_t ret = jet_memory_block_get_status(".ARINC_HEAP",
        &arinc_heap_status);

    if(ret != EOK) {
        printf("ERROR: Memory block for ARINC heap is not created.\n");
        printf("NOTE: Report this error to the developers.\n");
        abort();
    }

    jet_sallocator_init_from_memblock(&arinc_allocator, &arinc_heap_status);
}

/* 
 * Allocate memory of given size.
 * 
 * Memory will have alignment suitable for any object which fits into
 * given size.
 */
void* arinc_alloc(size_t size, size_t alignment)
{
    return jet_sallocator_alloc_aligned(&arinc_allocator, size, alignment);
}

/*
 * Return current state of the allocator.
 */
arinc_allocator_state arinc_allocator_get_state(void)
{
    return jet_sallocator_get_state(&arinc_allocator);
}

/*
 * Restore state of the arinc allocator.
 * State should be previously obtained with arinc_allocator_get_state().
 */
void arinc_allocator_reset_state(arinc_allocator_state state)
{
    jet_sallocator_set_state(&arinc_allocator, state);
}
