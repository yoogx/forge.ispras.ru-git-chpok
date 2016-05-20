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

#include <types.h>
#include <errno.h>
#include <libc.h>
#include <bsp_common.h>
#include <assert.h>

#include <arch.h>

#include <arch/x86/interrupt.h>

#include "gdt.h"
#include "tss.h"

#include "space.h"
#include "core/sched.h"
#include "core/partition.h"

#define KERNEL_STACK_SIZE 8192

uint8_t current_space_id = (uint8_t)(-1);

/*
 * Arguments of this function must match the layout 
 * of space_context_t
 */
static void pok_dispatch_space(
         uint8_t space_id,
         uint32_t user_pc,
         uint32_t user_sp,
         uint32_t kernel_sp,
         uint32_t arg1,
         uint32_t arg2)
{
   interrupt_frame   ctx;
   uint32_t          code_sel;
   uint32_t          data_sel;
   uint32_t          sp;

   assert(space_id < POK_CONFIG_NB_PARTITIONS); //TODO: fix comparision

   code_sel = GDT_BUILD_SELECTOR (GDT_PARTITION_CODE_SEGMENT (space_id), 0, 3);
   data_sel = GDT_BUILD_SELECTOR (GDT_PARTITION_DATA_SEGMENT (space_id), 0, 3);

   sp = (uint32_t) &ctx;

   memset (&ctx, 0, sizeof (interrupt_frame));

   pok_arch_preempt_disable ();

   ctx.es = ctx.ds = ctx.ss = data_sel;

   ctx.__esp   = (uint32_t) (&ctx.error); /* for pusha */
   ctx.eip     = user_pc;
   ctx.eax     = arg1;
   ctx.ebx     = arg2;
   ctx.cs      = code_sel;
   ctx.eflags  = 1 << 9 | 3<<12;
   ctx.esp     = user_sp;

   tss_set_esp0 (kernel_sp);

   asm ("mov %0, %%esp		\n"
        "pop %%es		\n"
        "pop %%ds		\n"
        "popa			\n"
        "addl $4, %%esp		\n"
        "iret			\n"
        :
        : "m" (sp)
       );
}

pok_ret_t pok_create_space (uint8_t space_id,
                            uint32_t addr,
                            uint32_t size)
{
   gdt_set_segment (GDT_PARTITION_CODE_SEGMENT (space_id),
         addr, size, GDTE_CODE, 3);

   gdt_set_segment (GDT_PARTITION_DATA_SEGMENT (space_id),
         addr, size, GDTE_DATA, 3);

   return (POK_ERRNO_OK);
}

pok_ret_t pok_space_switch (uint8_t space_id)
{
    if(current_space_id != (uint8_t)(-1)) {
        gdt_disable (GDT_PARTITION_CODE_SEGMENT(current_space_id));
        gdt_disable (GDT_PARTITION_DATA_SEGMENT(current_space_id));
    }
    gdt_enable (GDT_PARTITION_CODE_SEGMENT(space_id));
    gdt_enable (GDT_PARTITION_DATA_SEGMENT(space_id));

    current_space_id = space_id;

    return (POK_ERRNO_OK);
}

uint32_t	pok_space_base_vaddr (uint32_t addr)
{
   (void) addr;
   return (0);
}

static void 
pok_space_context_init(
        uint32_t stack_addr, /*should be `sp`, but it alredy used in code. */
        uint8_t space_id,
        uintptr_t entry_rel,
        uintptr_t stack_rel,
        uint32_t arg1,
        uint32_t arg2)
{
    space_context_t *sp = (space_context_t*)(stack_addr - sizeof(space_context_t) - 4);
    memset (sp, 0, sizeof(*sp));
    
    sp->ctx.__esp  = (uint32_t)(&sp->ctx.eip); /* for pusha */
    sp->ctx.eip    = (uint32_t)pok_dispatch_space;
    sp->ctx.cs     = GDT_CORE_CODE_SEGMENT << 3;
    sp->ctx.eflags = 1 << 9 | 3<<12;
    
    sp->arg1          = arg1;
    sp->arg2          = arg2;
    sp->kernel_sp     = (uint32_t)sp;
    sp->user_sp       = stack_rel;
    sp->user_pc       = entry_rel;
    sp->partition_id  = space_id;
}

uint32_t pok_space_context_create (
        uint8_t  space_id,
        uint32_t entry_rel,
        uint32_t stack_rel,
        uint32_t arg1,
        uint32_t arg2)
{
   char*             stack_addr;
   space_context_t*  sp;
   
   stack_addr = pok_bsp_mem_alloc (KERNEL_STACK_SIZE);

   sp = (space_context_t *)
      (stack_addr + KERNEL_STACK_SIZE - 4 - sizeof (space_context_t));

   pok_space_context_init(sp, space_id, entry_rel, stack_rel, arg1, arg2);

   return ((uint32_t) sp);
}

void pok_space_context_restart(
        uint32_t sp, 
        uint8_t  space_id,
        uint32_t entry_rel,
        uint32_t stack_rel,
        uint32_t arg1,
        uint32_t arg2)
{
    // it's the same sp that was 
    // returned by pok_space_context_create earlier
    // 
    // we don't need to allocate anything here, we only have to 
    // reset some values

    pok_space_context_init(
        (space_context_t*)sp, 
        space_id,
        entry_rel,
        stack_rel,
        arg1,
        arg2
    );
}

//Double check here because these function are called not only in syscall
//(where there is checking), but also inside kernel
//TODO: maybe rename to pok_arch_?
uintptr_t pok_virt_to_phys(uintptr_t virt) {
    if (POK_CHECK_PTR_IN_PARTITION(pok_current_partition, virt)) {
        printf("pok_virt_to_phys: wrong virtual address %p\n", (void*)virt);
        pok_fatal("wrong pointer in pok_virt_to_phys\n");
    }
    return virt + pok_partitions[pok_current_partition].base_addr;
}

uintptr_t pok_phys_to_virt(uintptr_t phys) {
    uintptr_t virt = phys - pok_partitions[pok_current_partition].base_addr;

    if (POK_CHECK_PTR_IN_PARTITION(pok_current_partition, virt)) {
        printf("pok_phys_to_virt: wrong virtual address %p\n", (void*)virt);
        pok_fatal("wrong pointer in pok_phys_to_virt\n");
    }
    return virt;
}
