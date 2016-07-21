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

extern char _end[];
static char *heap_end = _end;

void *ja_mem_alloc_aligned (size_t size, unsigned int alignment)
{
  char *res;
  uintptr_t mask = alignment - 1;

  res = (char *)(((unsigned int)heap_end + mask) & ~mask);
  heap_end = res + size;
  return res;
}

void *ja_mem_alloc (size_t size)
{
  unsigned int alignment;
  if(size == 1)
  {
    alignment = 1;
  }
  else if(size < 4)
  {
    alignment = 2;
  }
  else if(size < 8)
  {
    alignment = 4;
  }
  else
  {
    alignment = 8;
  }
  
  return ja_mem_alloc_aligned(size, alignment);
}
