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
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */
#include <types.h>
#include <stdio.h>

#include <mem.h>
extern char dynamic_memory[];
extern unsigned dynamic_memory_size;


static char * end = NULL;

void *smalloc_aligned(size_t sz, size_t alignment)
{
    char *res;
    if (!end)
        end = dynamic_memory;

    res = (char *) ALIGN_UP((uintptr_t)end, alignment);
    end = res + sz;
    if ((uintptr_t)end > (uintptr_t)dynamic_memory + dynamic_memory_size) {
        printf("Needs more memory!\n");
        return NULL;
    }
    return res;
}

void *smalloc(size_t sz)
{
    //in PPC every memory access must be 4-byte aligned
    return smalloc_aligned(sz, 4);
}

