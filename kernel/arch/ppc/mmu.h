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

#ifndef __POK_PPC_MMU_H__
#define __POK_PPC_MMU_H__

#include <arch/mmu_ext.h>

#define TLBnCFG_N_ENTRY_MASK    0x00000fff

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
void pok_ppc_tlb_write(
        unsigned tlbsel,
        uint32_t virtual, 
        uint64_t physical, 
        unsigned pgsize_enum, 
        unsigned permissions,
        unsigned wimge,
        unsigned pid,
        unsigned entry,
        pok_bool_t valid
    );

/**
 * Sets 'V' of the specified TLB entry to false.
 */
/*
 * @ requires tlbsel < 2;
 */
void pok_ppc_tlb_clear_entry(
        unsigned tlbsel,
        unsigned entry
    );

/*
 * @ requires tlbsel < 2;
 */
void pok_ppc_tlb_read_entry(
        unsigned tlbsel,
        unsigned entry,
        unsigned *valid, 
        unsigned *tsize, 
        uint32_t *epn,
        uint64_t *rpn);

/*
 * @ requires tlbsel < 5;
 */
// unsigned pok_ppc_get_tlb_nentry(
//        unsigned tlbsel;
//    );

#define pok_ppc_tlb_get_nentry(tlbsel)                       \
    (mfspr((SPRN_TLB ## tlbsel ## CFG)) & TLBnCFG_N_ENTRY_MASK)


void pok_ppc_tlb_print(unsigned tlbsel);

#endif
