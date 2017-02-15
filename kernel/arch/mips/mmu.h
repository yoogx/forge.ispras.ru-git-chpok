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

#ifndef __POK_MIPS_MMU_H__
#define __POK_MIPS_MMU_H__

#include <arch/mmu_ext.h>

extern int current_tlb_index;


/**
 * Write address mapping into the specified TLB entry.
 *
 * For the meaning of parameters please consult EREF_RM
 *   Section 4.12.10, "MMU Assist Registers (MASn)", and
 *   Section 7.5.3.6, "Writing TLB Entries".
 */
/*
 * @ requires tlbsel < 2;
 */
void pok_mips_tlb_write(
        uint64_t virtual, 
        uint32_t physical, 
        unsigned pgsize_enum, 
        unsigned permissions,
        unsigned wimge,
        unsigned pid,
        pok_bool_t valid
    );


void pok_mips_tlb_read_entry(
        int index,
        unsigned *valid, 
        unsigned *tsize, 
        uint32_t *epn,
        uint64_t *rpn);

void dump_tlb(int first, int last);

/*
 * @ requires tlbsel < 5;
 */
// unsigned pok_mips_get_tlb_nentry(
//        unsigned tlbsel;
//    );
  
void pok_mips_tlb_print();



#endif
