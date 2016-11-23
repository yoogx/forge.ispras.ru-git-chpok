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
void ja_space_layout_get(jet_space_id space_id,
    struct jet_space_layout* space_layout)
{
    assert(space_id != 0 && space_id <= ja_spaces_n);

    space_layout->kernel_addr = (char*) POK_PARTITION_MEMORY_BASE;
    space_layout->user_addr = (char*) POK_PARTITION_MEMORY_BASE;
    space_layout->size = ja_spaces[space_id - 1].size_normal;
}


struct jet_kernel_shared_data* __kuser ja_space_shared_data(jet_space_id space_id)
{
    return (struct jet_kernel_shared_data* __kuser)POK_PARTITION_MEMORY_BASE;
}

size_t ja_user_space_maximum_alignment = 16;

void ja_space_switch (jet_space_id space_id)
{
    //~ mtspr(SPRN_PID, space_id);
}

jet_space_id ja_space_get_current (void)
{
    return 0;//~(jet_space_id)mfspr(SPRN_PID);
}


void ja_ustack_init (jet_space_id space_id)
{
    assert(space_id != 0);

    ja_spaces[space_id - 1].ustack_state = POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE - 16;
}

jet_ustack_t ja_ustack_alloc (jet_space_id space_id, size_t stack_size)
{
    assert(space_id != 0);

    uint32_t* ustack_state_p = &ja_spaces[space_id - 1].ustack_state;

    size_t size_real = ALIGN_VAL(stack_size, 16);

    // TODO: Check boundaries.
    jet_ustack_t result = *ustack_state_p;

    *ustack_state_p -= size_real;

    return result;
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
    unsigned available_space = pok_mips_tlb_get_nentry(1); // mfspr(SPRN_TLB1CFG) & TLBnCFG_N_ENTRY_MASK;

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



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//~ 
//~ 
//~ 
//~ extern void build_tlb_refill_handler(void);
//~ 
//~ /*
 //~ * LOONGSON-2 has a 4 entry itlb which is a subset of jtlb, LOONGSON-3 has
 //~ * a 4 entry itlb and a 4 entry dtlb which are subsets of jtlb. Unfortunately,
 //~ * itlb/dtlb are not totally transparent to software.
 //~ */
//~ static inline void flush_micro_tlb(void)
//~ {
    //~ mtc0(CP0_ChipMemCtrl, LOONGSON_DIAG_ITLB | LOONGSON_DIAG_DTLB);
    //~ break;
//~ }
//~ 
//~ static inline void flush_micro_tlb_vm(struct vm_area_struct *vma)
//~ {
        //~ if (vma->vm_flags & VM_EXEC)
                //~ flush_micro_tlb();
//~ }
//~ 
//~ void local_flush_tlb_all(void)
//~ {
        //~ unsigned long flags;
        //~ unsigned long old_ctx;
        //~ int entry, ftlbhighset;
//~ 
        //~ local_irq_save(flags);
        //~ /* Save old context and create impossible VPN2 value */
        //~ old_ctx = mfc0(CP0_ENTRYHI);
        //~ htw_stop();
        //~ mtc0(CP0_ENTRYLO0, 0);
        //~ mtc0(CP0_ENTRYLO1, 0);
//~ 
        //~ entry = mfc0(CP0_WIRED);
//~ 
        //~ /* Blast 'em all away. */
        //~ if (cpu_has_tlbinv) {
                //~ if (current_cpu_data.tlbsizevtlb) {
                        //~ mtc0(CP0_INDEX, 0);
                        //~ 
                        //~ tlbinvf();  /* invalidate VTLB */
                //~ }
                //~ ftlbhighset = current_cpu_data.tlbsizevtlb +
                        //~ current_cpu_data.tlbsizeftlbsets;
                //~ for (entry = current_cpu_data.tlbsizevtlb;
                     //~ entry < ftlbhighset;
                     //~ entry++) {
                        //~ mtc0(CP0_INDEX, entry);
                        //~ 
                        //~ tlbinvf();  /* invalidate one FTLB set */
                //~ }
        //~ } else {
                //~ while (entry < current_cpu_data.tlbsize) {
                        //~ /* Make sure all entries differ. */
                        //~ mtc0(CP0_ENTRYHI, UNIQUE_ENTRYHI(entry));
                        //~ mtc0(CP0_INDEX, entry);
                        //~ 
                        //~ tlb_write_indexed();
                        //~ entry++;
                //~ }
        //~ }
        //~ 
        //~ mtc0(CP0_ENTRYHI, old_ctx);
        //~ htw_start();
        //~ flush_micro_tlb();
        //~ local_irq_restore(flags);
//~ }
//~ EXPORT_SYMBOL(local_flush_tlb_all);
//~ 
//~ /* All entries common to a mm share an asid.  To effectively flush
   //~ these entries, we just bump the asid. */
//~ void local_flush_tlb_mm(struct mm_struct *mm)
//~ {
        //~ int cpu;
//~ 
        //~ preempt_disable();
//~ 
        //~ cpu = smp_processor_id();
//~ 
        //~ if (cpu_context(cpu, mm) != 0) {
                //~ drop_mmu_context(mm, cpu);
        //~ }
//~ 
        //~ preempt_enable();
//~ }
//~ 
//~ void local_flush_tlb_range(struct vm_area_struct *vma, unsigned long start,
        //~ unsigned long end)
//~ {
        //~ struct mm_struct *mm = vma->vm_mm;
        //~ int cpu = smp_processor_id();
//~ 
        //~ if (cpu_context(cpu, mm) != 0) {
                //~ unsigned long size, flags;
//~ 
                //~ local_irq_save(flags);
                //~ start = round_down(start, PAGE_SIZE << 1);
                //~ end = round_up(end, PAGE_SIZE << 1);
                //~ size = (end - start) >> (PAGE_SHIFT + 1);
                //~ if (size <= (current_cpu_data.tlbsizeftlbsets ?
                             //~ current_cpu_data.tlbsize / 8 :
                             //~ current_cpu_data.tlbsize / 2)) {
                        //~ int oldpid = mfc0(CP0_ENTRYHI);
                        //~ int newpid = cpu_asid(cpu, mm);
//~ 
                        //~ htw_stop();
                        //~ while (start < end) {
                                //~ int idx;
//~ 
                                //~ mtc0(CP0_ENTRYHI, start | newpid);
                                //~ start += (PAGE_SIZE << 1);
                                //~ 
                                //~ __asm__ __volatile__("tlbp");
                                //~ 
                                //~ idx = mfc0(CP0_INDEX);
                                //~ mtc0(CP0_ENTRYLO0, 0);
                                //~ mtc0(CP0_ENTRYLO1, 0);
                                //~ if (idx < 0)
                                        //~ continue;
                                //~ /* Make sure all entries differ. */
                                //~ mtc0(CP0_ENTRYHI, UNIQUE_ENTRYHI(idx));
                                //~ 
                                //~ tlb_write_indexed();
                        //~ }
                        //~ 
                        //~ mtc0(CP0_ENTRYHI, oldpid);
                        //~ htw_start();
                //~ } else {
                        //~ drop_mmu_context(mm, cpu);
                //~ }
                //~ flush_micro_tlb();
                //~ local_irq_restore(flags);
        //~ }
//~ }
//~ 
//~ void local_flush_tlb_kernel_range(unsigned long start, unsigned long end)
//~ {
        //~ unsigned long size, flags;
//~ 
        //~ local_irq_save(flags);
        //~ size = (end - start + (PAGE_SIZE - 1)) >> PAGE_SHIFT;
        //~ size = (size + 1) >> 1;
        //~ if (size <= (current_cpu_data.tlbsizeftlbsets ?
                     //~ current_cpu_data.tlbsize / 8 :
                     //~ current_cpu_data.tlbsize / 2)) {
                //~ int pid = mfc0(CP0_ENTRYHI);
//~ 
                //~ start &= (PAGE_MASK << 1);
                //~ end += ((PAGE_SIZE << 1) - 1);
                //~ end &= (PAGE_MASK << 1);
                //~ htw_stop();
//~ 
                //~ while (start < end) {
                        //~ int idx;
//~ 
                        //~ mtc0(CP0_ENTRYHI, start);
                        //~ start += (PAGE_SIZE << 1);
                        //~ 
                        //~ __asm__ __volatile__("tlbp");
                        //~ 
                        //~ idx = mfc0(CP0_INDEX);
                        //~ mtc0(CP0_ENTRYLO0, 0);
                        //~ mtc0(CP0_ENTRYLO1, 0);
                        //~ if (idx < 0)
                                //~ continue;
                        //~ /* Make sure all entries differ. */
                        //~ mtc0(CP0_ENTRYHI, UNIQUE_ENTRYHI(idx));
                        //~ 
                        //~ tlb_write_indexed();
                //~ }
                //~ 
                //~ mtc0(CP0_ENTRYHI, pid);
                //~ htw_start();
        //~ } else {
                //~ local_flush_tlb_all();
        //~ }
        //~ flush_micro_tlb();
        //~ local_irq_restore(flags);
//~ }
//~ 
//~ void local_flush_tlb_page(struct vm_area_struct *vma, unsigned long page)
//~ {
        //~ int cpu = smp_processor_id();
//~ 
        //~ if (cpu_context(cpu, vma->vm_mm) != 0) {
                //~ unsigned long flags;
                //~ int oldpid, newpid, idx;
//~ 
                //~ newpid = cpu_asid(cpu, vma->vm_mm);
                //~ page &= (PAGE_MASK << 1);
                //~ local_irq_save(flags);
                //~ oldpid = mfc0(CP0_ENTRYHI);
                //~ htw_stop();
                //~ mtc0(CP0_ENTRYHI, page | newpid);
                //~ 
                //~ __asm__ __volatile__("tlbp");
                //~ 
                //~ idx = mfc0(CP0_INDEX);
                //~ mtc0(CP0_ENTRYLO0, 0);
                //~ mtc0(CP0_ENTRYLO1, 0);
                //~ if (idx < 0)
                        //~ goto finish;
                //~ /* Make sure all entries differ. */
                //~ mtc0(CP0_ENTRYHI, UNIQUE_ENTRYHI(idx));
                //~ 
                //~ tlb_write_indexed();
                //~ 
//~ 
        //~ finish:
                //~ mtc0(CP0_ENTRYHI, oldpid);
                //~ htw_start();
                //~ flush_micro_tlb_vm(vma);
                //~ local_irq_restore(flags);
        //~ }
//~ }
//~ 
//~ /*
 //~ * This one is only used for pages with the global bit set so we don't care
 //~ * much about the ASID.
 //~ */
//~ void local_flush_tlb_one(unsigned long page)
//~ {
        //~ unsigned long flags;
        //~ int oldpid, idx;
//~ 
        //~ local_irq_save(flags);
        //~ oldpid = mfc0(CP0_ENTRYHI);
        //~ htw_stop();
        //~ page &= (PAGE_MASK << 1);
        //~ mtc0(CP0_ENTRYHI, page);
        //~ 
        //~ __asm__ __volatile__("tlbp");
        //~ 
        //~ idx = mfc0(CP0_INDEX);
        //~ mtc0(CP0_ENTRYLO0, 0);
        //~ mtc0(CP0_ENTRYLO1, 0);
        //~ if (idx >= 0) {
                //~ /* Make sure all entries differ. */
                //~ mtc0(CP0_ENTRYHI, UNIQUE_ENTRYHI(idx));
                //~ 
                //~ tlb_write_indexed();
                //~ 
        //~ }
        //~ mtc0(CP0_ENTRYHI, oldpid);
        //~ htw_start();
        //~ flush_micro_tlb();
        //~ local_irq_restore(flags);
//~ }
//~ 
//~ /*
 //~ * We will need multiple versions of update_mmu_cache(), one that just
 //~ * updates the TLB with the new pte(s), and another which also checks
 //~ * for the R4k "end of page" hardware bug and does the needy.
 //~ */
//~ void __update_tlb(struct vm_area_struct * vma, unsigned long address, pte_t pte)
//~ {
        //~ unsigned long flags;
        //~ pgd_t *pgdp;
        //~ pud_t *pudp;
        //~ pmd_t *pmdp;
        //~ pte_t *ptep;
        //~ int idx, pid;
//~ 
        //~ /*
         //~ * Handle debugger faulting in for debugee.
         //~ */
        //~ if (current->active_mm != vma->vm_mm)
                //~ return;
//~ 
        //~ local_irq_save(flags);
//~ 
        //~ htw_stop();
        //~ pid = mfc0(CP0_ENTRYHI) & cpu_asid_mask(&current_cpu_data);
        //~ address &= (PAGE_MASK << 1);
        //~ mtc0(CP0_ENTRYHI, address | pid);
        //~ pgdp = pgd_offset(vma->vm_mm, address);
        //~ 
         //~ __asm__ __volatile__("tlbp");
        //~ 
        //~ pudp = pud_offset(pgdp, address);
        //~ pmdp = pmd_offset(pudp, address);
        //~ idx = mfc0(CP0_INDEX);
            //~ ptep = pte_offset_map(pmdp, address);
//~ 
//~ 
            //~ mtc0(CP0_ENTRYLO0, ptep->pte_high);
            //~ ptep++;
            //~ mtc0(CP0_ENTRYLO1, ptep->pte_high);
            //~ 
            //~ if (idx < 0)
                    //~ tlb_write_random();
            //~ else
                    //~ tlb_write_indexed();
        //~ 
        //~ htw_start();
        //~ flush_micro_tlb_vm(vma);
        //~ local_irq_restore(flags);
//~ }
//~ 
//~ void add_wired_entry(unsigned long entrylo0, unsigned long entrylo1,
                     //~ unsigned long entryhi, unsigned long pagemask)
//~ {
        //~ unsigned long flags;
        //~ unsigned long wired;
        //~ unsigned long old_pagemask;
        //~ unsigned long old_ctx;
//~ 
        //~ local_irq_save(flags);
        //~ /* Save old context and create impossible VPN2 value */
        //~ old_ctx = mfc0(CP0_ENTRYHI);
        //~ htw_stop();
        //~ old_pagemask = mfc0(CP0_PAGEMASK);
        //~ wired = mfc0(CP0_WIRED);
        //~ mtc0(CP0_WIRED, wired + 1);
        //~ mtc0(CP0_INDEX, wired);
        //~ 
        //~ mtc0(CP0_PAGEMASK, pagemask);
        //~ mtc0(CP0_ENTRYHI, entryhi);
        //~ mtc0(CP0_ENTRYLO0, entrylo0);
        //~ mtc0(CP0_ENTRYLO1, entrylo1);
        //~ 
        //~ tlb_write_indexed();
        //~ 
//~ 
        //~ mtc0(CP0_ENTRYHI, old_ctx);
         //~ 
        //~ htw_start();
        //~ mtc0(CP0_PAGEMASK, old_pagemask);
        //~ local_flush_tlb_all();
        //~ local_irq_restore(flags);
//~ }

//~ /*
 //~ * Used for loading TLB entries before trap_init() has started, when we
 //~ * don't actually want to add a wired entry which remains throughout the
 //~ * lifetime of the system
 //~ */
//~ 
//~ int temp_tlb_entry;
//~ 
//~ ////~ __init int add_temporary_entry(unsigned long entrylo0, unsigned long entrylo1,
//~ //                               //~ unsigned long entryhi, unsigned long pagemask)
//~ ////~ {
//~ //        //~ int ret = 0;
//~ //        //~ unsigned long flags;
//~ //        //~ unsigned long wired;
//~ //        //~ unsigned long old_pagemask;
//~ //        //~ unsigned long old_ctx;
//~ ////~ 
//~ //        //~ local_irq_save(flags);
//~ //        //~ /* Save old context and create impossible VPN2 value */
//~ //        //~ htw_stop();
//~ //        //~ old_ctx = mfc0(CP0_ENTRYHI);
//~ //        //~ old_pagemask = mfc0(CP0_PAGEMASK);
//~ //        //~ wired = mfc0(CP0_WIRED);
//~ //        //~ if (--temp_tlb_entry < wired) {
//~ //                //~ printk(KERN_WARNING
//~ //                       //~ "No TLB space left for add_temporary_entry\n");
//~ //                //~ ret = -ENOSPC;
//~ //                //~ goto out;
//~ //        //~ }
//~ ////~ 
//~ //        //~ mtc0(CP0_INDEX, temp_tlb_entry);
//~ //        //~ mtc0(CP0_PAGEMASK, pagemask);
//~ //        //~ mtc0(CP0_ENTRYHI, entryhi);
//~ //        //~ mtc0(CP0_ENTRYLO0, entrylo0);
//~ //        //~ mtc0(CP0_ENTRYLO1, entrylo1);
//~ //        //~ 
//~ //        //~ tlb_write_indexed();
//~ //        //~ 
//~ ////~ 
//~ //        //~ mtc0(CP0_ENTRYHI, old_ctx);
//~ //        //~ mtc0(CP0_PAGEMASK, old_pagemask);
//~ //        //~ htw_start();
//~ ////~ out:
//~ //        //~ local_irq_restore(flags);
//~ //        //~ return ret;
//~ ////~ }
//~ 
//~ static int ntlb;
//~ static int __init set_ntlb(char *str)
//~ {
        //~ get_option(&str, &ntlb);
        //~ return 1;
//~ }
//~ 
//~ __setup("ntlb=", set_ntlb);
//~ 
//~ /*
 //~ * Configure TLB (for init or after a CPU has been powered off).
 //~ */
//~ static void r4k_tlb_configure(void)
//~ {
        //~ /*
         //~ * You should never change this register:
         //~ *   - On R4600 1.7 the tlbp never hits for pages smaller than
         //~ *     the value in the c0_pagemask register.
         //~ *   - The entire mm handling assumes the c0_pagemask register to
         //~ *     be set to fixed-size pages.
         //~ */
        //~ mtc0(CP0_PAGEMASK, PM_DEFAULT_MASK);
        //~ 
        //~ if (mfc0(CP0_PAGEMASK) != PM_DEFAULT_MASK)
                //~ panic("MMU doesn't support PAGE_SIZE=0x%lx", PAGE_SIZE);
//~ 
        //~ mtc0(CP0_WIRED, 0);
//~ 
//~ 
        //~ temp_tlb_entry = current_cpu_data.tlbsize - 1;
//~ 
        //~ /* From this point on the ARC firmware is dead.  */
        //~ local_flush_tlb_all();
//~ 
        //~ /* Did I tell you that ARC SUCKS?  */
//~ }
//~ 
//~ void tlb_init(void)
//~ {
        //~ r4k_tlb_configure();
//~ 
        //~ if (ntlb) {
                //~ if (ntlb > 1 && ntlb <= current_cpu_data.tlbsize) {
                        //~ int wired = current_cpu_data.tlbsize - ntlb;
                        //~ mtc0(CP0_WIRED, wired);
                        //~ mtc0(CP0_INDEX, wired-1);
                        //~ printk("Restricting TLB to %d entries\n", ntlb);
                //~ } else
                        //~ printk("Ignoring invalid argument ntlb=%d\n", ntlb);
        //~ }
//~ 
        //~ build_tlb_refill_handler();
//~ }
//~ 
//~ static int r4k_tlb_pm_notifier(struct notifier_block *self, unsigned long cmd,
                               //~ void *v)
//~ {
        //~ switch (cmd) {
        //~ case CPU_PM_ENTER_FAILED:
        //~ case CPU_PM_EXIT:
                //~ r4k_tlb_configure();
                //~ break;
        //~ }
//~ 
        //~ return NOTIFY_OK;
//~ }
//~ 
//~ static struct notifier_block r4k_tlb_pm_notifier_block = {
        //~ .notifier_call = r4k_tlb_pm_notifier,
//~ };
//~ 
//~ static int __init r4k_tlb_init_pm(void)
//~ {
        //~ return cpu_pm_register_notifier(&r4k_tlb_pm_notifier_block);
//~ }
//~ arch_initcall(r4k_tlb_init_pm);
//~ 


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////



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
    pok_mips_tlb_write(1,
        virtual,
        physical,
        pgsize_enum,
        permissions,
        wimge,
        pid,
        entry,
        TRUE);
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

static void pok_mips_tlb_print(unsigned tlbsel) {
    unsigned limit = pok_mips_tlb_get_nentry(1);

    for (unsigned i = 0; i < limit; i++) {
        unsigned valid;
        unsigned tsize;
        uint32_t epn;
        uint64_t rpn;
        pok_mips_tlb_read_entry(tlbsel, i,
                &valid,
                &tsize,
                &epn,
                &rpn
                );
        if (valid) {
            printf("DEBUG: tlb entry %d:%d:\r\n", tlbsel, i);
            printf("DEBUG:   Valid\r\n");
            printf("DEBUG:   Effective: %p\r\n", (void*)epn);
            // FIXME This is wrong. We print only 32 bits out of 36
            printf("DEBUG:   Physical: %x:%p\r\n",
                    (unsigned)(rpn>>32), (void*)(unsigned)rpn);
            printf("DEBUG:   Size: %s\r\n", pok_mips_tlb_size(tsize));

        }
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
        MIPS_PGSIZE_16M,  //TODO make smaller
        MAS3_SW | MAS3_SR | MAS3_SX,
        0,
        0, // any pid
        TRUE
    );
    /*
     * Clear all other mappings. For instance, those created by u-boot.
     */
    unsigned limit = pok_mips_tlb_get_nentry(1);
    pok_mips_tlb_write(1,
            pok_bsp.ccsrbar_base, pok_bsp.ccsrbar_base_phys, MIPS_PGSIZE_16M,
            //MAS3_SW | MAS3_SR | MAS3_SX,
            MAS3_SW | MAS3_SR | MAS3_SX | MAS3_UW | MAS3_UR,
            MAS2_W | MAS2_I | MAS2_M | MAS2_G,
            0,
            limit-1,
            TRUE
            );
    pok_ccsrbar_ready = 1;

    pok_mips_tlb_print(0);
    pok_mips_tlb_print(1);
//    pok_mips_tlb_clear_entry(1, 2);
    for (unsigned i = 1; i < limit-1; i++) {
        pok_mips_tlb_clear_entry(1, i);
    }

    // DIRTY HACK
    // By some reason P3041 DUART blocks when TLB entry #1 is overrriden.
    // Preserve it, let's POK write it's entries starting 2
    next_non_resident = next_resident = 2;

    for(int i = 0; i < ja_spaces_n; i++)
    {
        // This should be checked when generate deployment.c too.
        assert(ja_spaces[i].size_normal < POK_PARTITION_MEMORY_SIZE);
    }
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
    unsigned pid = mfc0(CP0_ENTRYHI);

    if (tlb_miss && faulting_address >= pok_bsp.ccsrbar_base && faulting_address < pok_bsp.ccsrbar_base + pok_bsp.ccsrbar_size) {
        pok_insert_tlb1(
            pok_bsp.ccsrbar_base,
            pok_bsp.ccsrbar_base_phys,
            MIPS_PGSIZE_16M,
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
            MIPS_PGSIZE_64K,
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
        jet_space_id space_id = pid;

        pok_insert_tlb1(
            POK_PARTITION_MEMORY_BASE,
            ja_spaces[space_id - 1].phys_base,
            MIPS_PGSIZE_16M,
            MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX,
            0,
            pid,
            FALSE
        );
    } else {
        if (vctx->STATUS & CP0_STATUS_KSU_1) {
            printf("USER ");
        } else {
            printf("KERNEL ");
        }

        if (type == PF_DATA_TLB_MISS || type == PF_DATA_STORAGE) {
            printf("code at %p address tried to access data on %p address\n", (void*)vctx->EPC, (void*)faulting_address);
        } else {
            printf("code at %p address tried to execute code at %p address\n", (void *)vctx->r31, (void*)vctx->EPC);
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

uintptr_t pok_virt_to_phys(uintptr_t virt)
{
    if((virt < POK_PARTITION_MEMORY_BASE)
        || (virt > POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE))
    {
        // Fatal error despite it is called from user space!!
        printf("pok_virt_to_phys: wrong virtual address %p\n", (void*)virt);
        pok_fatal("wrong pointer in pok_virt_to_phys\n");
    }

    jet_space_id space_id = ja_space_get_current();

    return virt - POK_PARTITION_MEMORY_BASE + ja_spaces[space_id - 1].phys_base;
}

uintptr_t pok_phys_to_virt(uintptr_t phys)
{
    jet_space_id space_id = ja_space_get_current();

    if((phys < ja_spaces[space_id - 1].phys_base)
        || (phys >= ja_spaces[space_id - 1].phys_base + POK_PARTITION_MEMORY_SIZE))
    {
        // Fatal error despite it is called from user space!!
        printf("pok_phys_to_virt: wrong physical address %p\n", (void*)phys);
        pok_fatal("wrong pointer in pok_phys_to_virt\n");
    }

    return phys - ja_spaces[space_id - 1].phys_base + POK_PARTITION_MEMORY_BASE;
}
