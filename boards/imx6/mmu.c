
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
#include <arch/memlayout.h>

//TODO check alignment

// 1KB aligned
__attribute__ ((aligned(L2_TABLE_SIZE))) uint32_t vector_l2_table[L2_TABLE_SIZE] = {
    [L2_IDX(VECTOR_HIGH_VADDR)] = (VECTOR_PADDR) | L2_SECT_PRIVILEGED_RW | L2_SECT_MEM_DEFAULT | L2_SECT_NON_SUPER,
};

 //aligned by 16K
 // Here we use '+' instead of '|' because of compiler restrictions . Usage of '|'
 // and address of vector_l2_table leads to "error: initializer element is not constant"
__attribute__ ((aligned(L1_TABLE_SIZE))) uint32_t entry_l1_table[L1_TABLE_SIZE] = {
    // vector table
    [L1_IDX(VECTOR_HIGH_VADDR)] = PHYS(vector_l2_table) + L1_TYPE_TABLE,

    // kernel low address
    // Map VA's [0, 1MB) to PA's [0, 1MB). Hope this will be enough
    [L1_IDX(KERNBASE_PADDR)]= (KERNBASE_PADDR) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,

    // kernel high address
    [L1_IDX(KERNBASE_VADDR)] = (KERNBASE_PADDR) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,
    [L1_IDX(KERNBASE_VADDR) + 1] = (KERNBASE_PADDR + (1<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,
    [L1_IDX(KERNBASE_VADDR) + 2] = (KERNBASE_PADDR + (2<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,
    [L1_IDX(KERNBASE_VADDR) + 3] = (KERNBASE_PADDR + (3<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,
    [L1_IDX(KERNBASE_VADDR) + 4] = (KERNBASE_PADDR + (4<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,

    // uart
    [0x2020000>>20] = (0x2020000) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEVICE | L1_TYPE_SECT,

    // scu
    [0xa00000>>20] = (0xa00000) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEVICE | L1_TYPE_SECT,
};


void insert_kernel_mapping_into_table(uint32_t *l1_table)
{
    //vector_table
    l1_table[L1_IDX(VECTOR_HIGH_VADDR)] = PHYS(vector_l2_table) + L1_TYPE_TABLE;

    // kernel high address
    l1_table[L1_IDX(KERNBASE_VADDR)]     = (KERNBASE_PADDR) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;
    l1_table[L1_IDX(KERNBASE_VADDR) + 1] = (KERNBASE_PADDR + (1<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;
    l1_table[L1_IDX(KERNBASE_VADDR) + 2] = (KERNBASE_PADDR + (2<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;
    l1_table[L1_IDX(KERNBASE_VADDR) + 3] = (KERNBASE_PADDR + (3<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;
    l1_table[L1_IDX(KERNBASE_VADDR) + 4] = (KERNBASE_PADDR + (4<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;

    // uart
    l1_table[0x2020000>>20] = (0x2020000) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEVICE | L1_TYPE_SECT;

    // scu
    l1_table[0xa00000>>20] = (0xa00000) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEVICE | L1_TYPE_SECT;
}
