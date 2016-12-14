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

extern int current_tlb_index;


#define MIPS_MAX_TLB_SIZE   64



#define EntryHi_VPN2_clear_bit  ~((1 << 13) - 1)
#define EntryHi_VPN2_lo(x)      (x & EntryHi_VPN2_clear_bit)           /*Биты VA 31..13 виртуального адреса (номер виртуальной страницы / 2).*/
#define EntryHi_VPN2_hi(x)      (x & 0xFF)                             /*Биты VA 39..32 виртуального адреса (номер виртуальной страницы / 2).*/
#define EntryHi_ASID(x)         (x << 0)           /*Идентификатор виртуального адресного пространства*/
#define EntryHi_R(x)            ((x >> 4) << 30)   /*Сегмент виртуальной памяти, соответствующий   VA (63..62 битам).
                                                              Код            Значение
                                                              00              XUSEG. Пользовательский сегмент
                                                              01              XSSEG. Сегмент супервизора.
                                                              10              Зарезервировано.
                                                              11              XKSEG. Сегмент ядра.*/
#define EntryLo0_PFN(x)         ((x >> 12) << 6)    /*Номер страницы. Соответствует битам [35..12] физического адреса*/
#define EntryLo0_C(x)           (x << 3)            /*Код политики кэширования*/
#define EntryLo0_D(x)           (x & 0x4)           /*Бит, показывающий, что страница доступна для записи.*/
#define EntryLo0_V(x)           (x & 0x2)           /*Бит, показывающий, что содержимое TLB доступно для чтения.*/
#define EntryLo0_G(x)           (x << 0)            /*Бит, показывающий, что поле ASID при обращении в данную страницу не проверяется.
                                                       При записи в TLB, логическое «И» G битов из регистров EntryLo0 и EntryLo1 становится G битом
                                                       в записываемой строке TLB.*/
#define EntryLo1_PFN(x)         ((x >> 12) << 6)            /*Номер страницы. Соответствует битам [35..12] физического адреса*/
#define EntryLo1_C(x)           (x << 3)            /*Код политики кэширования*/
#define EntryLo1_D(x)           (x & 0x4)           /*Бит, показывающий, что страница доступна для записи.*/
#define EntryLo1_V(x)           (x & 0x2)           /*Бит, показывающий, что содержимое TLB доступно для чтения.*/
#define EntryLo1_G(x)           (x << 0)            /*Бит, показывающий, что поле ASID при обращении в данную страницу не проверяется.
                                                       При записи в TLB, логическое «И» G битов из регистров EntryLo0 и EntryLo1 становится G битом
                                                       в записываемой строке TLB.*/

#define PageSize_Mask_SHIFT        13
#define PageSize_Mask(x)           ((x) << PageSize_Mask_SHIFT)

#define EntryHi_R_shift(x)     (x << 4)

#define EntryHi_USEG           EntryHi_R_shift(0x0)
#define EntryHi_SSEG           EntryHi_R_shift(0x1)
#define EntryHi_KSEG           EntryHi_R_shift(0x3)

#define EntryLo_V              0x2                 /*Защита чтения. 1 - разрешено, 0 - запрещено*/
#define EntryLo_D              0x4                 /*Защита записи. 1 - разрешено, 0 - запрещено*/


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
        int index,
        unsigned *valid, 
        unsigned *tsize, 
        uint32_t *epn,
        uint64_t *rpn);

void dump_tlb(int first, int last);
const char *msk2str(uint32_t mask);

/*
 * @ requires tlbsel < 5;
 */
// unsigned pok_mips_get_tlb_nentry(
//        unsigned tlbsel;
//    );



 /*
  * PAGE_SHIFT determines the page size
  */
#define PAGE_MASK(page_shift)       (~((1 << page_shift) - 1))

/*TLB pages in system*/
#define jet_mips_tlb_get_index      current_tlb_index  
#define jet_mips_tlb_get_inc_index  current_tlb_index++;


#define PM_4K           0x000 << 13
#define PM_16K          0x003 << 13
#define PM_64K          0x00f << 13
#define PM_256K         0x03f << 13
#define PM_1M           0x0ff << 13
#define PM_4M           0x3ff << 13
#define PM_16M          0xfff << 13

void pok_ppc_tlb_print();


#endif
