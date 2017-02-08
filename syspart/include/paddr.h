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

#ifndef __POK_SYSPART_PADDR_H__
#define __POK_SYSPART_PADDR_H__
#include <assert.h>

/*
 * Converts virtiual address in memory block to physical address
 */
static inline uint64_t jet_virt_to_phys(jet_memory_block_status_t *mb, void *virt_ptr) {
    if (mb->paddr == 0) {
        printf("%s: Memory block paddr == 0. Memory block inn't contiguous?"
                "Memory block addr = 0x%x\n", __func__, mb->addr);
        abort();
    }

    uintptr_t vaddr = (uintptr_t) virt_ptr;
    assert(vaddr >= mb->addr);
    assert(vaddr < mb->addr+mb->size);

    return vaddr - mb->addr + mb->paddr;
}

/*
 * Converts physical address in memory block to virtual address
 */

static inline void* jet_phys_to_virt(jet_memory_block_status_t *mb, uint64_t phys) {
    if (mb->paddr == 0) {
        printf("%s: Memory block paddr == 0. Memory block isn't contiguous?"
                "Memory block addr = 0x%x\n", __func__, mb->addr);
        abort();
    }
    assert(phys >= mb->paddr);
    assert(phys < mb->paddr+mb->size);
    uintptr_t vaddr = (uintptr_t)(phys-mb->paddr) + mb->addr;

    return (void *)vaddr;
}

#endif
