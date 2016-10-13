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

#include <asp/alloc.h>

/* Start of the memory heap. */
char libja_heap_start[{{part.get_intra_size()}}]
    __attribute__ ((aligned (16)));

/* Size of the memory heap. */
const size_t libja_heap_size = {{part.get_intra_size()}};

/* 
 * Alignment of the memory heap.
 * 
 * No allocation should use alignment which exceeds given one.
 */
const size_t libja_heap_alignment = 16;
