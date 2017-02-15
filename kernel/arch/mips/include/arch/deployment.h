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

#ifndef __JET_MIPS_DEPLOYMENT_H__
#define __JET_MIPS_DEPLOYMENT_H__

#include <stdint.h>

/*
 * Signle TLB entry for memory maping.
 */
    
struct tlb_entry {
    uint32_t virt_addr;
    uint64_t phys_addr;
    unsigned size;
    unsigned permissions;
    unsigned cache_policy;
    unsigned pid;
};


/*
 * Global array TLB entries.
 *
 * Should be defined in deployment_arch.c.
 */
extern struct tlb_entry tlb_entries[];
extern int tlb_entries_n;

#endif /* __JET_MIPS_DEPLOYMENT_H__ */
