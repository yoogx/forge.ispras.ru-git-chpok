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
#include "core/partition.h"
#include "core/partition_arinc.h"
#include "core/error.h"

//FIXME
#include <arch/deployment.h>


size_t ja_ustack_get_alignment(void)
{
    return 16;
}


//~ struct jet_kernel_shared_data* __kuser ja_space_shared_data(jet_space_id space_id)
//~ {
    //~ return (struct jet_kernel_shared_data* __kuser)POK_PARTITION_MEMORY_BASE;
//~ }
//~ 
//~ static const size_t ja_user_space_maximum_alignment = 16;
	//~ 
	//~ 
//~ void __user* ja_space_get_heap(jet_space_id space_id)
//~ {
    //~ struct ja_mips_space * space = &ja_spaces[space_id - 1];
//~ 
    //~ return POK_PARTITION_MEMORY_BASE + (char __user*)
    //~ ALIGN_VAL((unsigned long)space->size_normal, ja_user_space_maximum_alignment);
//~ }


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
    //~ INT_ENABLE
    //~ while (1==1){
    //~ 
    //~ }
    mtc0(CP0_ENTRYHI, space_id);
}

jet_space_id ja_space_get_current (void)
{
    return (jet_space_id)   (mfc0(CP0_ENTRYHI) & 0xf);
}
    
static inline const char* pok_mips_tlb_size(unsigned size)
{
    switch (size) {
#define CASE(x) case MIPS_PGSIZE_##x: return #x;
        CASE(4K);
        CASE(16K);
        CASE(64K);
        CASE(256K);
        CASE(1M);
        CASE(4M);
        CASE(16M);
#undef CASE
        default:
        return "Unknown";
    }
}

//~ static void pok_mips_tlb_clear(        
        //~ uint32_t virtual, 
        //~ uint64_t physical, 
        //~ unsigned pgsize_enum, 
        //~ unsigned permissions,
        //~ unsigned wimge,
        //~ unsigned pid,
        //~ pok_bool_t  valid)
//~ {
        //~ int tlb_index = jet_mips_tlb_get_index;
        //~ 
        //~ for (int i = tlb_index + 1; i < MIPS_MAX_TLB_SIZE; i ++){
            //~ pok_mips_tlb_write(        
                //~ virtual, 
                //~ physical, 
                //~ pgsize_enum, 
                //~ permissions,
                //~ wimge,
                //~ pid,
                //~ valid);
        //~ }
        //~ jet_mips_tlb_get_index = tlb_index;
//~ }


/*
 *  Quote from the manual:
 *
 *      A 512-entry, 4-way set-associative unified (for instruction and data accesses) L2 TLB array
 *      (TLB0) supports only 4-Kbyte pages.
 *
 *      TLB0 entry replacement is also implemented by software. To assist the software with TLB0 replacement,
 *      the core provides a hint that can be used for implementing a round-robin replacement algorithm. <...>
 */

//~ static unsigned next_resident = 0;
//~ static unsigned next_non_resident = 0;

extern void dmfc0_asm(void);

void pok_arch_space_init (void)
{
    printf("Hello!\n");
    /*
     * Mapping for serial IN/OUT
     */
    pok_mips_tlb_write(
        pok_bsp.ccsrbar_base_phys,
        pok_bsp.ccsrbar_base_phys,
        MIPS_PGSIZE_1M,  //TODO make smaller
        EntryLo_D | EntryLo_V, //TODO Save it from user read/write 
        0,
        0, // any pid
        TRUE
    );


    printf("EBASE = 0x%lx\n", mfc0(CP0_EBASE));

    printf("Status = 0x%lx\n", mfsr());
    printf("jet_mips_tlb_get_index = %d\n", jet_mips_tlb_get_index);
    pok_mips_tlb_print();


    for(int i = 0; i < tlb_entries_n; i++)
    {
        struct tlb_entry* entry = &tlb_entries[i];

        pok_mips_tlb_write(
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
    if (vctx->STATUS & CP0_STATUS_KSU_1) {
        printf("USER ");
    } else {
        printf("KERNEL ");
    }

    //~ if (type == PF_DATA_TLB_MISS || type == PF_DATA_STORAGE) {
        printf("code at %p address tried to access/execute data on %p address\n", (void*)vctx->EPC, (void*)faulting_address);
    //~ } else {
        //~ printf("code at %p address tried to execute code at %p address\n", (void *)vctx->r31, (void*)vctx->EPC);
    //~ }
#endif
    pok_raise_error(POK_ERROR_ID_MEMORY_VIOLATION, vctx->STATUS & CP0_STATUS_KSU_1, (void*) faulting_address);
}
//~ uintptr_t pok_virt_to_phys(uintptr_t virt)
//~ {
    //~ if((virt < POK_PARTITION_MEMORY_BASE)
        //~ || (virt > POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE))
    //~ {
        //~ // Fatal error despite it is called from user space!!
        //~ printf("pok_virt_to_phys: wrong virtual address %p\n", (void*)virt);
        //~ pok_fatal("wrong pointer in pok_virt_to_phys\n");
    //~ }
//~ 
    //~ jet_space_id space_id = ja_space_get_current();
//~ 
    //~ return virt - POK_PARTITION_MEMORY_BASE + ja_spaces[space_id - 1].phys_base;
//~ }
//~ 
//~ uintptr_t pok_phys_to_virt(uintptr_t phys)
//~ {
    //~ jet_space_id space_id = ja_space_get_current();
//~ 
    //~ if((phys < ja_spaces[space_id - 1].phys_base)
        //~ || (phys >= ja_spaces[space_id - 1].phys_base + POK_PARTITION_MEMORY_SIZE))
    //~ {
        //~ // Fatal error despite it is called from user space!!
        //~ printf("pok_phys_to_virt: wrong physical address %p\n", (void*)phys);
        //~ pok_fatal("wrong pointer in pok_phys_to_virt\n");
    //~ }
//~ 
    //~ return phys - ja_spaces[space_id - 1].phys_base + POK_PARTITION_MEMORY_BASE;
//~ }
