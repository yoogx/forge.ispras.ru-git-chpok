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
#include <assert.h>

#include "interrupt.h"
#include <asp/arch.h>
#include <arch/deployment.h>

#include "gdt.h"
#include "tss.h"

#include "space.h"
#include <core/sched.h>

#include <asp/alloc.h>
#include <arch/memlayout.h>
#include <arch/mmu.h>
#include "regs.h"

#include "kernel_pgdir.h"
#include <alloc.h>

#define KERNEL_STACK_SIZE 8192

size_t ja_ustack_get_alignment(void)
{
    return 16;
}

/*
 * All segments has RWX access. So kernel may write to them at any time.
 */
void ja_uspace_grant_access(void) {}
void ja_uspace_revoke_access(void) {}

void ja_uspace_grant_access_local(jet_space_id space_id) {(void)space_id;}
void ja_uspace_revoke_access_local(jet_space_id space_id) {(void)space_id;}


jet_space_id current_space_id = 0;

void ja_user_space_jump(
    jet_stack_t stack_kernel,
    jet_space_id space_id,
    void (__user * entry_user)(void),
    uintptr_t stack_user)
{
    assert(space_id > 0);
    assert(space_id <= ja_partitions_pages_nb);

    /*
     * Reuse layout of interrupt_frame structure, allocated on stack,
     * for own purposes.
     *
     * Usage of this structure here is unrelated to interrupts
     * because it is allocated not at the *beginning* of the stack.
     */
   interrupt_frame   ctx;
   uint32_t          code_sel;
   uint32_t          data_sel;
   uint32_t          sp;

   code_sel = GDT_BUILD_SELECTOR(GDT_PARTITION_CODE_SEGMENT, 0, 3);
   data_sel = GDT_BUILD_SELECTOR(GDT_PARTITION_DATA_SEGMENT, 0, 3);

   sp = (uint32_t) &ctx;

   memset (&ctx, 0, sizeof (interrupt_frame));

   ctx.es = ctx.ds = ctx.ss = data_sel;

   // ctx.__esp   = (uint32_t) (&ctx.error); /* for pusha */
   ctx.eip     = (uint32_t)entry_user;
   //ctx.eax     = arg1;
   //ctx.ebx     = arg2;
   ctx.cs      = code_sel;
   ctx.eflags  = 1 << 9 | 3<<12;
   ctx.esp     = stack_user;

   tss_set_esp0 (stack_kernel);

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

/* insert page mapping to pgdir, allocate page table is needed */
static void pgdir_insert_page(uint32_t *pgdir, struct page *page)
{
    if (page->is_big) {
        //4MB page
        pgdir[PDX(page->vaddr)] = page->paddr_and_flags | PAGE_P | PAGE_U;
    } else {
        // 4KB page
        if (pgdir[PDX(page->vaddr)] != 0) {
            //already allocated page table
            uint32_t *pgtable = (uint32_t *)VIRT(PT_ADDR(pgdir[PDX(page->vaddr)]));
            pgtable[PTX(page->vaddr)] = page->paddr_and_flags | PAGE_P | PAGE_U;
        } else {
            //page table hasn't been created yet
            uint32_t *pgtable = ja_mem_alloc_aligned(PAGE_SIZE, PAGE_SIZE);
            memset(pgtable, 0, PAGE_SIZE);
            pgdir[PDX(page->vaddr)] = PHYS(pgtable) | PAGE_P | PAGE_RW| PAGE_U;

            pgtable[PTX(page->vaddr)] = page->paddr_and_flags | PAGE_P | PAGE_U;
        }
    }
}

uint32_t **pgdirs; //array of pointers to pgdirs

void ja_space_init(void)
{
    pgdirs = jet_mem_alloc(ja_partitions_pages_nb*sizeof(*pgdirs));

    for (unsigned i = 0; i < ja_partitions_pages_nb; i++) {
        uint32_t *pgdir = ja_mem_alloc_aligned(PAGE_SIZE, PAGE_SIZE);
        memset(pgdir, 0, PAGE_SIZE);
        pgdirs[i] = pgdir;

        /* user mapping */
        for (unsigned j = 0; j < ja_partitions_pages[i].len; j++) {
            pgdir_insert_page(pgdir, &ja_partitions_pages[i].pages[j]);
        }

        /* kernel mapping */
        pgdir_insert_kernel_mapping(pgdir);
    }
}

void ja_space_switch (jet_space_id space_id)
{
    if (space_id != 0) {
        asm volatile("movl %0,%%cr3" : : "r" (PHYS(pgdirs[space_id - 1])));
    }

    current_space_id = space_id;
}

jet_space_id ja_space_get_current (void)
{
    return current_space_id;
}

// TODO: Storage for floating point registers and operations with them.
struct jet_fp_store
{
  int todo;
};

/* 
 * Allocate place for store floating point registers.
 * 
 * May be called only during OS init.
 */
struct jet_fp_store* ja_alloc_fp_store(void)
{
    struct jet_fp_store* res = ja_mem_alloc_aligned(sizeof(*res), 4);

    return res;
}

/* Save floating point registers into given place. */
void ja_fp_save(struct jet_fp_store* fp_store)
{
    // TODO
    (void)fp_store;
}

/* Restore floating point registers into given place. */
void ja_fp_restore(struct jet_fp_store* fp_store)
{
    // TODO
    (void)fp_store;
}

/* Initialize floating point registers with zero. */
void ja_fp_init(void)
{
    // TODO
}
