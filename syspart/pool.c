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
#include <pool.h>


//TODO use align function from libjet instead
#define ALIGN_UP(addr,size) (((addr)+((size)-1))&(~((size)-1)))

static struct pool_elem *get_pool_elem(struct pool *pool, int idx)
{
    return (struct pool_elem *) (&pool->data[idx * pool->stride]);
}

struct pool *jet_pool_create(size_t elem_size, int num)
{
    struct pool *pool;
    struct pool_elem *elem;

    pool = smalloc(sizeof(*pool));

    pool->elem_size = elem_size;
    pool->num = num;
    pool->stride = ALIGN_UP(pool->elem_size + sizeof(struct pool_elem), sizeof(unsigned long));
    pool->data = smalloc(pool->stride*num);
    pool->free_elem_idx = 0;

    for (int i = 0; i < num - 1; i++) {
       elem = get_pool_elem(pool, i);
       elem->next_free_idx = i + 1;
       elem->idx = i;
    }
    elem = get_pool_elem(pool, num - 1);
    elem->next_free_idx = -1;

    return pool;
}

struct pool_elem * jet_pool_get_free_elem(struct pool *pool)
{
    if (pool->free_elem_idx == -1)
        return NULL;

    struct pool_elem *elem = get_pool_elem(pool, pool->free_elem_idx);
    elem->is_free = 0;
    pool->free_elem_idx = elem->next_free_idx;
    return elem;
}

void jet_pool_free_elem(struct pool *pool, struct pool_elem *elem)
{
    elem->is_free = 1;
    elem->next_free_idx = pool->free_elem_idx;
    pool->free_elem_idx = elem->idx;
}

