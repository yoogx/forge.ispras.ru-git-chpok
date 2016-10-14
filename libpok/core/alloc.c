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

void* aligned_alloc(size_t size, size_t alignment)
{
    assert_os(kshd.partition_mode != POK_PARTITION_MODE_NORMAL);
    
    char* obj_start = ALIGN_PTR(heap_current, alignment);
    char* obj_end = obj_start + size;
    
    assert_os(obj_end <= kshd.heap_end);
    
    heap_current = obj_end;
    
    return obj_start;
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
