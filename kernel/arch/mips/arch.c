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
#include "bsp/bsp.h"
#include <asp/bsp_common.h>
#include <core/uaccess.h>
#include "timer.h"
#include "mmu.h"

/**
 * Function that initializes architecture concerns.
 *
 * Called from entry.S.
 */
void pok_arch_init (void)
{

  ja_bsp_init();

  pok_arch_space_init();

  ja_time_init();
}

void ja_preempt_disable(void)
{
    INT_DISABLE

}

void ja_preempt_enable(void)
{
    mtsr(mfsr() | CP0_STATUS_IE);
    mtsr(mfsr() & (~CP0_STATUS_EXL));
    mtsr(mfsr() & (~CP0_STATUS_ERL));
}

pok_bool_t ja_preempt_enabled(void)
{
  return (mfsr() & CP0_STATUS_IE);
}

void ja_inf_loop(void)
{
   while (1)
   {
      asm("wait": : :"memory");
   }
}


#define DCFG_RSTCR 0xb0
#define RSTCR_RESET_REQ 0x2
void ja_cpu_reset(void)
{
    uintptr_t addr = pok_bsp.ccsrbar_base + pok_bsp.dcfg_offset + DCFG_RSTCR;
    *(uint32_t *) addr = RSTCR_RESET_REQ;
}

pok_ret_t pok_bsp_get_info(void * __user addr) {
    pok_bsp_t* __kuser k_addr = jet_user_to_kernel(addr, sizeof(pok_bsp_t));
    if(!k_addr) return POK_ERRNO_EFAULT;

    *k_addr = pok_bsp;

    return POK_ERRNO_OK;
}
