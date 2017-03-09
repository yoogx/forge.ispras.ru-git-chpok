
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

#include <stdint.h>
#include <libc.h>
#include "mmu.h"


extern uint32_t __KERNBASE_PADDR; //should be aligned by 1MB
extern uint32_t __KERNBASE_VADDR; //should be aligned by 1MB

//uintptr_t KERNBASE_PADDR = (uint32_t)&__KERNBASE_PADDR;
//uintptr_t KERNBASE_VADDR = (uint32_t)&__KERNBASE_VADDR;

#define KERNBASE_PADDR 0x10000000
#define KERNBASE_VADDR 0xc0000000

 //aligned by 16K
__attribute__ ((aligned(0x4000))) uint32_t entry_l1_table[4096] = {

    [0] = ((KERNBASE_PADDR + (1<<20))&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_NORMAL_CACHEABLE | L1_TYPE_SECT,

    [KERNBASE_PADDR>>20] = (KERNBASE_PADDR&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_NORMAL_CACHEABLE | L1_TYPE_SECT,

    [KERNBASE_VADDR>>20] = (KERNBASE_PADDR&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_NORMAL_CACHEABLE | L1_TYPE_SECT,

    [0x2020000>>20] = (0x2020000&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_DEVICE | L1_TYPE_SECT, //UART

    [0xa00000>>20] = (0xa00000&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_DEVICE | L1_TYPE_SECT, //SCU
};





static inline uint32_t cp15_get_SCTLR (void)
{
    uint32_t rval;
    asm volatile("mrc p15, 0, %0, c1, c0, 0"
            : "=r" (rval)
            :: "memory");
    return rval;
}

static inline void cp15_set_SCTLR (uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c1, c0, 0"
            : : "r" (val)
            : "memory");
}

static inline void load_l1_table(uint32_t *l1_table)
{
    // Write l1_table addr to TTBR0
    asm("mcr p15, 0, %0, c2, c0, 0"
            :
            :"r"(l1_table)
            :"memory");
}

extern char __vector_table_start[];
extern char __vector_table_end[];



void mmu_enable(void)
{
    memcpy(0, __vector_table_start, __vector_table_end - __vector_table_start);
    printf("copy vector table to %p, from %p, size (0x%x)\n", NULL, __vector_table_start, __vector_table_end - __vector_table_start);

/*
    printf("access to wrong memory\n");
    int *a = (void *) 0x30000000;
    *a = 1; // throw 'data abort' exception
    */
}

/*
 * TODO add interface
 * map_page(page_table_ptr, page_vaddr, page_paddr, page_size(enum 4K, 16K, 1M, 16M), flags)
 * use ja_mem_alloc_aligned for allocation of L2 tables
 *
 */
