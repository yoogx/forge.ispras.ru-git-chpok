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

#include <asp/space.h>
#include <alloc.h>
#include <arch/deployment.h>
#include <assert.h>
#include "regs.h"
#include <arch/memlayout.h>
#include <arch/mmu.h>

/*
 * Currently, kernel has rw access to all pages.
 * So switching entries is not required.
 */
void ja_uspace_grant_access(void) {}
void ja_uspace_revoke_access(void) {}

void ja_uspace_grant_access_local(jet_space_id space_id) {(void)space_id;}
void ja_uspace_revoke_access_local(jet_space_id space_id) {(void)space_id;}


jet_space_id current_space_id = 0;

jet_space_id ja_space_get_current (void)
{
    return current_space_id;
}

size_t ja_ustack_get_alignment(void)
{
    return 8;
}

uint32_t **l1_tables; //array of pointers to L1 table

void ja_space_switch(jet_space_id new_space_id)
{
    if (new_space_id != 0) {
        ttbr0_set(PHYS(l1_tables[new_space_id - 1]));
    }
    asm volatile ("MCR p15, 0, r0, c8, c7, 0"); // invalidate all TLB

    current_space_id = new_space_id;
}

/* insert page mapping to pgdir, allocate page table is needed */
static void l1_insert_page(uint32_t *l1_table, struct page *page)
{
    if (page->size == PAGE_SIZE_1M) {
        l1_table[L1_IDX(page->vaddr)] = page->paddr | page->flags | L1_TYPE_SECT;
    } else if (page->size == PAGE_SIZE_4K) {
        uint32_t *l2_table;
        uint32_t *l1_entry_ptr = &l1_table[L1_IDX(page->vaddr)];

        if (*l1_entry_ptr != 0) {
            //already allocated L2 table
            l2_table = (uint32_t *)VIRT(L2_ADDR(*l1_entry_ptr));
        } else {
            //page table hasn't been created yet. Create it
            l2_table  = ja_mem_alloc_aligned(L2_TABLE_SIZE, L2_TABLE_SIZE);
            memset(l2_table, 0, L2_TABLE_SIZE);
            *l1_entry_ptr = PHYS(l2_table) | L1_TYPE_TABLE;
        }

        l2_table[L2_IDX(page->vaddr)] = page->paddr | page->flags | L2_SECT_NON_SUPER;
    } else {
        //TODO
        assert(0);
    }
}

void space_init()
{
    l1_tables = jet_mem_alloc(ja_partitions_pages_nb*sizeof(*l1_tables));

    for (unsigned i = 0; i < ja_partitions_pages_nb; i++) {
        // create new L1 table
        uint32_t *l1_table = ja_mem_alloc_aligned(L1_TABLE_SIZE, L1_TABLE_SIZE);
        memset(l1_table, 0, L1_TABLE_SIZE);

        // user mapping
        for (unsigned j = 0; j < ja_partitions_pages[i].len; j++) {
            l1_insert_page(l1_table, &ja_partitions_pages[i].pages[j]);
        }

        // kernel mapping
        l1_insert_kernel_mapping(l1_table);

        // save created L1 table to L1_table_list
        l1_tables[i] = l1_table;
    }
}
