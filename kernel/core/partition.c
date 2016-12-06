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
#include <alloc.h>

#ifdef POK_NEEDS_MONITOR
extern pok_partition_t partition_monitor;
#endif /* POK_NEEDS_MONITOR */

#ifdef POK_NEEDS_GDB
extern pok_partition_t partition_gdb;
#endif /* POK_NEEDS_GDB */

void pok_partition_init(pok_partition_t* part)
{
   if(part->partition_event_max != 0)
   {
      /*
       * DEV: Allocate one element more than required.
       * 
       * This simplifies implementation of ring buffer, when
       * extracting messages may be concurrent with adding them.
       */ 
      part->partition_events = ja_mem_alloc_aligned(
         sizeof(*part->partition_events) * (part->partition_event_max + 1),
         __alignof__(*part->partition_events));
   }
   else
   {
      part->partition_events = NULL;
   }
}

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

/* 
 * Add event to partition's queue and notify it if needed.
 * 
 * Should be called with global preemption disabled.
 */
void pok_partition_add_event(pok_partition_t* part,
    enum jet_partition_event_type event_type,
    uint16_t handler_id)
{
   // Whether it is needed to set is_event flag for partition.
   pok_bool_t set_event =
      (part->partition_event_end == part->partition_event_begin);
   /* 
    * TODO: This is a result of configuration error, when partition
    * doesn't expect events at all.
    */
   assert(part->partition_event_max != 0);

   struct jet_partition_event* partition_event = part->partition_events
      + part->partition_event_end;

   partition_event->handler_id = handler_id;
   partition_event->event_type = event_type;

   part->partition_event_end++;
   if(part->partition_event_end > part->partition_event_max)
      part->partition_event_end = 0;

   /*
    * TODO: This is a result of configuration error, when partition
    * doesn't expect so much events pending.
    */
   assert(part->partition_event_end != part->partition_event_begin);

   if(set_event) {
      part->is_event = TRUE;
   }
}


/* 
 * Consume event from current partition's queue.
 * 
 * Return TRUE on success, FALSE if the queue is empty.
 * 
 * Should be called with local preemption disabled.
 */
pok_bool_t pok_partition_get_event(struct jet_partition_event* event)
{
   pok_partition_t* part = current_partition;

   uint16_t partition_event_begin = part->partition_event_begin;
   uint16_t partition_event_end = ACCESS_ONCE(part->partition_event_end);

   if(partition_event_end == partition_event_begin) return FALSE;

   *event = ACCESS_ONCE(part->partition_events[partition_event_begin]);

   ACCESS_ONCE(part->partition_event_begin) =
      (partition_event_begin == part->partition_event_max) ?
         0: partition_event_begin + 1;

   return TRUE;
}

void pok_partition_set_timer(pok_partition_t* part,
    pok_time_t timer_new)
{
     part->timer = timer_new;
}


static void kernel_thread_on_event(void)
{
    //Shouldn't be call
    unreachable();
}

#ifdef POK_NEEDS_GDB

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

static void kernel_thread_get_thread_info(pok_partition_t* part,
   int index, void* private,
   print_cb_t print_cb, void* cb_data)
{
   (void)part;
   (void)private;
   (void)index;

   static const char* kernel_thread_name = "kernel_thread";

   print_cb(kernel_thread_name, strlen(kernel_thread_name), cb_data);
}

#endif /* POK_NEEDS_GDB */

const struct pok_partition_sched_operations partition_sched_ops_kernel =
{
   .on_event = &kernel_thread_on_event,
#ifdef POK_NEEDS_GDB
   .get_number_of_threads = &kernel_thread_get_number_of_threads,
   .get_current_thread_index = &kernel_thread_get_current_thread_index,
   .get_thread_at_index = &kernel_thread_get_thread_at_index,
   .get_thread_info = &kernel_thread_get_thread_info
#endif /* POK_NEEDS_GDB */
};
