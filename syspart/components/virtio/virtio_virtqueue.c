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

#include "virtio_virtqueue.h"

#include <smalloc.h>
#include <string.h>
#include <stdio.h>

void* virtio_virtqueue_setup(struct jet_sallocator *allocator, struct virtio_virtqueue *vq, uint16_t size, size_t alignment)
{
    size_t mem_size = vring_size(size, alignment);

    void *mem = jet_sallocator_alloc_aligned(allocator, mem_size, alignment);
    if (mem == NULL) {
        printf("Virtio: heap alloc return zero (not enough memory)\n");
        return NULL;
    }
    memset(mem, 0, mem_size);

    vring_init(&vq->vring, size, mem, alignment);

    vq->free_index = 0;
    vq->num_free = size;
    vq->last_seen_used = 0;

    // establish linked list
    int i;
    for (i = 0; i < size - 1; i++) {
        vq->vring.desc[i].next = i + 1;
    }

    return mem;
}
