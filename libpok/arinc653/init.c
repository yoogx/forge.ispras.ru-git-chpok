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

#include "buffer.h"
#include "blackboard.h"
#include "event.h"
#include "semaphore.h"

#include <stdio.h> /* for printf() */
#include <stdlib.h> /* for abort() */

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
 * Allocate array for ARINC needs on initialization stage.
 *
 * On error print appropriate message and abort().
 */

static void* arinc_alloc_array(size_t n_elems, size_t elem_size, size_t alignment)
{
    if (n_elems == 0) return NULL;

    void* res = arinc_alloc(n_elems * elem_size, alignment);

    if(res == NULL)
    {
        printf("ERROR: Insufficient memory for ARINC needs.\n");
        printf("NOTE: Report this error to the developers.\n");
        abort();
    }

    return res;
}

#define ARINC_ALLOC_ARRAY(n_elems, type) (type*)arinc_alloc_array(n_elems, sizeof(type), __alignof__(type))

void libjet_arinc_init(void)
{
    arinc_allocator_init();

#ifdef POK_NEEDS_ARINC653_BUFFER
    arinc_buffers = ARINC_ALLOC_ARRAY(arinc_config_nbuffers, struct arinc_buffer);
#endif /* POK_NEEDS_ARINC653_BUFFER */

#ifdef POK_NEEDS_ARINC653_BLACKBOARD
    arinc_blackboards = ARINC_ALLOC_ARRAY(arinc_config_nblackboards, struct arinc_blackboard);
#endif /* POK_NEEDS_ARINC653_BLACKBOARD */

#ifdef POK_NEEDS_ARINC653_SEMAPHORE
    arinc_semaphores = ARINC_ALLOC_ARRAY(arinc_config_nsemaphores, struct arinc_semaphore);
#endif /* POK_NEEDS_ARINC653_SEMAPHORE */

#ifdef POK_NEEDS_ARINC653_EVENT
    arinc_events = ARINC_ALLOC_ARRAY(arinc_config_nevents, struct arinc_event);
#endif /* POK_NEEDS_ARINC653_EVENT */
}
