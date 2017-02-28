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

#ifndef __POK_MIPS_MMU_EXT_H__
#define __POK_MIPS_MMU_EXT_H__

#define MIPS_PGSIZE_4K        0x000
#define MIPS_PGSIZE_16K       0x003 
#define MIPS_PGSIZE_64K       0x00F 
#define MIPS_PGSIZE_256K      0x03F 
#define MIPS_PGSIZE_1M        0x0FF 
#define MIPS_PGSIZE_4M        0x3FF 
#define MIPS_PGSIZE_16M       0xFFF 

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
#define EntryLo0_PFN(x)         ((x >> 12) << 6)    /*Номер четной страницы. Соответствует битам [35..12] физического адреса*/
#define EntryLo0_C(x)           (x << 3)            /*Код политики кэширования*/
#define EntryLo0_D(x)           (x & 0x4)           /*Бит, показывающий, что страница доступна для записи.*/
#define EntryLo0_V(x)           (x & 0x2)           /*Бит, показывающий, что содержимое TLB доступно для чтения.*/
#define EntryLo0_G(x)           (x << 0)            /*Бит, показывающий, что поле ASID при обращении в данную страницу не проверяется.
                                                       При записи в TLB, логическое «И» G битов из регистров EntryLo0 и EntryLo1 становится G битом
                                                       в записываемой строке TLB.*/
#define EntryLo1_PFN(x)         ((x >> 12) << 6)    /*Номер нечетной страницы. Соответствует битам [35..12] физического адреса*/
#define EntryLo1_PS(x)          ((x + 0x1) << 6)    /*Добавка к нечетной физ. странице, зависящая от размера страницы*/
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


#endif
