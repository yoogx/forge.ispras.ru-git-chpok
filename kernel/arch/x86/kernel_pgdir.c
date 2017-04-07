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

#include <arch/mmu.h>
#include <stdint.h>
#include <arch/memlayout.h>

#define PGDIR_ENTRIES_N  1024 // page directory entries per page directory

/* page directory used while starting of kernel */
__attribute__((__aligned__(PAGE_SIZE))) uint32_t entry_pgdir[PGDIR_ENTRIES_N] = {

        // Map VA's [0, 4MB) to PA's [0, 4MB) //This should be enough
        [0] = (0) | PAGE_P | PAGE_RW| PAGE_S,

        // Map VA's [KERNBASE, KERNBASE+8MB) to PA's [0, 8MB)
        [PDX(KERNBASE)]     = (0)           | PAGE_P | PAGE_RW | PAGE_S,
        [PDX(KERNBASE) + 1] = (1<<PDXSHIFT) | PAGE_P | PAGE_RW | PAGE_S,
    };

void pgdir_insert_kernel_mapping(uint32_t *pgdir)
{
    // Map VA's [KERNBASE, KERNBASE+8MB) to PA's [0, 8MB)
    pgdir[PDX(KERNBASE)]     = (0)           | PAGE_P | PAGE_RW | PAGE_S;
    pgdir[PDX(KERNBASE) + 1] = (1<<PDXSHIFT) | PAGE_P | PAGE_RW | PAGE_S;
}

