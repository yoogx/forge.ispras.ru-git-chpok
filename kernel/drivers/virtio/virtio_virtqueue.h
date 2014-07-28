/*  
 *  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef POK_NEEDS_NETWORKING

#ifndef __POK_KERNEL_VIRTIO_VIRTQUEUE_H__
#define __POK_KERNEL_VIRTIO_VIRTQUEUE_H__

#include <net/network.h>

#include "virtio_ring.h"

struct virtio_virtqueue {
    struct vring vring;

    // index of first free descriptor
    uint16_t free_index;

    // count of free descriptors
    uint16_t num_free;

    // last seen used
    uint16_t last_seen_used;

    struct {
        pok_network_buffer_callback_t callback;
        void *callback_arg;
    } *callbacks;
};

void* virtio_virtqueue_setup(struct virtio_virtqueue *vq, uint16_t size, size_t alignment);

void virtio_virtqueue_allocate_callbacks(struct virtio_virtqueue *vq);

#endif // __POK_KERNEL_VIRTIO_VIRTQUEUE_H__
#endif // POK_NEEDS_NETWORKING
