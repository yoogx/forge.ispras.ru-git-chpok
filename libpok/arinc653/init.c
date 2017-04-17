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
#include "arinc_config.h"

#include <asp/alloc.h>

#include "buffer.h"
#include "blackboard.h"
#include "event.h"
#include "semaphore.h"
#include "logbook.h"

#include <kernel_shared_data.h>

#include <stdio.h> /* for printf() */
#include <stdlib.h> /* for abort() */

struct arinc_buffer* arinc_buffers;

struct arinc_blackboard* arinc_blackboards;

struct arinc_event* arinc_events;

struct arinc_semaphore* arinc_semaphores;

struct arinc_logbook *arinc_logbooks;


char* arinc_intra_heap = NULL;

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

size_t arinc_config_nbuffers;
size_t arinc_config_nblackboards;
size_t arinc_config_nsemaphores;
size_t arinc_config_nevents;
size_t arinc_config_messages_memory_size;

size_t arinc_config_nlogbooks;

static void logbook_init()
{
    unsigned i = 0;
    for (i = 0; i < arinc_config_nlogbooks; i++)
    {
        arinc_logbooks[i].max_message_size = kshd->logbooks_data[i].message_size;
        arinc_logbooks[i].max_nb_logged_messages = kshd->logbooks_data[i].max_nb_logged_messages;
        arinc_logbooks[i].max_nb_in_progress_messages = kshd->logbooks_data[i].max_nb_in_progress_messages;
        strcpy(arinc_logbooks[i].logbook_name, kshd->logbooks_data[i].logbook_name);        
    }
}


void libjet_arinc_init(void)
{
    arinc_allocator_init();

    arinc_config_nbuffers = kshd->arinc_config_nbuffers;
    arinc_config_nblackboards = kshd->arinc_config_nblackboards;
    arinc_config_nsemaphores = kshd->arinc_config_nsemaphores;
    arinc_config_nevents = kshd->arinc_config_nevents;
    arinc_config_messages_memory_size = kshd->arinc_config_messages_memory_size;
    
    arinc_config_nlogbooks = kshd->arinc_config_nlogbooks;

    arinc_buffers = ARINC_ALLOC_ARRAY(arinc_config_nbuffers, struct arinc_buffer);
    arinc_blackboards = ARINC_ALLOC_ARRAY(arinc_config_nblackboards, struct arinc_blackboard);
    arinc_semaphores = ARINC_ALLOC_ARRAY(arinc_config_nsemaphores, struct arinc_semaphore);
    arinc_events = ARINC_ALLOC_ARRAY(arinc_config_nevents, struct arinc_event);

    arinc_logbooks = ARINC_ALLOC_ARRAY(arinc_config_nlogbooks, struct arinc_logbook);
    logbook_init();    
    
}
