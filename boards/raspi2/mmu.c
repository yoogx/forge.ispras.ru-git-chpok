
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
#include <bsp/memory_mapping.h>
#include <assert.h>


STATIC_ASSERT(KERNBASE_PADDR%0x100000 == 0); //should be 1MB aligned
STATIC_ASSERT(KERNBASE_VADDR%0x100000 == 0); //should be 1MB aligned

// 1KB aligned
__attribute__ ((aligned(L2_TABLE_SIZE))) uint32_t vector_l2_table_rw[L2_TABLE_SIZE] = {
    [L2_IDX(VECTOR_HIGH_VADDR)] = (VECTOR_PADDR) | L2_SECT_PRIVILEGED_RW | L2_SECT_MEM_DEFAULT | L2_SECT_NON_SUPER,
};

// 1KB aligned
__attribute__ ((aligned(L2_TABLE_SIZE))) uint32_t vector_l2_table_ro[L2_TABLE_SIZE] = {
    [L2_IDX(VECTOR_HIGH_VADDR)] = (VECTOR_PADDR) | L2_SECT_PRIVILEGED_RW | L2_SECT_MEM_DEFAULT | L2_SECT_NON_SUPER,
};

 //aligned by 16K
 // Here we use '+' instead of '|' because of compiler restrictions . Usage of '|'
 // and address of vector_l2_table leads to "error: initializer element is not constant"
__attribute__ ((aligned(L1_TABLE_SIZE))) uint32_t entry_l1_table[L1_TABLE_SIZE] = {
    // vector table
    // Maps page: VA=VECTOR_HIGH_VADDR to PA=VECTOR_PADDR. RW access to allow vector table filling
    [L1_IDX(VECTOR_HIGH_VADDR)] = PHYS(vector_l2_table_rw) + L1_TYPE_TABLE,

    // kernel low address
    // Map VA's [0, 1MB) to PA's [0, 1MB). Hope this will be enough
    [L1_IDX(KERNBASE_PADDR)]= (KERNBASE_PADDR) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,

    // kernel high address
    [L1_IDX(KERNBASE_VADDR)]     = (KERNBASE_PADDR) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,
    [L1_IDX(KERNBASE_VADDR) + 1] = (KERNBASE_PADDR + (1<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,
    [L1_IDX(KERNBASE_VADDR) + 2] = (KERNBASE_PADDR + (2<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,
    [L1_IDX(KERNBASE_VADDR) + 3] = (KERNBASE_PADDR + (3<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,
    [L1_IDX(KERNBASE_VADDR) + 4] = (KERNBASE_PADDR + (4<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT,


    // gpio (for uart)
    [L1_IDX(VIRT_IO(0x3F200000))] = (0x3F200000) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEVICE | L1_TYPE_SECT,

    // local peripherals (timer irq)
    [L1_IDX(VIRT_IO(0x40000000))] = (0x40000000) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEVICE | L1_TYPE_SECT,
};


void insert_kernel_mapping_into_table(uint32_t *l1_table)
{
    // vector_table
    // Maps page: VA=VECTOR_HIGH_VADDR to PA=VECTOR_PADDR. RO access to not allow changing
    l1_table[L1_IDX(VECTOR_HIGH_VADDR)] = PHYS(vector_l2_table_ro) + L1_TYPE_TABLE;

    // kernel high address
    l1_table[L1_IDX(KERNBASE_VADDR)]     = (KERNBASE_PADDR) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;
    l1_table[L1_IDX(KERNBASE_VADDR) + 1] = (KERNBASE_PADDR + (1<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;
    l1_table[L1_IDX(KERNBASE_VADDR) + 2] = (KERNBASE_PADDR + (2<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;
    l1_table[L1_IDX(KERNBASE_VADDR) + 3] = (KERNBASE_PADDR + (3<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;
    l1_table[L1_IDX(KERNBASE_VADDR) + 4] = (KERNBASE_PADDR + (4<<L1_IDX_SHIFT)) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEFAULT | L1_TYPE_SECT;

    // gpio (for uart)
    l1_table[L1_IDX(VIRT_IO(0x3F200000))] = (0x3F200000) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEVICE | L1_TYPE_SECT;

    // local peripherals (timer irq)
    l1_table[L1_IDX(VIRT_IO(0x40000000))] = (0x40000000) | L1_SECT_PRIVILEGED_RW | L1_SECT_MEM_DEVICE | L1_TYPE_SECT;

}
