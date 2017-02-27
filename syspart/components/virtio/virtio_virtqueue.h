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

#ifndef __VIRTIO_VIRTQUEUE_H__
#define __VIRTIO_VIRTQUEUE_H__

#include "virtio_ring.h"
#include <smalloc.h>

struct virtio_virtqueue {
    struct vring vring;

    // index of first free descriptor
    uint16_t free_index;

    // count of free descriptors
    uint16_t num_free;

    // last seen used
    uint16_t last_seen_used;
};

void* virtio_virtqueue_setup(struct jet_sallocator *allocator, struct virtio_virtqueue *vq, uint16_t size, size_t alignment);

#endif
