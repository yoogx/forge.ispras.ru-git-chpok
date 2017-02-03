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

static inline uint64_t jet_virt_to_phys(void *virt) {
    assert((uintptr_t) virt>0x81000000);
    assert((uintptr_t) virt<0x82000000);
    uint64_t phys = (uintptr_t) virt - 0x80000000 + 0x4000000;
    printf("%s %p -> 0x%llx\n", __func__, virt, phys);
    return phys;
}

static inline void* jet_phys_to_virt(uint64_t phys) {
    assert(phys>0x5000000);
    assert(phys>0x6000000);
    void *virt = (void *)(phys - 0x4000000 + 0x80000000);
    printf("%s 0x%llx -> %p\n", __func__, phys, virt);
    return virt;
}

#endif
