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

#include <config.h>

#ifdef POK_NEEDS_NETWORKING_VIRTIO

#include "virtio_virtqueue.h"

#include <bsp.h>
#include <libc.h>

void* virtio_virtqueue_setup(struct virtio_virtqueue *vq, uint16_t size, size_t alignment)
{
    size_t mem_size = vring_size(size, alignment);

    void *mem = pok_bsp_mem_alloc_aligned(mem_size, alignment);
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

    vq->callbacks = NULL;

    return mem;
}

void virtio_virtqueue_allocate_callbacks(struct virtio_virtqueue *vq)
{
    vq->callbacks = pok_bsp_mem_alloc(sizeof(vq->callbacks[0]) * vq->vring.num);
}

#endif // POK_NEEDS_NETWORKING
