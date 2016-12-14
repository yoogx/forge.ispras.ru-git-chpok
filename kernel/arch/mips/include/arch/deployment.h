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
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

/* Definitions which are used in deployment.c */

#ifndef __JET_MIPS_DEPLOYMENT_H__
#define __JET_MIPS_DEPLOYMENT_H__

/* 
 * Virtual address where partition's memory starts.
 * 
 * This address is the same for user and kernel.
 */
#define POK_PARTITION_MEMORY_BASE 0x60000000ULL
/*
 * Size of memory in the single chunk.
 * 
 * DEV: This size corresponds to MIPS_PGSIZE_16M constant, used
 * for memory mapping.
 */
#define POK_PARTITION_MEMORY_SIZE 0x1000000ULL 

/* 
 * Description of one user space.
 */
struct ja_mips_space
{
    /* Physical address of memory chunk. */
    uintptr_t   phys_base;
    /* 
     * Size of the memory for normal use. 
     * Everything above is used for stack.
     */
    size_t      size_normal;
    
    uint32_t    ustack_state; // State of the user stack allocator.
};

/*
 * Array of user space descriptions.
 * 
 * Should be defined in deployment.c.
 */
extern struct ja_mips_space ja_spaces[];
extern int ja_spaces_n;

#endif /* __JET_MIPS_DEPLOYMENT_H__ */
