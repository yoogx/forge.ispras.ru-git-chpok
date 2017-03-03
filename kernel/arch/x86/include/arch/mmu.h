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

// page directory index from virtual addr
#define PDX(va)    ((((uintptr_t) (va)) >> PDXSHIFT) & 0x3FF)

// page table index from virtual addr
#define PTX(va)    ((((uintptr_t) (va)) >> PTXSHIFT) & 0x3FF)

// Page table addr (which is phys addr) from pde by masking 12 low bits
#define PT_ADDR(pde) ((uintptr_t) (pde) & ~0xFFF)

#define PAGE_SIZE   4096 // bytes mapped by a page

#define PTXSHIFT 12   // offset of PTX in a virtual address
#define PDXSHIFT 22   // offset of PDX in a virtual address



// Page table/directory entry flags.
#define PAGE_P  0x001 // Present
#define PAGE_RW 0x002 // Writeable
#define PAGE_U  0x004 // User
#define PAGE_WT 0x008 // Write-Through
#define PAGE_CD 0x010 // Cache-Disable
#define PAGE_A  0x020 // Accessed
#define PAGE_D  0x040 // Dirty
#define PAGE_S  0x080 // Page Size
#define PAGE_G  0x100 // Global

#define PAGE_AVAIL 0xE00 // Available for software use

