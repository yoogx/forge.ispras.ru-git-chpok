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
#include <asp/stack.h>
#include <common.h>
#include <libc.h>
#include <core/debug.h>

extern char jet_heap_start[];
extern char jet_heap_end[];

static char *cur_heap_end = jet_heap_start;

void *ja_mem_alloc_aligned (size_t size, unsigned int alignment)
{
  char *res;

  res = (char *)(ALIGN_VAL((uintptr_t) cur_heap_end, alignment));
  cur_heap_end = res + size;

  if (cur_heap_end > jet_heap_end)
      pok_fatal("Not enough space in heap. Increase jet_heap_end in kernel.lds\n");
  return res;
}

unsigned int ja_mem_get_alignment (size_t size)
{
  unsigned int alignment = 1;
  if(size == 1)
  {
    alignment = 1;
  }
  else if(size < 4)
  {
    alignment = 2;
  }
  else {
    alignment = 4;
  }
  
  return alignment;
}

jet_stack_t pok_stack_alloc(uint32_t stack_size)
{
    /* 
     * Align stack size.
     * 
     * This is needed because we need aligned stack head, not only stack
     * tail.
     * 
     * DEV: For kernel stack it is assumed that alignment on 4 bytes is
     * sufficient.
     */
    const unsigned int alignment = 4;
    uint32_t stack_size_real = ALIGN_VAL(stack_size, alignment);
    
    char* stack_tail = ja_mem_alloc_aligned(stack_size_real, alignment);
    /*
     * Reserve area *above* stack and fill it with zeroes.
     * These bytes compose terminated stack frame.
     * 
     * See also comments for type 'jet_stack_t' on x86.
     */
    char* stack_head = stack_tail + stack_size_real - 2 * sizeof(uint32_t);
    memset(stack_head, 0, 2 * sizeof(uint32_t));
    
    return (uint32_t)stack_head;
}
