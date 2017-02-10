
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

uint32_t l1_table[4096] __attribute__ ((aligned(0x4000))); //align by 16K

extern uint32_t __KERNBASE_PADDR; //should be aligned by 1MB
extern uint32_t __KERNBASE_VADDR; //should be aligned by 1MB

uintptr_t KERNBASE_PADDR = (uint32_t)&__KERNBASE_PADDR;
uintptr_t KERNBASE_VADDR = (uint32_t)&__KERNBASE_VADDR;


#define L1_TYPE_FAULT (0)
#define L1_TYPE_TABLE (1)
#define L1_TYPE_SECT  (2)
#define L1_TYPE_MASK  (3)

#define L1_SECT_B      ( 1 << 2)   // bufferable
#define L1_SECT_C      ( 1 << 3)   // cacheable
#define L1_SECT_XN     ( 1 << 4)   // execute never
#define L1_SECT_AP(x)  ((x) << 10) // access permission
#define L1_SECT_TEX(x) ((x) << 12) // type extension
#define L1_SECT_APX    ( 1 << 15)
#define L1_SECT_S      ( 1 << 16)  // shareable
#define L1_SECT_nG     ( 1 << 17)  // non global
#define L1_SECT_SUPER  ( 1 << 18)  // supersection

#define L1_SECT_PRIVILEGED_RW (L1_SECT_AP(1))
#define L1_SECT_MEM_NORMAL_CACHEABLE (L1_SECT_TEX(0) | L1_SECT_C | L1_SECT_B)

#define L1_SECT_MEM_DEVICE (L1_SECT_TEX(2))


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

void mmu_enable(void)
{
    l1_table[KERNBASE_VADDR>>20] = (KERNBASE_PADDR&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_NORMAL_CACHEABLE | L1_TYPE_SECT;

    l1_table[0x2020000>>20] = (0x2020000&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_DEVICE | L1_TYPE_SECT;

    // Write 0 to TTBCR
    asm("mcr p15, 0, %0, c2, c0, 2"
            :
            :"r"(0)
            :"memory");
    // Write l1_table addr to TTBR0
    asm("mcr p15, 0, %0, c2, c0, 0"
            :
            :"r"(l1_table)
            :"memory");
    uint32_t sctlr = cp15_get_SCTLR();
    printf("sctlr val = %lx\n", sctlr);
    cp15_set_SCTLR(sctlr|1);

}

/*
 * TODO add interface
 * map_page(page_table_ptr, page_vaddr, page_paddr, page_size(enum 4K, 16K, 1M, 16M), flags)
 * use ja_mem_alloc_aligned for allocation of L2 tables
 *
 */
