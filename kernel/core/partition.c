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

#include <config.h>
#include <core/partition_arinc.h>

#ifdef POK_NEEDS_MONITOR
extern pok_partition_t partition_monitor;
#endif /* POK_NEEDS_MONITOR */

#ifdef POK_NEEDS_GDB
extern pok_partition_t partition_gdb;
#endif /* POK_NEEDS_GDB */


void for_each_partition(void (*f)(pok_partition_t* part))
{
#ifdef POK_NEEDS_MONITOR
   f(&partition_monitor);
#endif /* POK_NEEDS_MONITOR */

#ifdef POK_NEEDS_GDB
   f(&partition_gdb);
#endif /* POK_NEEDS_GDB */


   for(int i = 0; i < pok_partitions_arinc_n; i++)
   {
      f(&pok_partitions_arinc[i].base_part);
   }
}

static void kernel_thread_on_event(void)
{
    //Shouldn't be call
    unreachable();
}

static int kernel_thread_get_number_of_threads(pok_partition_t* part)
{
   (void)part;
   return 1;
}

static int kernel_thread_get_current_thread_index(pok_partition_t* part)
{
   (void)part;
   return 0;
}

static int kernel_thread_get_thread_at_index(pok_partition_t* part,
     int index, void** private)
{
   (void)part;
   (void)private;
   return index == 0? 0 : 1;
}

static size_t kernel_thread_get_thread_info(pok_partition_t* part,
   int index, void* private,
   char* buf, size_t size)
{
   (void)part;
   (void)private;
   (void)index;

   static const char* kernel_thread_name = "kernel_thread";
   int len = strlen(kernel_thread_name);

   memcpy(buf, kernel_thread_name, len);
   return len;
}

const struct pok_partition_sched_operations partition_sched_ops_kernel =
{
   .on_event = &kernel_thread_on_event,
   .get_number_of_threads = &kernel_thread_get_number_of_threads,
   .get_current_thread_index = &kernel_thread_get_current_thread_index,
   .get_thread_at_index = &kernel_thread_get_thread_at_index,
   .get_thread_info = &kernel_thread_get_thread_info
};
