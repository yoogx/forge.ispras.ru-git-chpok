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

#ifndef __ARM_MMU_H__
#define __ARM_MMU_H__

#define L1_TYPE_FAULT (0)
#define L1_TYPE_TABLE (1)
#define L1_TYPE_SECT  (2)
#define L1_TYPE_MASK  (3)

#define L1_SECT_B      ( 1 << 2)   // bufferable
#define L1_SECT_C      ( 1 << 3)   // cacheable
#define L1_SECT_XN     ( 1 << 4)   // execute never
#define L1_SECT_AP(x)  ((x) << 10) // access permission
#define L1_SECT_TEX(x) ((x) << 12) // type extension
#define L1_SECT_APX    ( 1 << 15)
#define L1_SECT_S      ( 1 << 16)  // shareable
#define L1_SECT_nG     ( 1 << 17)  // non global
#define L1_SECT_SUPER  ( 1 << 18)  // supersection

#define L1_SECT_PRIVILEGED_RW (L1_SECT_AP(1))
#define L1_SECT_USER_RW (L1_SECT_AP(3))
#define L1_SECT_USER_RO (L1_SECT_AP(2))

#define L1_SECT_MEM_DEFAULT (L1_SECT_TEX(0) | L1_SECT_C | L1_SECT_B)

#define L1_SECT_MEM_DEVICE (L1_SECT_TEX(2))

#define L2_ADDR(l1_entry) ((uint32_t)l1_entry&0xfffffc00) //make low bit to get l2 base addr

#define L2_SECT_XN     ( 1 << 0)    // execute never
#define L2_SECT_NON_SUPER  ( 1 << 1)// not a supersection
#define L2_SECT_B      ( 1 << 2)    // bufferable
#define L2_SECT_C      ( 1 << 3)    // cacheable
#define L2_SECT_AP(x)  ((x) << 4)   // access permission
#define L2_SECT_TEX(x) ((x) << 6)   // type extension
#define L2_SECT_APX    ( 1 << 9)
#define L2_SECT_S      ( 1 << 10)  // shareable
#define L2_SECT_nG     ( 1 << 11)  // non global

#define L2_SECT_PRIVILEGED_RW (L2_SECT_AP(1))
#define L2_SECT_MEM_DEFAULT (L2_SECT_TEX(0) | L2_SECT_C | L2_SECT_B)

#define L2_SECT_USER_RW (L2_SECT_AP(3))
#define L2_SECT_USER_RO (L2_SECT_AP(2))

#define L1_TABLE_SIZE 0x4000 // 4096 of 4 byte
#define L2_TABLE_SIZE 0x400 // 256 of 4 byte

#define L1_IDX(va) (((uint32_t) va) >> 20)

#define L2_IDX(va) ((((uint32_t) va) >> 12) & 0xff)


// Write l1_table addr to TTBR0
static inline void load_l1_table(uint32_t *l1_table)
{
    asm("mcr p15, 0, %0, c2, c0, 0"
            :
            :"r"(l1_table)
            :"memory");
}

#endif
