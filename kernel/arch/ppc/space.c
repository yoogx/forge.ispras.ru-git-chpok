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


// TODO Fix space.c to compile with 64 bit address

#include <config.h>

#include <types.h>
#include <errno.h>
#include <libc.h>
#include <bsp_common.h>
#include <core/sched.h>
#include <core/debug.h>

#include <arch.h>
#include "context.h"
#include "interrupt_context.h"
#include "msr.h"
#include "reg.h"
#include "mmu.h"
#include "space.h"
#include "cons.h"
#include "core/partition.h"
#include "core/partition_arinc.h"
#include "core/error.h"

pok_ret_t ja_space_create (uint8_t space_id,
                            uintptr_t addr,
                            size_t size)
{
#ifdef POK_NEEDS_DEBUG
  printf ("pok_create_space space_id=%d: phys=%x size=%x\n", space_id, addr, size);
#endif
  spaces[space_id].phys_base = addr;
  spaces[space_id].size = size;

  return (POK_ERRNO_OK);
}

pok_ret_t ja_space_switch (uint8_t space_id)
{
    mtspr(SPRN_PID, space_id + 1);

    return POK_ERRNO_OK;
}

uint8_t ja_space_get_current (void)
{
    return ((uint8_t)mfspr(SPRN_PID)) - 1;
}


uintptr_t ja_space_base_vaddr(uintptr_t addr)
{
    (void) addr;
    return POK_PARTITION_MEMORY_BASE;
}
    
struct jet_context* ja_space_context_init(
        uint32_t sp,
        uint8_t space_id,
        uint32_t entry_rel,
        uint32_t stack_rel,
        uint32_t arg1,
        uint32_t arg2)
{
    struct jet_interrupt_context *vctx = (struct jet_interrupt_context*)
        (sp - sizeof (*vctx));
    struct jet_context *ctx = (struct jet_context*)
        (sp - (sizeof (*vctx) + sizeof(*ctx)));

    memset (ctx, 0, sizeof(*ctx));
    memset (vctx, 0, sizeof(*vctx));

    extern void pok_arch_rfi (void);

    /* Fill interrupt frame */
    vctx->r3     = arg1;
    vctx->r4     = arg2;
    vctx->r1     = stack_rel - 12;
    vctx->srr0   = entry_rel;
    vctx->srr1   = MSR_EE | MSR_IP | MSR_PR | MSR_FP;

    // Linkage between frames
    jet_stack_frame_link(&vctx->stack_frame, &ctx->stack_frame, pok_arch_rfi);

    return ctx;
}

static unsigned next_resident = 0;
static unsigned next_non_resident = 0;

/*
 * Returns next available TLB1 entry index.
 *
 * If is_resident is true, entry will never be overwritten.
 *
 * First N entries are "resident" (kernel, devices, all that stuff), and others are evicted
 * by primitive round-robin algorithm.
 *
 * Note that the first request for resident TLB1 entry
 * returns the entry occupidied by the kernel.
 * This is intentional, as we have to overwrite it with 
 * appropriate access rights.
 */
int pok_get_next_tlb1_index(int is_resident)
{

    unsigned res;
    unsigned available_space = pok_ppc_tlb_get_nentry(1); // mfspr(SPRN_TLB1CFG) & TLBnCFG_N_ENTRY_MASK;

    if (is_resident) {
        if (next_resident >= available_space) {
            pok_fatal("Out of TLB1 space");
        }

        res = next_resident++;
        next_non_resident = next_resident;
    } else {
        if (next_non_resident >= available_space) {
            // wrap around
            next_non_resident = next_resident;
        }
        if (next_non_resident >= available_space) {
            pok_fatal("Out of TLB1 space");
        }
        res = next_non_resident++;
    }
    
    return res;
}

/*
 *  Quotes from the manual:
 *
 *      64-entry, fully-associative unified (for instruction and data accesses) L2 TLB array (TLB1)
 *      supports the 11 VSP page sizes shown in Section 6.2.3, “Variable-Sized Pages.”
 *
 *      The replacement algorithm for TLB1 must be implemented completely by the system software. Thus,
 *      when an entry in TLB1 is to be replaced, the software selects which entry to replace and writes the entry
 *      number to MAS0[ESEL] before executing a tlbwe instruction.
 */
