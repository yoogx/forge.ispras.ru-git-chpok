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

#include <types.h>
#include "mmu.h"
#include "reg.h"
#include "cp0.h"

#include <assert.h>

int     current_tlb_index = -1;


const char *msk2str(uint32_t mask)
{
        switch (mask) {
        case PM_4K:     return "4kb";
        case PM_16K:    return "16kb";
        case PM_64K:    return "64kb";
        case PM_256K:   return "256kb";
        case PM_1M:     return "1Mb";
        case PM_4M:     return "4Mb";
        case PM_16M:    return "16Mb";        
        }
        return "";
}

static int msk2offset(uint32_t mask)
{
        switch (mask) {
        case PM_4K:     return 12;
        case PM_16K:    return 14;
        case PM_64K:    return 16;
        case PM_256K:   return 18;
        case PM_1M:     return 20;
        case PM_4M:     return 22;
        case PM_16M:    return 24;        
        }
        return 12;
}



/*
 *  Quotes from the manual:
 *
 *      64-entry, fully-associative unified (for instruction and data accesses) L2 TLB array (TLB1)
 *      supports the 11 VSP page sizes shown in Section 6.2.3, “Variable-Sized Pages.”
 *
 * TODO: document parameters
 */

void pok_mips_tlb_write(
        uint64_t virtual, 
        uint32_t physical, 
        unsigned pgsize_enum, 
        unsigned permissions,
        unsigned wimge,
        unsigned pid,
        pok_bool_t  valid
        )
{
    (void) wimge;
    (void) valid;
    /*
     * TLB can be written by first writing the necessary information into EntryLo1, EntryLo2, EntryHi and
     * PageMask using mtc0 and then executing the tlbwr instruction.
     */
    jet_mips_tlb_get_inc_index;
    assert(jet_mips_tlb_get_index < MIPS_MAX_TLB_SIZE);
    mtc0(CP0_INDEX, jet_mips_tlb_get_index);
    mtc0(CP0_PAGEMASK, PageSize_Mask(pgsize_enum));
    uint32_t entrylo0, entrylo1;
    entrylo0 = EntryLo0_PFN(physical) | EntryLo0_D(permissions) | EntryLo0_V(permissions);
    entrylo1 = (EntryLo1_PFN(physical) | EntryLo1_D(permissions) | EntryLo1_V(permissions)) + EntryLo1_PS(pgsize_enum);
    /*Registers EntryLo0 and EntryLo1 used only 30 bit, so we can use mtc0*/
    if (pid == 0) //any pid 
    {
        mtc0(CP0_ENTRYLO0, EntryLo0_G(1) | entrylo0);
        mtc0(CP0_ENTRYLO1, EntryLo1_G(1) | entrylo1);
    }else{
        mtc0(CP0_ENTRYLO0, EntryLo0_G(0) | entrylo0);
        mtc0(CP0_ENTRYLO1, EntryLo1_G(0) | entrylo1);
    }        
    dmtc0(CP0_ENTRYHI, EntryHi_R(permissions) | EntryHi_VPN2_hi(virtual), EntryHi_ASID(pid) | EntryHi_VPN2_lo(virtual));
    asm volatile("tlbwi":::"memory");
    asm volatile("sync");

}



void pok_mips_tlb_read_entry(
        int index,
        unsigned *valid, 
        unsigned *tsize, 
        uint32_t *epn,
        uint64_t *rpn0,
        uint64_t *rpn1)
{
    uint32_t entrylo1, entrylo0, entryhi, pagemask;

    

    mtc0(CP0_INDEX, index);

    asm volatile("tlbr;sync");

    entrylo0 = mfc0(CP0_ENTRYLO0);
    entrylo1 = mfc0(CP0_ENTRYLO1);
    entryhi  = mfc0(CP0_ENTRYHI);
    pagemask = mfc0(CP0_PAGEMASK);


    *valid = (entrylo1 & EntryLo_V) & (entrylo0 & EntryLo_V);
    *tsize = pagemask;
    *epn = entryhi & EntryHi_VPN2_clear_bit;
/*  Read only 1 entrylo, because they are the same.*/
    *rpn0 = (entrylo0 << 6) & PAGE_MASK(msk2offset(pagemask));
    *rpn1 = (entrylo1 << 6) & PAGE_MASK(msk2offset(pagemask));
}







// #define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

