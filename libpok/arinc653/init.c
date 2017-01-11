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

#include <init_arinc.h>
#include "arinc_alloc.h"
#include <arinc_config.h>

#include <asp/alloc.h>
#include <memblocks.h>

#include "buffer.h"
#include "blackboard.h"
#include "event.h"
#include "semaphore.h"

#include <stdio.h> /* for printf() */
#include <stdlib.h> /* for abort() */
#include <utils.h> /* ALIGN_PTR */

#ifdef POK_NEEDS_ARINC653_BUFFER
struct arinc_buffer* arinc_buffers;
#endif /* POK_NEEDS_ARINC653_BUFFER */

#ifdef POK_NEEDS_ARINC653_BLACKBOARD
struct arinc_blackboard* arinc_blackboards;
#endif /* POK_NEEDS_ARINC653_BLACKBOARD */

#ifdef POK_NEEDS_ARINC653_EVENT
struct arinc_event* arinc_events;
#endif /* POK_NEEDS_ARINC653_EVENT */

#ifdef POK_NEEDS_ARINC653_SEMAPHORE
struct arinc_semaphore* arinc_semaphores;
#endif /* POK_NEEDS_ARINC653_SEMAPHORE */


#if defined(POK_NEEDS_ARINC653_BUFFER) || defined(POK_NEEDS_ARINC653_BLACKBOARD)
char* arinc_intra_heap = NULL;
#endif /* defined(POK_NEEDS_ARINC653_BUFFER) || defined(POK_NEEDS_ARINC653_BLACKBOARD) */

/*
 * ARINC heap.
 *
 * It is used for allocate things at initialization.
 */
struct arinc_heap
{
    char* ptr;
    size_t size;
};

/* Advance heap usage by given size. */
static void aheap_advance(struct arinc_heap* aheap, size_t size)
{
    if(aheap->size < size)
    {
        printf("ERROR: Insufficient memory for ARINC needs.\n");
        printf("NOTE: Report this error to the developers.\n");
        abort();
    }

    aheap->ptr += size;
    aheap->size -= size;
}

/* Align pointer of the heap. */
static void aheap_align(struct arinc_heap* aheap, size_t alignment)
{
    char* ptr_new = ALIGN_PTR(aheap->ptr, alignment);

    aheap_advance(aheap, ptr_new - aheap->ptr);
}


/*
 * Allocate array for ARINC needs.
 *
 * Update heap usage accordingly.
 */
static void* aheap_alloc_array(struct arinc_heap* aheap, size_t n_elems, size_t elem_size)
{
    if(n_elems == 0) return NULL; // Note: This is not an error indicator

    aheap_align(aheap, libja_mem_get_alignment(elem_size));

    void* res = aheap->ptr;

    aheap_advance(aheap, n_elems * elem_size);

    return res;
}



void libjet_arinc_init(void)
{
    jet_memory_block_status_t arinc_heap_status;

    pok_ret_t ret = pok_memory_block_get_status(".ARINC_HEAP",
        &arinc_heap_status);

    if(ret != POK_ERRNO_OK) {
        printf("ERROR: Memory block for ARINC heap is not created.\n");
        printf("NOTE: Report this error to the developers.\n");
        abort();
    }

    struct arinc_heap aheap = {
        .ptr = (char*)arinc_heap_status.addr,
        .size = arinc_heap_status.size,
    };

#ifdef POK_NEEDS_ARINC653_BUFFER
    arinc_buffers = aheap_alloc_array(&aheap,
        arinc_config_nbuffers, sizeof(*arinc_buffers));
#endif /* POK_NEEDS_ARINC653_BUFFER */

#ifdef POK_NEEDS_ARINC653_BLACKBOARD
    arinc_blackboards = aheap_alloc_array(&aheap,
        arinc_config_nblackboards, sizeof(*arinc_blackboards));
#endif /* POK_NEEDS_ARINC653_BLACKBOARD */

#ifdef POK_NEEDS_ARINC653_SEMAPHORE
    arinc_semaphores = aheap_alloc_array(&aheap,
        arinc_config_nsemaphores, sizeof(*arinc_semaphores));
#endif /* POK_NEEDS_ARINC653_SEMAPHORE */

#ifdef POK_NEEDS_ARINC653_EVENT
    arinc_events = aheap_alloc_array(&aheap,
        arinc_config_nevents, sizeof(*arinc_events));
#endif /* POK_NEEDS_ARINC653_EVENT */


#if defined(POK_NEEDS_ARINC653_BUFFER) || defined(POK_NEEDS_ARINC653_BLACKBOARD)
    aheap_align(&aheap, libja_mem_get_alignment(sizeof(size_t)));

    arinc_intra_heap = aheap.ptr;

    aheap_advance(&aheap, arinc_config_messages_memory_size);
#endif /* defined(POK_NEEDS_ARINC653_BUFFER) || defined(POK_NEEDS_ARINC653_BLACKBOARD) */
}