void pok_insert_tlb1(
        uint64_t virtual, 
        uint64_t physical, 
        unsigned pgsize_enum, 
        unsigned permissions,
        unsigned wimge,
        unsigned pid,
        pok_bool_t is_resident)
{
    /*
     * TLB1 can be written by first writing the necessary information into MAS0–MAS3, MAS5, MAS7, and
     * MAS8 using mtspr and then executing the tlbwe instruction. To write an entry into TLB1,
     * MAS0[TLBSEL] must be equal to 1, and MAS0[ESEL] must point to the desired entry. When the tlbwe
     * instruction is executed, the TLB entry information stored in MAS0–MAS3, MAS5, MAS7, and MAS8 is
     * written into the selected TLB entry in the TLB1 array.
     */
    
    unsigned entry;

    entry = pok_get_next_tlb1_index(is_resident);
    pok_ppc_tlb_write(1,
        virtual, 
        physical, 
        pgsize_enum, 
        permissions,
        wimge,
        pid,
        entry,
        TRUE);
}

static inline const char* pok_ppc_tlb_size(unsigned size)
{
    switch (size) {
#define CASE(x) case E500MC_PGSIZE_##x: return #x; 
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
        default:
        return "Unknown";
    }
}

/*
 *  Quote from the manual:
 *
 *      A 512-entry, 4-way set-associative unified (for instruction and data accesses) L2 TLB array
 *      (TLB0) supports only 4-Kbyte pages.
 *
 *      TLB0 entry replacement is also implemented by software. To assist the software with TLB0 replacement,
 *      the core provides a hint that can be used for implementing a round-robin replacement algorithm. <...>
 */
// XXX not implemented
void pok_insert_tlb0();

static int pok_ccsrbar_ready = 0;

static void pok_ppc_tlb_print(unsigned tlbsel) {
    unsigned limit = pok_ppc_tlb_get_nentry(1);

    for (unsigned i = 0; i < limit; i++) {
        unsigned valid;
        unsigned tsize; 
        uint32_t epn;
        uint64_t rpn;
        pok_ppc_tlb_read_entry(tlbsel, i,
                &valid,
                &tsize,
                &epn,
                &rpn
                );
        //~ if (valid) {
            //~ printf("DEBUG: tlb entry %d:%d:\r\n", tlbsel, i);
            //~ printf("DEBUG:   Valid\r\n");
            //~ printf("DEBUG:   Effective: %p\r\n", (void*)epn);
            //~ // FIXME This is wrong. We print only 32 bits out of 36
            //~ printf("DEBUG:   Physical: %x:%p\r\n", 
                    //~ (unsigned)(rpn>>32), (void*)(unsigned)rpn);
            //~ printf("DEBUG:   Size: %s\r\n", pok_ppc_tlb_size(tsize));
//~ 
        //~ }
    }
}

void pok_arch_space_init (void)
{
    // overwrites first TLB1 entry
    // we just need to change access bits for the kernel,
    // so user won't be able to access it
    pok_insert_tlb1(
        0,
        0,
        E500MC_PGSIZE_256M,  //TODO make smaller
        MAS3_SW | MAS3_SR | MAS3_SX,
        0,
        0, // any pid 
        TRUE
    );
    /*
     * Clear all other mappings. For instance, those created by u-boot.
     */
    unsigned limit = pok_ppc_tlb_get_nentry(1);
    pok_ppc_tlb_write(1,
            pok_bsp.ccsrbar_base, pok_bsp.ccsrbar_base_phys, E500MC_PGSIZE_16M,
            //MAS3_SW | MAS3_SR | MAS3_SX,
            MAS3_SW | MAS3_SR | MAS3_SX | MAS3_UW | MAS3_UR,
            MAS2_W | MAS2_I | MAS2_M | MAS2_G,
            0,
            limit-1,
            TRUE
            );
    pok_ccsrbar_ready = 1;

    pok_ppc_tlb_print(0);
    pok_ppc_tlb_print(1);
//    pok_ppc_tlb_clear_entry(1, 2);
    for (unsigned i = 1; i < limit-1; i++) {
        pok_ppc_tlb_clear_entry(1, i);
    }

    // DIRTY HACK
    // By some reason P3041 DUART blocks when TLB entry #1 is overrriden.
    // Preserve it, let's POK write it's entries starting 2
    next_non_resident = next_resident = 2;
    //
}

