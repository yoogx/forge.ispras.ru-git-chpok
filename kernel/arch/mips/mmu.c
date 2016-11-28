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
#include "msr.h"

#include <assert.h>

/*
 *  Quotes from the manual:
 *
 *      64-entry, fully-associative unified (for instruction and data accesses) L2 TLB array (TLB1)
 *      supports the 11 VSP page sizes shown in Section 6.2.3, “Variable-Sized Pages.”
 *
 * TODO: document parameters
 */
void pok_mips_tlb_write(
        uint32_t virtual, 
        uint64_t physical, 
        unsigned pgsize_enum, 
        unsigned permissions,
        unsigned wimge,
        unsigned pid,
        pok_bool_t  valid
        )
{
    /*
     * TLB can be written by first writing the necessary information into EntryLo1, EntryLo2, EntryHi and
     * PageMask using mtc0 and then executing the tlbwr instruction.
     */
    
    mtc0(CP0_PAGEMASK, pgsize_enum);
    
    if (pid == 0) //any pid 
    {
        mtc0(CP0_ENTRYLO0, EntryLo0_G(0));
        mtc0(CP0_ENTRYLO1, EntryLo1_G(0));
    }else{
        mtc0(CP0_ENTRYLO0, EntryLo0_G(1));
        mtc0(CP0_ENTRYLO1, EntryLo1_G(1));
    }        
    
    mtc0(CP0_ENTRYHI,  EntryHi_ASID(pid));
    mtc0(CP0_ENTRYHI,  EntryHi_VPN2(virtual));
    mtc0(CP0_ENTRYLO0, EntryLo0_PFN(physical));
    mtc0(CP0_ENTRYLO0, EntryLo0_D(permissions) | EntryLo0_V(permissions));
    mtc0(CP0_ENTRYLO1, EntryLo1_PFN(physical));
    mtc0(CP0_ENTRYLO1, EntryLo1_D(permissions) | EntryLo1_V(permissions));
    
    //~ int test = 0;
    //~ asm volatile("synci 0(%0)":: "r" (test));
    asm volatile("tlbwr":::"memory");
    //~ asm volatile("synci 0(%0)":: "r" (test));


//~ 
    //~ uint32_t mas0, mas1, mas2, mas3, mas7;
//~ 
    //~ assert(tlbsel <= 1) ;
//~ 
    //~ mas0 = MAS0_TLBSEL(tlbsel) | MAS0_ESEL(entry);
    //~ mas1 = ((valid != 0)? MAS1_VALID : 0) | MAS1_TID(pid) | MAS1_TSIZE(pgsize_enum);
    //~ mas2 = (virtual & MAS2_EPN) | wimge;
    //~ mas3 = (physical & MAS3_RPN) | permissions; 
    //~ mas7 = physical >> 32;

    //~ mtspr(SPRN_MAS0, mas0); 
    //~ mtspr(SPRN_MAS1, mas1); 
    //~ mtspr(SPRN_MAS2, mas2);
    //~ mtspr(SPRN_MAS3, mas3);
    //~ mtspr(SPRN_MAS7, mas7);
    //~ int test = 0;
    //~ asm volatile("synci 0(%0)":: "r" (test));
    //~ asm volatile("synci; tlbwr; synci":::"memory");
}


/*
unsigned pok_mips_get_tlb_nentry(unsigned tlbsel) {
    static unsigned regid[] =  { SPRN_TLB0CFG,
        SPRN_TLB1CFG, 
        SPRN_TLB2CFG, 
        SPRN_TLB3CFG 
    };

    assert(tlbsel < 5);
    unsigned sprn = regid[tlbsel];
    return mfspr(sprn) & TLBnCFG_N_ENTRY_MASK;
}
*/

void pok_mips_tlb_read_entry(
        unsigned *valid, 
        unsigned *tsize, 
        uint32_t *epn,
        uint64_t *rpn)
{
    //~ uint32_t entrylo1, entrylo0, entryhi, pagesize;

    //~ assert(tlbsel <= 1) ;
    //~ entrylo1 = mfc0(CP0_ENTRYLO0);
    //~ entrylo1 = mfc0(CP0_ENTRYLO1);
    //~ entryhi  = mfc0(CP0_ENTRYHI);
    //~ pagesize = mfc0(CP0_PAGEMASK);
    

    //~ mas0 = MAS0_TLBSEL(tlbsel) | MAS0_ESEL(entry);

    //~ mtspr(SPRN_MAS0, mas0);
    //~ asm volatile("tlbre;isync");
    //~ mas1 = mfspr(SPRN_MAS1);

    //~ *valid = (mas1 & MAS1_VALID);
    //~ *tsize = (mas1 >> 7) & 0x1f;
    //~ *epn = mfspr(SPRN_MAS2) & MAS2_EPN;
    //~ *rpn = mfspr(SPRN_MAS3) & MAS3_RPN;
    //~ *rpn |= ((uint64_t)mfspr(SPRN_MAS7)) << 32;
}


 /*
  * PAGE_SHIFT determines the page size
  */
 #define PAGE_MASK(page_shift)       (~((1 << page_shift) - 1))


#define PM_4K           0x000 << 13
#define PM_16K          0x003 << 13
#define PM_64K          0x00f << 13
#define PM_256K         0x03f << 13
#define PM_1M           0x0ff << 13
#define PM_4M           0x3ff << 13
#define PM_16M          0xfff << 13

static inline const char *msk2str(uint32_t mask)
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



//~ #define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

void dump_tlb(int first, int last)
{
        uint32_t s_entryhi, entryhi, asid;
        uint32_t entrylo0, entrylo1, pa;
        uint32_t s_index, s_pagemask;
        uint32_t pagemask, c0, c1, i;
        uint32_t asidmask = 0xff;
        //~ int asidwidth = DIV_ROUND_UP(ilog2(asidmask) + 1, 4);
//~ #ifdef CONFIG_32BIT
        //~ bool xpa = cpu_has_xpa && (read_c0_pagegrain() & PG_ELPA);
        //~ int pwidth = 8;// xpa ? 11 : 8;
        //~ int vwidth = 8;
//~ #else
        //~ bool xpa = false;
        //~ int pwidth = 11;
        //~ int vwidth = 11;
//~ #endif

        s_pagemask = mfc0(CP0_PAGEMASK);
        s_entryhi  = mfc0(CP0_ENTRYHI);
        s_index    = mfc0(CP0_INDEX);
        asid = s_entryhi & asidmask;

        for (i = first; i <= last; i++) {
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
                //~ if ((entryhi & ~0x1ffffUL) == CKSEG0)
                        //~ continue;
                
                /*
                 * ASID takes effect in absence of G (global) bit.
                 * We check both G bits, even though architecturally they should
                 * match one another, because some revisions of the SB1 core may
                 * leave only a single G bit set after a machine check exception
                 * due to duplicate TLB entry.
                 */
                if (!((entrylo0 | entrylo1) & CP0_ENTRYLO_G) &&
                    (entryhi & asidmask) != asid)
                        continue;

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



