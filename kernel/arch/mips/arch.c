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

#include <types.h>
#include <errno.h>
#include <core/partition.h>
#include "reg.h"
#include "msr.h"
#include "space.h"
#include "devtree.h"
#include <bsp_common.h>

pok_ret_t pok_arch_init ()
{
 /*MSR_IP |*/   mtsr(Status_XX);

#if POK_NEEDS_PARTITIONS
  pok_arch_space_init();
#endif

  return (POK_ERRNO_OK);
}

pok_ret_t pok_arch_preempt_disable()
{

  asm("DI");

  return (POK_ERRNO_OK);
}

pok_ret_t pok_arch_preempt_enable()
{

  asm("EI");

  return (POK_ERRNO_OK);
}

pok_bool_t pok_arch_preempt_enabled(void)
{
  return !!(mfsr() & Status_IE);
}

void pok_arch_inf_loop()
{
   pok_arch_preempt_disable();

   while (1)
   {}
}

pok_ret_t pok_arch_idle()
{
   pok_arch_preempt_enable();

   while (1)
   {
   }

   return (POK_ERRNO_OK);	
}

pok_ret_t pok_arch_event_register (uint8_t vector, void (*handler)(void))
{
  (void) vector;
  (void) handler;

  return (POK_ERRNO_OK);
}


uint32_t    ja_thread_stack_addr   (uint8_t    space_id,
                                     uint32_t stack_size,
                                     uint32_t* state)
{
   uint32_t result = POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE - 16 - (*state);
   // TODO: Check boundaries.
   (void) space_id;
   *state += stack_size;
   return result;
}


#include <arch/linux_io.h>
#define DCFG_RSTCR 0xb0
#define RSTCR_RESET_REQ 0x2
void pok_arch_cpu_reset()
{
    uintptr_t addr = pok_bsp.ccsrbar_base + pok_bsp.dcfg_offset + DCFG_RSTCR;
    out_be32((void*)addr, RSTCR_RESET_REQ);
}
