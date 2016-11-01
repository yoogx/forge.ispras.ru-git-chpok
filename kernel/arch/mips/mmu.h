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

#define MIPS_PGSIZE_4K        0b00010 
#define MIPS_PGSIZE_16K       0b00100 
#define MIPS_PGSIZE_64K       0b00110 
#define MIPS_PGSIZE_256K      0b01000 
#define MIPS_PGSIZE_1M        0b01010 
#define MIPS_PGSIZE_4M        0b01100 
#define MIPS_PGSIZE_16M       0b01110 
#define MIPS_PGSIZE_64M       0b10000 
#define MIPS_PGSIZE_256M      0b10010 
#define MIPS_PGSIZE_1G        0b10100 
#define MIPS_PGSIZE_4G        0b10110 

#if 0
static inline int pok_arch_mmu_shift_by_size(unsigned size)
{
    switch (size) {
#define CASE(x) case MIPS_PGSIZE_##x: return MIPS_PGSIZE_##x##_SHFT; 
        CASE(4K);
        CASE(16K);
        CASE(64K);
        CASE(256K);
        CASE(1M);
        CASE(4M);
        CASE(16M);
        CASE(64M);
        CASE(256M);
        CASE(1G);
        CASE(4G);
#undef CASE
    }
}
#endif

#define TLBnCFG_N_ENTRY_MASK    0x00000fff

#define MAS0_TLBSEL_MASK        0x30000000
#define MAS0_TLBSEL_SHIFT       28
#define MAS0_TLBSEL(x)          (((x) << MAS0_TLBSEL_SHIFT) & MAS0_TLBSEL_MASK)
#define MAS0_GET_TLBSEL(mas0)   (((mas0) & MAS0_TLBSEL_MASK) >> \
                                MAS0_TLBSEL_SHIFT)
#define MAS0_ESEL_MASK          0x0FFF0000
#define MAS0_ESEL_SHIFT         16
#define MAS0_ESEL(x)            (((x) << MAS0_ESEL_SHIFT) & MAS0_ESEL_MASK)
#define MAS0_NV(x)              ((x) & 0x00000FFF)
#define MAS0_HES                0x00004000
#define MAS0_WQ_ALLWAYS         0x00000000
#define MAS0_WQ_COND            0x00001000
#define MAS0_WQ_CLR_RSRV        0x00002000

#define MAS1_VALID              0x80000000
#define MAS1_IPROT              0x40000000
#define MAS1_TID(x)             (((x) << 16) & 0x3FFF0000)
#define MAS1_IND                0x00002000
#define MAS1_TS                 0x00001000
#define MAS1_TSIZE_MASK         0x00000f80
#define MAS1_TSIZE_SHIFT        7
#define MAS1_TSIZE(x)           (((x) << MAS1_TSIZE_SHIFT) & MAS1_TSIZE_MASK)
#define MAS1_GET_TSIZE(mas1)    (((mas1) & MAS1_TSIZE_MASK) >> MAS1_TSIZE_SHIFT)

#define MAS2_EPN                (~0xFFFUL)
#define MAS2_X0                 0x00000040
#define MAS2_X1                 0x00000020
#define MAS2_W                  0x00000010
#define MAS2_I                  0x00000008
#define MAS2_M                  0x00000004
#define MAS2_G                  0x00000002
#define MAS2_E                  0x00000001
#define MAS2_WIMGE_MASK         0x0000001f
#define MAS2_EPN_MASK(size)             (~0 << (size + 10))
#define MAS2_VAL(addr, size, flags)     ((addr) & MAS2_EPN_MASK(size) | (flags))

#define MAS3_RPN                0xFFFFF000
#define MAS3_U0                 0x00000200
#define MAS3_U1                 0x00000100
#define MAS3_U2                 0x00000080
#define MAS3_U3                 0x00000040
#define MAS3_UX                 0x00000020
#define MAS3_SX                 0x00000010
#define MAS3_UW                 0x00000008
#define MAS3_SW                 0x00000004
#define MAS3_UR                 0x00000002
#define MAS3_SR                 0x00000001
#define MAS3_BAP_MASK           0x0000003f
#define MAS3_SPSIZE             0x0000003e
#define MAS3_SPSIZE_SHIFT       1

#define MAS7_RPN                0xFFFFFFFF
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

#define pok_ppc_tlb_get_nentry(tlbsel) 0
//FIXIT
    //~ (mfspr((SPRN_TLB ## tlbsel ## CFG)) & TLBnCFG_N_ENTRY_MASK)

#endif
