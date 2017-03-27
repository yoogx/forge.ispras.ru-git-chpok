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

#include <errno.h>
#include <core/partition.h>

#include "event.h"
#include "gdt.h"
#include "space.h"
#include <bsp/bsp.h>
#include <asp/entries.h>

void pok_arch_init (void)
{
    jet_console_init_all();
    pok_gdt_init ();
    ja_event_init ();
    ja_bsp_init();
    ja_space_init();
}

void ja_preempt_disable(void)
{
  asm ("cli");
}

void ja_preempt_enable(void)
{
  asm ("sti");
}

static unsigned long get_flags(void)
{
  unsigned long ret;

  asm volatile(
    "pushf\n"
    "pop %0"
    : "=rm" (ret)
    :
    : "memory"
  );

  return ret;
}

pok_bool_t ja_preempt_enabled(void)
{
  unsigned long flags = get_flags();
  return !!(flags & (1<<9));
}

void ja_inf_loop(void)
{
   while (1)
   {
      asm ("hlt");
   }
}

#include <ioports.h>
void ja_cpu_reset(void)
{
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    ja_inf_loop();
}
