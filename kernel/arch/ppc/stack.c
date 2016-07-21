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

#include <asp/stack.h>
#include <asp/alloc.h>

jet_stack_t pok_stack_alloc(uint32_t stack_size)
{
    /* 
     * Align stack size.
     * 
     * This is needed because we need aligned stack head, not only stack
     * tail.
     */
    const unsigned int alignment = 16;
    const unsigned int mask = alignment - 1;
    uint32_t stack_size_real = (stack_size + mask) & ~mask;
    
    char* stack_tail = ja_mem_alloc_aligned(stack_size_real, alignment);
    char* stack_head = stack_tail + stack_size_real;
    
    return (uint32_t)stack_head;
}
