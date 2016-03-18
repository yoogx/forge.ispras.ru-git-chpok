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

/**
 * \file    arch/ppc/arch.c
 * \author  Tristan Gingold
 * \date    2009
 * \brief   Provide generic architecture access for PPC architecture
 */

#include <config.h>

#include <types.h>
#include <errno.h>
#include <core/partition.h>
#include "reg.h"
#include "msr.h"
#include "space.h"
#include "devtree.h"
#include <bsp_common.h>

#include <libc.h>
#include "space.h"
#include "mmu.h"


void pok_ppc_tlb_print(unsigned tlbsel);

void pok_ppc_tlb_write_empty(
        unsigned tlbsel,
        uint32_t virtual,
        uint64_t physical,
        unsigned pgsize_enum,
        unsigned permissions,
        unsigned wimge,
        unsigned pid,
        unsigned entry,
        bool_t   valid
        );

void pok_ppc_tlb_only_write_reg(
        unsigned tlbsel,
        uint32_t virtual,
        uint64_t physical,
        unsigned pgsize_enum,
        unsigned permissions,
        unsigned wimge,
        unsigned pid,
        unsigned entry,
        bool_t   valid
        );

void pok_ppc_tlb_write_nosync(
        unsigned tlbsel,
        uint32_t virtual,
        uint64_t physical,
        unsigned pgsize_enum,
        unsigned permissions,
        unsigned wimge,
        unsigned pid,
        unsigned entry,
        bool_t   valid
        );


static inline uint64_t
__ppc_get_timebase (void)
{
    uint32_t __tbu, __tbl, __tmp;
    asm volatile ("0:\n\t"
            "mftbu %0\n\t"
            "mftbl %1\n\t"
            "mftbu %2\n\t"
            "cmpw %0, %2\n\t"
            "bne- 0b"
            : "=r" (__tbu), "=r" (__tbl), "=r" (__tmp));
    return (((uint64_t) __tbu << 32) | __tbl);
}

#define COUNT 2000000
#define TLB_NB 50
#define START 0x400000
void make_measurement ( void (*func) (
                unsigned tlbsel,
                uint32_t virtual,
                uint64_t physical,
                unsigned pgsize_enum,
                unsigned permissions,
                unsigned wimge,
                unsigned pid,
                unsigned entry,
                bool_t   valid
                )
    )
  {
      uint64_t start = __ppc_get_timebase();

      for (int j = 0; j < COUNT; j++) {
          for (int i = 1; i < 1 + TLB_NB; i++) {
              func(
                      1,
                      START + i*0x1000,
                      START + i*0x1000,
                      E500MC_PGSIZE_4K,
                      MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX,
                      0,
                      0,
                      i,
                      TRUE
                      );

          }
      }

      uint64_t end = __ppc_get_timebase();
      uint64_t delta = end - start;
      unsigned delta_div = delta/(TLB_NB*COUNT/1000);

      printf("%llu ticks for %d iterations %u.%03u ticks per iteration\n\n", delta, TLB_NB*COUNT,
              delta_div/1000, delta_div%1000);
  }

