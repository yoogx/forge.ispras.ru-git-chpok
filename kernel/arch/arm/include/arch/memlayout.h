/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

#ifndef __ARM_MEMLAYOUT_H__
#define __ARM_MEMLAYOUT_H__

#include <board/ram.h>

/* KERNBASE_ values should be aligned by 1MB */
#define KERNBASE_PADDR RAM_START_ADDR
/* #define KERNBASE_PADDR 0x0 */
#define KERNBASE_VADDR 0xc0000000

/* addr of interrupt vector table */
#define VECTOR_HIGH_ADDR 0xffff0000


#ifdef __ASSEMBLER__
#define PHYS(pa) ((pa) - KERNBASE_VADDR + KERNBASE_PADDR)
#define VIRT(va) ((va) - KERNBASE_PADDR + KERNBASE_VADDR)
#else
#define PHYS(pa) ((uintptr_t)(pa) - KERNBASE_VADDR + KERNBASE_PADDR)
#define VIRT(va) ((uintptr_t)(va) - KERNBASE_PADDR + KERNBASE_VADDR)
#endif


#endif