//TODO get this values from devtree!
#define MPC8544_PCI_IO_SIZE      0x10000ULL
#define MPC8544_PCI_IO           0xE1000000ULL

void pok_arch_handle_page_fault(
        struct jet_interrupt_context *vctx,
        uintptr_t faulting_address,
        uint32_t syndrome,
        pf_type_t type)
{
    int tlb_miss = (type == PF_INST_TLB_MISS || type == PF_DATA_TLB_MISS);
    unsigned pid = mfspr(SPRN_PID);

    if (tlb_miss && faulting_address >= pok_bsp.ccsrbar_base && faulting_address < pok_bsp.ccsrbar_base + pok_bsp.ccsrbar_size) {
        pok_insert_tlb1(
            pok_bsp.ccsrbar_base, 
            pok_bsp.ccsrbar_base_phys, 
            E500MC_PGSIZE_16M, 
            //MAS3_SW | MAS3_SR,
            MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR,
            MAS2_W | MAS2_I | MAS2_M | MAS2_G,
            0, /* any pid */
            TRUE 
        );
    } else if (tlb_miss && faulting_address >= MPC8544_PCI_IO && faulting_address < MPC8544_PCI_IO + MPC8544_PCI_IO_SIZE) {
        pok_insert_tlb1(
            MPC8544_PCI_IO,
            MPC8544_PCI_IO,
            E500MC_PGSIZE_64K,
            //MAS3_SW | MAS3_SR,
            MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR,
            MAS2_W | MAS2_I | MAS2_M | MAS2_G,
            0, /* any pid */
            TRUE
        );
    } else if (
            tlb_miss &&
            pid != 0 &&
            faulting_address >= POK_PARTITION_MEMORY_BASE &&
            faulting_address < POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE)
    {
        uint8_t space_id = pid - 1;

        pok_insert_tlb1( 
            POK_PARTITION_MEMORY_BASE,
            spaces[space_id].phys_base,
            E500MC_PGSIZE_16M,
            MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX,
            0,
            pid,
            FALSE
        );
    } else {
        if (vctx->srr1 & MSR_PR) {
            printf("USER ");
        } else {
            printf("KERNEL ");
        }

        if (type == PF_DATA_TLB_MISS || type == PF_DATA_STORAGE) {
            printf("code at %p address tried to access data on %p address\n", (void*)vctx->srr0, (void*)faulting_address);
        } else {
            printf("code at %p address tried to execute code at %p address\n", (void *)vctx->lr, (void*)vctx->srr0);
        }
#ifdef PARTITION_DEBUG_MODE
        pok_fatal("page fault");
#else
        printf("raising error in pagefault addr %p  syndrome 0x%lx\n", (void*) faulting_address, syndrome);
        POK_ERROR_CURRENT_THREAD(POK_ERROR_KIND_MEMORY_VIOLATION);
#endif
        //pok_fatal("bad memory access");
    }
}

//Double check here because these function are called not only in syscall
//(where there is checking), but also inside kernel
//TODO: maybe rename to pok_arch_?
uintptr_t pok_virt_to_phys(uintptr_t virt)
{
    pok_partition_id_t partid = mfspr(SPRN_PID) - 1;
    if (!POK_CHECK_PTR_IN_PARTITION(partid, virt)) {
        printf("pok_virt_to_phys: wrong virtual address %p\n", (void*)virt);
        pok_fatal("wrong pointer in pok_virt_to_phys\n");
    }

    return virt - POK_PARTITION_MEMORY_BASE + spaces[partid].phys_base;
}

uintptr_t pok_phys_to_virt(uintptr_t phys)
{
    pok_partition_id_t partid = mfspr(SPRN_PID) - 1;

    uintptr_t virt = phys - spaces[partid].phys_base + POK_PARTITION_MEMORY_BASE;
    if (!POK_CHECK_PTR_IN_PARTITION(partid, virt)) {
        printf("pok_phys_to_virt: wrong virtual address %p\n", (void*)virt);
        pok_fatal("wrong pointer in pok_phys_to_virt\n");
    }
    return virt;
}
