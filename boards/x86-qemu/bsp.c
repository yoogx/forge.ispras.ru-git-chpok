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
#include <arch.h>

#include "cons.h"
#include "pm.h"
#include "pit.h"
#include "pic.h"

#include <assert.h>

#include <bsp_common.h>

#define ALIGN_UP(boundary, val) \
	(val + (boundary - 1)) & (~(boundary - 1))

int pok_bsp_init (void)
{
   pok_cons_init ();
   pok_pm_init ();
   pok_pic_init ();

   return (POK_ERRNO_OK);
}

pok_ret_t pok_bsp_irq_acknowledge (uint8_t irq)
{
   pok_pic_eoi (irq);

   return (POK_ERRNO_OK);
}

pok_ret_t pok_bsp_irq_register (uint8_t   irq,
                                void      (*handler)(void))
{
   pok_pic_unmask (irq);

   pok_arch_event_register (32 + irq, handler);

   return (POK_ERRNO_OK);
}

/**
 * Allocate data. At this time, the pok_pm_sbrk function
 * only increment size each time we allocate memory
 * and was not designed to free previously allocated
 * memory.
 */
void *pok_bsp_mem_alloc (size_t size)
{
   return ((void *)pok_pm_sbrk(size));
}

void  *pok_bsp_mem_alloc_aligned(size_t size, size_t alignment)
{
   uintptr_t original_brk = (uintptr_t) pok_pm_sbrk(0);
   uintptr_t aligned_brk = ALIGN_UP(alignment, original_brk); 
   uintptr_t new_brk = pok_pm_sbrk(size + (aligned_brk - original_brk));

   assert(original_brk == new_brk);
   assert(aligned_brk - new_brk < alignment);

   return (void *) aligned_brk;
}

void *pok_bsp_alloc_partition(size_t size)
{
    return pok_bsp_mem_alloc_aligned(size, 0x1000);
}

/**
 * Init time. \a freq is the frequency
 * of the oscillator.
 */
pok_ret_t pok_bsp_time_init ()
{
   return (pok_x86_qemu_timer_init ());
}

void pok_bsp_get_info(void *addr) {
    pok_fatal("pok_bsp_get_info unimplemented on x86");
}


