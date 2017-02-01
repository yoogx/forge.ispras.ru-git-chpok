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
#include "bsp/bsp.h"
#include <core/sched.h>
#include <core/debug.h>

#include "context.h"
#include "interrupt_context.h"
#include "msr.h"
#include "reg.h"
#include "mmu.h"
#include "space.h"
#include "cons.h"
#include <core/partition.h>
#include <core/partition_arinc.h>
#include <core/error.h>
#include <arch/deployment.h>

size_t ja_ustack_get_alignment(void)
{
    return 16;
}

/*
 * Currently, kernel has rw access to all tlb entries.
 * So switching entries is not required.
 */
void ja_uspace_grant_access(void) {}
void ja_uspace_revoke_access(void) {}

void ja_uspace_grant_access_local(jet_space_id space_id) {(void)space_id;}
void ja_uspace_revoke_access_local(jet_space_id space_id) {(void)space_id;}


void ja_space_switch (jet_space_id space_id)
{
    mtspr(SPRN_PID, space_id);
}

jet_space_id ja_space_get_current (void)
{
    return (jet_space_id)mfspr(SPRN_PID);
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
//    pok_ppc_tlb_clear_entry(1, 2);
    for (unsigned i = 1; i < limit; i++) {
        pok_ppc_tlb_clear_entry(1, i);
    }
    pok_ppc_tlb_write(1,
            pok_bsp.ccsrbar_base, pok_bsp.ccsrbar_base_phys, E500MC_PGSIZE_16M,
            //MAS3_SW | MAS3_SR | MAS3_SX,
            MAS3_SW | MAS3_SR | MAS3_SX | MAS3_UW | MAS3_UR,
            MAS2_W | MAS2_I | MAS2_M | MAS2_G,
            0,
            limit-1,
            TRUE);

    // DIRTY HACK
    // By some reason P3041 DUART blocks when TLB entry #1 is overrriden.
    // Preserve it, let's POK write it's entries starting 2
    next_non_resident = next_resident = 2;

    for (int i = 0; i < tlb_entries_n; i++) {
        struct tlb_entry* entry = &tlb_entries[i];

        pok_insert_tlb1(
            entry->virt_addr,
            entry->phys_addr,
            entry->size,
            entry->permissions,
            entry->cache_policy,
            entry->pid,
            TRUE
            );
    }
}

void pok_arch_handle_page_fault(
        struct jet_interrupt_context *vctx,
        uintptr_t faulting_address,
        uint32_t syndrome,
        pf_type_t type)
{
#ifdef POK_NEEDS_DEBUG
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
#endif
    pok_raise_error(POK_ERROR_ID_MEMORY_VIOLATION, vctx->srr1 & MSR_PR, (void*) faulting_address);
}
