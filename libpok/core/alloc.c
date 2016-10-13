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

#include <alloc.h>
#include <kernel_shared_data.h>
#include <core/assert_os.h>
#include <libc/stdio.h>

static size_t heap_pos_current = 0;

void* aligned_alloc(size_t size, size_t alignment)
{
    assert_os(kshd.partition_mode != POK_PARTITION_MODE_NORMAL);
    assert_os(alignment <= libja_heap_alignment);
    
    size_t heap_pos_start = ALIGN_VAL(heap_pos_current, alignment);
    size_t heap_pos_end = heap_pos_start + size;
    
    assert_os(heap_pos_end <= libja_heap_size);
    
    heap_pos_current = heap_pos_end;
    
    return libja_heap_start + heap_pos_start;
}

/* 
 * Allocate memory region of given size and alignment, suitable for any
 * object of size less-or-equal than given one.
 * 
 * May be used only in init mode.
 */
void* malloc(size_t size)
{
    return aligned_alloc(size, libja_mem_get_alignment(size));
}
