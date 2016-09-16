
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
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

#include <config.h>

#ifdef POK_NEEDS_DEBUG

#include <asp/arch.h>
#include <errno.h>
#include <core/debug.h>
#include <cons.h>
#include <core/sched.h>
#include <core/thread.h>
#include <core/partition.h>

int debug_strlen (const char* str)
{
  int		i = 0;

  while (*str != '\0')
  {
    str++;
    i++;
  }
  return i;
}

void pok_debug_print_current_state ()
{
   /*uint32_t i;
   printf ("\nCurrent state\n");
   printf ("-------------\n");
   printf ("Kernel thread        : %d\n", KERNEL_THREAD);
   printf ("Idle thread          : %d\n", IDLE_THREAD);
#ifdef POK_NEEDS_PARTITIONS
   printf ("Current partition    : %d\n", POK_SCHED_CURRENT_PARTITION);
   printf ("Thread index         : %lu\n",   (unsigned long) POK_CURRENT_PARTITION.thread_index);
   printf ("Thread low           : %lu\n",   (unsigned long) POK_CURRENT_PARTITION.thread_index_low);
   printf ("Thread high          : %lu\n",   (unsigned long) POK_CURRENT_PARTITION.thread_index_high);
   printf ("Thread capacity      : %lu\n",   (unsigned long) POK_CURRENT_PARTITION.nthreads);
   printf ("Base addr            : 0x%lx\n", (unsigned long) POK_CURRENT_PARTITION.base_addr);
   printf ("Base vaddr           : 0x%lx\n", (unsigned long) POK_CURRENT_PARTITION.base_vaddr);
   printf ("Size                 : %lu\n", (unsigned long) POK_CURRENT_PARTITION.size);
   printf ("Current thread       : %lu\n", (unsigned long) POK_CURRENT_PARTITION.current_thread);
   printf ("Prev current thread  : %lu\n", (unsigned long) POK_CURRENT_PARTITION.prev_thread);
   printf ("Main thread          : %u\n", POK_CURRENT_PARTITION.thread_main);
   printf ("Main thread entry    : 0x%lx\n", (unsigned long) POK_CURRENT_PARTITION.thread_main_entry);
   printf ("Partition threads sp :");
   for (i = POK_CURRENT_PARTITION.thread_index_low ; i < POK_CURRENT_PARTITION.thread_index_low + POK_CURRENT_PARTITION.thread_index ; i++)
   {
      printf (" 0x%lx", (unsigned long) pok_threads[i].sp);
   }
   printf ("\n");
   printf ("-------------\n");
#endif
   printf ("Current thread    : %d\n", POK_SCHED_CURRENT_THREAD);
   printf ("Period            : %lld\n", (long long) POK_CURRENT_THREAD.period);
   printf ("Deadline          : %d\n", POK_CURRENT_THREAD.deadline);
   printf ("Partition         : %d\n", POK_CURRENT_THREAD.partition);
   printf ("sp                : 0x%lx\n", (unsigned long) POK_CURRENT_THREAD.sp);
   printf ("init_stack_addr   : 0x%lx\n", (unsigned long) POK_CURRENT_THREAD.init_stack_addr);
   printf ("entry             : 0x%p\n", POK_CURRENT_THREAD.entry);*/
}

void pok_fatal (const char* message)
{
  // it can be called from a kernel thread
  // with preemption enabled
  // also, some bugs somehow reenable preemption
  // where it shouldn't be enabled
  ja_preempt_disable();

  jet_console_write ("FATAL ERROR: \n", 13);
  jet_console_write (message , debug_strlen(message));

  POK_DEBUG_PRINT_CURRENT_STATE
  ja_inf_loop();

  __builtin_unreachable();
}

void hexdump (const void *addr, int len)
{
    int i;
    unsigned char buff[17];
    const unsigned char *pc = (const unsigned char*)addr;

    if (len == 0) {
        printf("Len is zero\n");
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
}

#endif /* POK_CONFIG_NEEDS_DEBUG */
