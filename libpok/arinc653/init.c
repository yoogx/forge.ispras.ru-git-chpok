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

#include <stdio.h> /* for printf() */
#include <stdlib.h> /* for abort() */

#include <conftree.h>

struct arinc_buffer* arinc_buffers;

struct arinc_blackboard* arinc_blackboards;

struct arinc_event* arinc_events;

struct arinc_semaphore* arinc_semaphores;


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

// Explicitely initialize variables with default values.
uint32_t arinc_config_nbuffers = 0;
uint32_t arinc_config_nblackboards = 0;
uint32_t arinc_config_nsemaphores = 0;
uint32_t arinc_config_nevents = 0;
uint32_t arinc_config_messages_memory_size = 0;

void libjet_arinc_init(void)
{
    arinc_allocator_init();

    jet_pt_node_t arinc_node = jet_pt_find(part_config_tree, JET_PT_ROOT, "ARINC");
    if(arinc_node != JET_PT_INVALID_NODE) {
        // Ignore error from get functions: variables already have default values.
        (void)jet_pt_get_uint32(part_config_tree, arinc_node, "buffers_n", &arinc_config_nbuffers);
        (void)jet_pt_get_uint32(part_config_tree, arinc_node, "blackboards_n", &arinc_config_nblackboards);
        (void)jet_pt_get_uint32(part_config_tree, arinc_node, "semaphores_n", &arinc_config_nsemaphores);
        (void)jet_pt_get_uint32(part_config_tree, arinc_node, "events_n", &arinc_config_nevents);
        (void)jet_pt_get_uint32(part_config_tree, arinc_node, "messages_memory_size_n", &arinc_config_messages_memory_size);
    }

    arinc_buffers = ARINC_ALLOC_ARRAY(arinc_config_nbuffers, struct arinc_buffer);
    arinc_blackboards = ARINC_ALLOC_ARRAY(arinc_config_nblackboards, struct arinc_blackboard);
    arinc_semaphores = ARINC_ALLOC_ARRAY(arinc_config_nsemaphores, struct arinc_semaphore);
    arinc_events = ARINC_ALLOC_ARRAY(arinc_config_nevents, struct arinc_event);
}
