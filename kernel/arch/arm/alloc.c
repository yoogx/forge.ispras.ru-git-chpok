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

#include <types.h>
#include <asp/alloc.h>
#include <asp/stack.h>
#include <asp/space.h>
#include <assert.h>
#include <common.h>

extern char jet_heap_start[];
extern char jet_heap_end[];

static char *cur_heap_end = jet_heap_start;

void* ja_mem_alloc_aligned(size_t size, unsigned int alignment)
{
  char *res;

  res = (char *)(ALIGN_VAL((uintptr_t) cur_heap_end, alignment));
  cur_heap_end = res + size;

  if (cur_heap_end > jet_heap_end)
      pok_fatal("Not enough space in heap. Increase jet_heap_end in kernel.lds\n");
  return res;
}

unsigned int ja_mem_get_alignment(size_t size)
{
    // copy ppc ja_mem_get_alignment logic
    if(size == 1)
        return 1;
    else if(size < 4)
        return 2;
    else if(size < 8)
        return 4;
    else
        return 8;
}

jet_stack_t pok_stack_alloc(uint32_t stack_size)
{
    size_t alignment = ja_ustack_get_alignment();
    uint32_t stack_size_real = ALIGN_VAL(stack_size, alignment);

    char* stack_tail = ja_mem_alloc_aligned(stack_size_real, alignment);
    char* stack_head = stack_tail + stack_size_real;

    return (uint32_t)stack_head;
}
