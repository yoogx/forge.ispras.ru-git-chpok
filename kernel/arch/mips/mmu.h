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

#define MIPS_PGSIZE_4K        0x000
#define MIPS_PGSIZE_16K       0x003 
#define MIPS_PGSIZE_64K       0x00F 
#define MIPS_PGSIZE_256K      0x03F 
#define MIPS_PGSIZE_1M        0x0FF 
#define MIPS_PGSIZE_4M        0x3FF 
#define MIPS_PGSIZE_16M       0xFFF 

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
#undef CASE
    }
}
#endif


#define EntryHi_VPN2(x)         (x << 13)           /*Биты VA 39..13 виртуального адреса (номер виртуальной страницы / 2).*/
#define EntryHi_ASID(x)         (x << 0)            /*Идентификатор виртуального адресного пространства*/
#define EntryHi_R(x)            ((x >> 4) << 29)    /*Сегмент виртуальной памяти, соответствующий   VA (63..62 битам).
                                                              Код            Значение
                                                              00              XUSEG. Пользовательский сегмент
                                                              01              XSSEG. Сегмент супервизора.
                                                              10              Зарезервировано.
                                                              11              XKSEG. Сегмент ядра.*/
#define EntryLo0_PFN(x)         (x >> 12) << 6            /*Номер страницы. Соответствует битам [35..12] физического адреса*/
#define EntryLo0_C(x)           (x << 3)            /*Код политики кэширования*/
#define EntryLo0_D(x)           (x & 0x4)           /*Бит, показывающий, что страница доступна для записи.*/
#define EntryLo0_V(x)           (x & 0x2)           /*Бит, показывающий, что содержимое TLB доступно для чтения.*/
#define EntryLo0_G(x)           (x << 0)            /*Бит, показывающий, что поле ASID при обращении в данную страницу не проверяется.
                                                       При записи в TLB, логическое «И» G битов из регистров EntryLo0 и EntryLo1 становится G битом
                                                       в записываемой строке TLB.*/
#define EntryLo1_PFN(x)         (x << 6)            /*Номер страницы. Соответствует битам [35..12] физического адреса*/
#define EntryLo1_C(x)           (x << 3)            /*Код политики кэширования*/
#define EntryLo1_D(x)           (x & 0x4)           /*Бит, показывающий, что страница доступна для записи.*/
#define EntryLo1_V(x)           (x & 0x2)           /*Бит, показывающий, что содержимое TLB доступно для чтения.*/
#define EntryLo1_G(x)           (x << 0)            /*Бит, показывающий, что поле ASID при обращении в данную страницу не проверяется.
                                                       При записи в TLB, логическое «И» G битов из регистров EntryLo0 и EntryLo1 становится G битом
                                                       в записываемой строке TLB.*/

#define PageSize_Mask_SHIFT        7
#define PageSize_Mask(x)           ((x) << PageSize_Mask_SHIFT)

#define EntryHi_USEG           0x0
#define EntryHi_SSEG           0x1
#define EntryHi_KSEG           0x3

#define EntryLo_R              0x2                 /*Защита чтения*/
#define EntryLo_W              0x4                 /*Защита записи*/






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
void pok_mips_tlb_write(
        uint32_t virtual, 
        uint64_t physical, 
        unsigned pgsize_enum, 
        unsigned permissions,
        unsigned wimge,
        unsigned pid,
        pok_bool_t valid
    );

/**
 * Sets 'V' of the specified TLB entry to false.
 */
/*
 * @ requires tlbsel < 2;
 */

/*
 * @ requires tlbsel < 2;
 */
void pok_mips_tlb_read_entry(
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

#define pok_mips_tlb_get_nentry(tlbsel) 0
//FIXIT
    //~ (mfspr((SPRN_TLB ## tlbsel ## CFG)) & TLBnCFG_N_ENTRY_MASK)

#endif
