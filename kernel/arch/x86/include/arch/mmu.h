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

// A linear address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(la) --/ \--- PTX(la) --/ \---- PGOFF(la) ----/
//  \---------- PGNUM(la) ----------/
//
// The PDX, PTX, PGOFF, and PGNUM macros decompose linear addresses as shown.
// To construct a linear address la from PDX(la), PTX(la), and PGOFF(la),
// use PGADDR(PDX(la), PTX(la), PGOFF(la)).

// page number field of address
#define PGNUM(la)       (((uintptr_t) (la)) >> PTXSHIFT)

// page directory index
#define PDX(la)         ((((uintptr_t) (la)) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(la)         ((((uintptr_t) (la)) >> PTXSHIFT) & 0x3FF)

// offset in page
#define PGOFF(la)       (((uintptr_t) (la)) & 0xFFF)

// Page directory and page table constants.
#define NPDENTRIES      1024            // page directory entries per page directory
#define NPTENTRIES      1024            // page table entries per page table

#define PGSIZE          4096            // bytes mapped by a page
#define PGSHIFT         12              // log2(PGSIZE)

#define PTSIZE          (PGSIZE*NPTENTRIES) // bytes mapped by a page directory entry
#define PTSHIFT         22              // log2(PTSIZE)

#define PTXSHIFT        12              // offset of PTX in a linear address
#define PDXSHIFT        22              // offset of PDX in a linear address

#define PTE_ADDR(pte) ((uintptr_t) (pte) & ~0xFFF)


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

