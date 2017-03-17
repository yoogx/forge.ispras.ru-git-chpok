
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
#include <arch/mmu.h>
#include "memlayout.h"
#include "regs.h"

#define VECTOR_HIGH_ADDR 0xffff0000

// 1KB aligned
__attribute__ ((aligned(0x400))) uint32_t vector_l2_table[256] = {
    [(VECTOR_HIGH_ADDR >> 12)  & 0xff] = ((KERNBASE_PADDR + (1<<20))&0xfffff000) |
        L2_SECT_PRIVILEGED_RW | L2_SECT_MEM_DEFAULT | L2_SECT_NON_SUPER,
};

 //aligned by 16K
 // Here we use '+' instead of '|' because of compiler restrictions . Usage of '|'
 // and address of vector_l2_table leads to "error: initializer element is not constant"
__attribute__ ((aligned(0x4000))) uint32_t entry_l1_table[4096] = {
    // vector table
    [VECTOR_HIGH_ADDR>>20] = PHYS(vector_l2_table) + L1_TYPE_TABLE,

    // kernel low address
    [KERNBASE_PADDR>>20] = (KERNBASE_PADDR&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,

    // kernel high address
    [KERNBASE_VADDR>>20] = (KERNBASE_PADDR&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,

    // uart
    [0x2020000>>20] = (0x2020000&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_DEVICE | L1_TYPE_SECT,

    // scu
    [0xa00000>>20] = (0xa00000&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_DEVICE | L1_TYPE_SECT,
};




extern char __vector_table_start[];
extern char __vector_table_end[];
extern char vector_table[];


void copy_vector_table(void)
{
    printf("copy vector table to %p, from %p, size (0x%x)\n", (void *)VECTOR_HIGH_ADDR, __vector_table_start, __vector_table_end - __vector_table_start);
    memcpy((void *)VECTOR_HIGH_ADDR, __vector_table_start, __vector_table_end - __vector_table_start);

    sctlr_set(sctlr_get()|SCTLR_V);

    // Ask linker to not throw away exceptions.o from libkernel.a
    int tmp = vector_table[0];
    (void) tmp;
}

void l1_insert_kernel_mapping(uint32_t *l1_table)
{
    //vector_table
    l1_table[VECTOR_HIGH_ADDR>>20] = PHYS(vector_l2_table) + L1_TYPE_TABLE;

    // kernel high address
    l1_table[KERNBASE_VADDR>>20] = (KERNBASE_PADDR&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;

    // uart
    l1_table[0x2020000>>20] = (0x2020000&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_DEVICE | L1_TYPE_SECT;

    // scu
    l1_table[0xa00000>>20] = (0xa00000&0xfff00000) | L1_SECT_PRIVILEGED_RW |
        L1_SECT_MEM_DEVICE | L1_TYPE_SECT;
}
