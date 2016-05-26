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

pok_ret_t pok_arch_init ()
{
  pok_gdt_init ();
  pok_event_init ();

  return (POK_ERRNO_OK);
}

pok_ret_t pok_arch_preempt_disable()
{
  asm ("cli");
  return (POK_ERRNO_OK);
}

pok_ret_t pok_arch_preempt_enable()
{
  asm ("sti");
  return (POK_ERRNO_OK);
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

pok_bool_t pok_arch_preempt_enabled(void)
{
  unsigned long flags = get_flags();
  return !!(flags & (1<<9));
}

void pok_arch_inf_loop()
{
   pok_arch_preempt_disable();
   while (1) {
      asm ("hlt");
   }
}

pok_ret_t pok_arch_idle()
{
   while (1)
   {
      asm ("hlt");
   }

   return (POK_ERRNO_OK);	
}

pok_ret_t pok_arch_event_register  (uint8_t vector,
                                    void (*handler)(void))
{
   pok_idt_set_gate (vector,
                     GDT_CORE_CODE_SEGMENT << 3,
                     (uint32_t)handler,
                     IDTE_INTERRUPT,
                     3);

   return (POK_ERRNO_OK);
}

uint32_t    pok_thread_stack_addr   (uint8_t    space_id,
                                     uint32_t stack_size,
                                     uint32_t* state)
{
   uint32_t result = spaces[space_id].size - 4 - (*state);
   //TODO: Check boundaries
   *state += stack_size;
   
   return result;
}

#include <ioports.h>
void pok_arch_cpu_reset()
{
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    pok_arch_idle();
}
