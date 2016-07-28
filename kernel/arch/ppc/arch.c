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

/**
 * Function that initializes architecture concerns.
 * 
 * Called from entry.S.
 */
void pok_arch_init (void)
{
  mtmsr(MSR_IP | MSR_FP);

#if POK_NEEDS_PARTITIONS
  pok_arch_space_init();
#endif
}

void ja_preempt_disable(void)
{
  mtmsr(mfmsr() & ~MSR_EE);
}

void ja_preempt_enable(void)
{
  mtmsr(mfmsr() | MSR_EE);
}

pok_bool_t ja_preempt_enabled(void)
{
  return !!(mfmsr() & MSR_EE);
}

void ja_inf_loop(void)
{
   while (1)
   {
      asm("wait": : :"memory");
   }
}

#include <arch/linux_io.h>
#define DCFG_RSTCR 0xb0
#define RSTCR_RESET_REQ 0x2
void ja_cpu_reset(void)
{
    uintptr_t addr = pok_bsp.ccsrbar_base + pok_bsp.dcfg_offset + DCFG_RSTCR;
    out_be32((void*)addr, RSTCR_RESET_REQ);
}
