/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */
// TODO Fix space.c to compile with 64 bit address

#include <config.h>

#include <types.h>
#include <errno.h>
#include <libc.h>
#include <bsp.h>
#include <core/sched.h>
#include <core/debug.h>

#include <arch.h>
#include "thread.h"
#include "msr.h"
#include "reg.h"
#include "mmu.h"
#include "space.h"
#include "bspconfig.h"

extern struct pok_space spaces[];

pok_ret_t pok_create_space (pok_partition_id_t partition_id,
                            uintptr_t addr,
                            size_t size)
{
#ifdef POK_NEEDS_DEBUG
  printf ("pok_create_space partid=%d: phys=%x size=%x\n", partition_id, addr, size);
  printf("CCSRBAR: %llu\n", CCSRBAR_BASE);
#endif
  spaces[partition_id].phys_base = addr;
  spaces[partition_id].size = size;

  return (POK_ERRNO_OK);
}

pok_ret_t pok_space_switch (pok_partition_id_t old_partition_id,
                            pok_partition_id_t new_partition_id)
{
    (void) old_partition_id;
    mtspr(SPRN_PID, new_partition_id + 1);

    return POK_ERRNO_OK;
}

uintptr_t pok_space_base_vaddr(uintptr_t addr)
{
    (void) addr;
    return POK_PARTITION_MEMORY_BASE;
}
    
static void
pok_space_context_init(
        volatile_context_t *vctx,
        context_t *ctx,
        uint8_t partition_id,
        uintptr_t entry_rel,
        uintptr_t stack_rel,
        uint32_t arg1,
        uint32_t arg2)
{
    (void) partition_id;

    memset (ctx, 0, sizeof(*ctx));
    memset (vctx, 0, sizeof(*vctx));

    extern void pok_arch_rfi (void);

    vctx->r3     = arg1;
    vctx->r4     = arg2;
    vctx->sp     = stack_rel - 12;
    vctx->srr0   = entry_rel;
    vctx->srr1   = MSR_EE | MSR_IP | MSR_PR | MSR_FP;
    ctx->lr      = (uintptr_t) pok_arch_rfi;

    ctx->sp      = (uintptr_t) &vctx->sp;
}

uint32_t pok_space_context_create (
        uint8_t partition_id,
        uint32_t entry_rel,
        uint32_t stack_rel,
        uint32_t arg1,
        uint32_t arg2)
{
  volatile_context_t* vctx;
  context_t* ctx;
  char*      stack_addr;

  stack_addr = pok_bsp_mem_alloc (KERNEL_STACK_SIZE);
  
  vctx = (volatile_context_t *)
    (stack_addr + KERNEL_STACK_SIZE - sizeof (volatile_context_t));
  
  ctx = (context_t *)(((char*)vctx) - sizeof(context_t) + 8);
  
  pok_space_context_init(vctx, ctx, partition_id, entry_rel, stack_rel, arg1, arg2);

#ifdef POK_NEEDS_DEBUG
  printf ("space_context_create %lu: entry=%lx stack=%lx arg1=%lx arg2=%lx ksp=%p\n",
          (unsigned long) partition_id, 
          (unsigned long) entry_rel, 
          (unsigned long) stack_rel, 
          (unsigned long) arg1, 
          (unsigned long) arg2, 
          &vctx->sp);
#endif

  return (uint32_t)ctx;
}

void pok_space_context_restart(
        uint32_t sp, 
        uint8_t  partition_id,
        uint32_t entry_rel,
        uint32_t stack_rel,
        uint32_t arg1,
        uint32_t arg2)
{
    // it's the same sp that was 
    // returned by pok_space_context_create earlier
    // 
    // we don't need to allocate anything here, we only have to 
    // reset some values

    volatile_context_t *vctx = (volatile_context_t*)(sp + sizeof(context_t) - 8);
    context_t *ctx = (context_t*) sp;

    pok_space_context_init(
        vctx,
        ctx, 
        partition_id,
        entry_rel,
        stack_rel,
        arg1,
        arg2
    );
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
        uint32_t virtual, 
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
void pok_instert_tlb0();

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
		if (valid) {
			printf("DEBUG: tlb entry %d:%d:\r\n", tlbsel, i);
			printf("DEBUG:   Valid\r\n");
			printf("DEBUG:   Effective: %p\r\n", (void*)epn);
			// FIXME This is wrong. We print only 32 bits out of 36
			printf("DEBUG:   Physical: %x:%p\r\n", 
				(unsigned)(rpn>>32), (void*)(unsigned)rpn);
			printf("DEBUG:   Size: %s\r\n", pok_ppc_tlb_size(tsize));
			
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
        E500MC_PGSIZE_64M,  //TODO make smaller
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
		CCSRBAR_BASE, CCSRBAR_BASE, E500MC_PGSIZE_16M, 
		MAS3_SW | MAS3_SR | MAS3_SX,
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
#define MPC8544_PCI_IO             0xE1000000ULL

void pok_arch_handle_page_fault(uintptr_t faulting_address, uint32_t syndrome)
{
    (void) syndrome;

    unsigned pid = mfspr(SPRN_PID);
	if (pok_ccsrbar_ready) {
		printf("DEBUG: page fault: pid: %u, address %p, syndrome %u\n", 
			pid,
			(void*)faulting_address, 
			(unsigned)syndrome);
	}
    if (faulting_address >= CCSRBAR_BASE && faulting_address < CCSRBAR_BASE + CCSRBAR_SIZE) {
        pok_insert_tlb1(
            CCSRBAR_BASE, 
            CCSRBAR_BASE, 
            E500MC_PGSIZE_16M, 
            MAS3_SW | MAS3_SR,
            MAS2_W | MAS2_I | MAS2_M | MAS2_G,
            0, /* any pid */
            TRUE 
        );
    } else if (faulting_address >= MPC8544_PCI_IO && faulting_address < MPC8544_PCI_IO + MPC8544_PCI_IO_SIZE) {
		pok_insert_tlb1(
            MPC8544_PCI_IO,
            MPC8544_PCI_IO,
            E500MC_PGSIZE_64K,
            MAS3_SW | MAS3_SR,
            MAS2_W | MAS2_I | MAS2_M | MAS2_G,
            0, /* any pid */
            TRUE
        );
    } else if (
            pid != 0 &&
            faulting_address >= POK_PARTITION_MEMORY_BASE && 
            faulting_address < POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE) 
    {
        pok_partition_id_t partid = pid - 1;

        pok_insert_tlb1( 
            POK_PARTITION_MEMORY_BASE,
            spaces[partid].phys_base,
            E500MC_PGSIZE_16M,
            MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX,
            0,
            pid,
            FALSE
        );
    } else {
        // TODO handle it correctly, distinguish kernel code / user code, etc.
        pok_fatal("bad memory access");
    }
}

