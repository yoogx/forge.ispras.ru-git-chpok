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
 */

#include <arch.h>
#include <asp/alloc.h>

size_t libja_mem_get_alignment(size_t obj_size)
{
   if(obj_size <= 1) {
      return 1;
   }
   else if(obj_size < 4) {
      return 2;
   }
   else if(obj_size < 8) {
      return 4;
   }
   else if(obj_size < 16) {
      return 8;
   }
   else {
      return 16;
   }
}
