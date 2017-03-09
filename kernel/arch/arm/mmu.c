
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
#include "memlayout.h"

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


extern char __vector_table_start[];
extern char __vector_table_end[];

void copy_vector_table(void)
{
    memcpy(0, __vector_table_start, __vector_table_end - __vector_table_start);
    printf("copy vector table to %p, from %p, size (0x%x)\n", NULL, __vector_table_start, __vector_table_end - __vector_table_start);
}