void dump_tlb(int first, int last)
{
        uint32_t s_entryhi, entryhi;//, asid;
        uint32_t entrylo0, entrylo1, pa;
        uint32_t s_index, s_pagemask;
        uint32_t pagemask, c0, c1;
        uint32_t asidmask = 0xff;
        // int asidwidth = DIV_ROUND_UP(ilog2(asidmask) + 1, 4);
// #ifdef CONFIG_32BIT
        // bool xpa = cpu_has_xpa && (read_c0_pagegrain() & PG_ELPA);
        // int pwidth = 8;// xpa ? 11 : 8;
        // int vwidth = 8;
// #else
        // bool xpa = false;
        // int pwidth = 11;
        // int vwidth = 11;
// #endif

        s_pagemask = mfc0(CP0_PAGEMASK);
        s_entryhi  = mfc0(CP0_ENTRYHI);
        s_index    = mfc0(CP0_INDEX);
        // asid = s_entryhi & asidmask;

        for (int i = first; i <= last; i++) {
                mtc0(CP0_INDEX, i);
                TLBR();
                pagemask = mfc0(CP0_PAGEMASK);
                entryhi  = mfc0(CP0_ENTRYHI);
                entrylo0 = mfc0(CP0_ENTRYLO0);
                entrylo1 = mfc0(CP0_ENTRYLO1);
                    
                /*
                 * Prior to tlbinv, unused entries have a virtual address of
                 * CKSEG0.
                 */
                // if ((entryhi & ~0x1ffffUL) == CKSEG0)
                //        continue;
                
                /*
                 * ASID takes effect in absence of G (global) bit.
                 * We check both G bits, even though architecturally they should
                 * match one another, because some revisions of the SB1 core may
                 * leave only a single G bit set after a machine check exception
                 * due to duplicate TLB entry.
                 */
                // if (!((entrylo0 | entrylo1) & CP0_ENTRYLO_G) &&
                //  (entryhi & asidmask) != asid)
                //      continue;
                //      continue;

                /*
                 * Only print entries in use
                 */
                printf("Index: %2d pgmask=%s ", i, msk2str(pagemask));

                c0 = (entrylo0 & CP0_ENTRYLO_C) >> 3;
                c1 = (entrylo1 & CP0_ENTRYLO_C) >> 3;


                printf("va=0x%lx asid=0x%x",
                       /*vwidth,*/ (entryhi & ~0x1fffUL),
                       /*asidwidth,*/ entryhi & asidmask);

                pa = entrylo0;
                pa = (pa << 6) & PAGE_MASK(msk2offset(pagemask));
                printf("\n\t[");                
                printf("pa=0x%x c=%d d=%d v=%d g=%d] [",
                       /*pwidth,*/ pa, c0,
                       (entrylo0 & CP0_ENTRYLO_D) ? 1 : 0,
                       (entrylo0 & CP0_ENTRYLO_V) ? 1 : 0,
                       (entrylo0 & CP0_ENTRYLO_G) ? 1 : 0);

                pa = entrylo1;
                pa = (pa << 6) & PAGE_MASK(msk2offset(pagemask));
                
                printf("pa=0x%x c=%d d=%d v=%d g=%d]\n",
                       /*pwidth,*/ pa, c1,
                       (entrylo1 & CP0_ENTRYLO_D) ? 1 : 0,
                       (entrylo1 & CP0_ENTRYLO_V) ? 1 : 0,
                       (entrylo1 & CP0_ENTRYLO_G) ? 1 : 0);
        }
        printf("\n");

        mtc0(CP0_ENTRYHI,  s_entryhi);
        mtc0(CP0_INDEX,    s_index);
        mtc0(CP0_PAGEMASK, s_pagemask);
}


void pok_mips_tlb_print() {
    int limit = jet_mips_tlb_get_index;
    printf("DEBUG:   TLB pages = %d\n", limit + 1);
    printf("DEBUG:   -----------------\r\n");
    for (int i = 0; i <= limit; i++) {
        unsigned valid;
        unsigned tsize;
        uint32_t epn;
        uint64_t rpn0;
        uint64_t rpn1;
        pok_mips_tlb_read_entry(
                i,
                &valid,
                &tsize,
                &epn,
                &rpn0,
                &rpn1);
        if (valid) {
            printf("DEBUG:   Index = %d\n", i);
            printf("DEBUG:   Valid\r\n");
            printf("DEBUG:   Effective: %p\r\n", (void*)epn);
            // FIXME This is wrong. We print only 32 bits out of 36
            printf("DEBUG:   Physical 0:  %p\r\n",
                    (void*)(unsigned)rpn0);
            printf("DEBUG:   Physical 1:  %p\r\n",
                    (void*)(unsigned)rpn1);
            printf("DEBUG:   Size:      %s\r\n", msk2str(tsize));
            printf("DEBUG:   -----------------\r\n");
        }
    }
}