pok_ret_t pok_arch_init ()
{
  mtmsr(MSR_IP | MSR_FP);


#if POK_NEEDS_PARTITIONS
  pok_arch_space_init();
#endif

  printf("----------------------------------------\n");

  {
      uint64_t start = __ppc_get_timebase();

      for (int j = 0; j < COUNT; j++) {
          for (int i = 1; i < 1 + TLB_NB; i++) {
              pok_ppc_tlb_write_empty(
                      1,
                      START + i*0x1000,
                      START + i*0x1000,
                      E500MC_PGSIZE_4K,
                      MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX,
                      0,
                      0,
                      i,
                      TRUE
                      );

          }
      }

      uint64_t end = __ppc_get_timebase();
      uint64_t delta = end - start;
      unsigned delta_div = delta/(TLB_NB*COUNT/1000);

      printf("empty function: ");
      printf("%llu ticks for %d iterations %u.%03u ticks per iteration\n", delta, TLB_NB*COUNT,
              delta_div/1000, delta_div%1000);
  }
      printf("by func ptr: ");
      make_measurement(pok_ppc_tlb_write_empty);
      printf("\n");

  {
      uint64_t start = __ppc_get_timebase();

      for (int j = 0; j < COUNT; j++) {
          for (int i = 1; i < 1 + TLB_NB; i++) {
              pok_ppc_tlb_only_write_reg(
                      1,
                      START + i*0x1000,
                      START + i*0x1000,
                      E500MC_PGSIZE_4K,
                      MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX,
                      0,
                      0,
                      i,
                      TRUE
                      );

          }
      }

      uint64_t end = __ppc_get_timebase();
      uint64_t delta = end - start;
      unsigned delta_div = delta/(TLB_NB*COUNT/1000);

      printf("only write to reg: ");
      printf("%llu ticks for %d iterations %u.%03u ticks per iteration\n", delta, TLB_NB*COUNT,
              delta_div/1000, delta_div%1000);
  }
      printf("by func ptr: ");
      make_measurement(pok_ppc_tlb_only_write_reg);
      printf("\n");


  {
      uint64_t start = __ppc_get_timebase();

      for (int j = 0; j < COUNT; j++) {
          for (int i = 1; i < 1 + TLB_NB; i++) {
              pok_ppc_tlb_write_nosync(
                      1,
                      START + i*0x1000,
                      START + i*0x1000,
                      E500MC_PGSIZE_4K,
                      MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX,
                      0,
                      0,
                      i,
                      TRUE
                      );

          }
      }

      uint64_t end = __ppc_get_timebase();
      uint64_t delta = end - start;
      unsigned delta_div = delta/(TLB_NB*COUNT/1000);

      printf("without sync op: ");
      printf("%llu ticks for %d iterations %u.%03u ticks per iteration\n", delta, TLB_NB*COUNT,
              delta_div/1000, delta_div%1000);
  }
      printf("by func ptr: ");
      make_measurement(pok_ppc_tlb_write_nosync);
      printf("\n");
  {
      uint64_t start = __ppc_get_timebase();

      for (int j = 0; j < COUNT; j++) {
          for (int i = 1; i < 1 + TLB_NB; i++) {
              pok_ppc_tlb_write(
                      1,
                      START + i*0x1000,
                      START + i*0x1000,
                      E500MC_PGSIZE_4K,
                      MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX,
                      0,
                      0,
                      i,
                      TRUE
                      );

          }
      }

      uint64_t end = __ppc_get_timebase();
      uint64_t delta = end - start;
      unsigned delta_div = delta/(TLB_NB*COUNT/1000);

      printf("full : ");
      printf("%llu ticks for %d iterations %u.%03u ticks per iteration\n", delta, TLB_NB*COUNT,
              delta_div/1000, delta_div%1000);
      printf("by func ptr: ");
      make_measurement(pok_ppc_tlb_write);
      printf("\n");
  }

  for (unsigned i = 1; i < 63; i++) {
      pok_ppc_tlb_clear_entry(1, i);
  }

  printf("----------------------------------------\n");



  return (POK_ERRNO_OK);
}

pok_ret_t pok_arch_preempt_disable()
{
  mtmsr(mfmsr() & ~MSR_EE);

  return (POK_ERRNO_OK);
}

pok_ret_t pok_arch_preempt_enable()
{
  mtmsr(mfmsr() | MSR_EE);

  return (POK_ERRNO_OK);
}

pok_bool_t pok_arch_preempt_enabled(void)
{
  return !!(mfmsr() & MSR_EE);
}

void pok_arch_inf_loop()
{
   pok_arch_preempt_disable();

   while (1)
   {}
}

pok_ret_t pok_arch_idle()
{
   pok_arch_preempt_enable();

   while (1)
   {
   }

   return (POK_ERRNO_OK);	
}

pok_ret_t pok_arch_event_register (uint8_t vector, void (*handler)(void))
{
  (void) vector;
  (void) handler;

  return (POK_ERRNO_OK);
}


uint32_t    pok_thread_stack_addr   (const uint8_t    partition_id,
                                     const uint32_t   local_thread_id)
{
   (void) partition_id; 
   return POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE - 16 - (local_thread_id * POK_USER_STACK_SIZE);
}


#include <arch/ppc/linux_io.h>
#define DCFG_RSTCR 0xb0
#define RSTCR_RESET_REQ 0x2
void pok_arch_cpu_reset()
{
    uintptr_t addr = pok_bsp.ccsrbar_base + pok_bsp.dcfg_offset + DCFG_RSTCR;
    out_be32((void*)addr, RSTCR_RESET_REQ);
}
